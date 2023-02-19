#include "shader_compiler.h"

#include <regex>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/edge_list.hpp>
#include <boost/graph/depth_first_search.hpp>

#include <boost/graph/graph_utility.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/algorithm/string.hpp>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include "runtime/core/encryption/md5.h"
#include "runtime/core/io/file_system.h"
#include "runtime/core/log/log.h"
#include "runtime/core/memory/allocators.h"
#include "runtime/core/platform/platform.h"
#include "runtime/core/utils/functions.h"

namespace std {
template <typename X, typename Y> struct std::hash<std::pair<X, Y>> {
    std::size_t operator()(const std::pair<X, Y> &pair) const {
        return std::hash<X>()(pair.first) ^ std::hash<Y>()(pair.second);
    }
};
} // namespace std

namespace Horizon {

using namespace boost;

ShaderCompiler::ShaderCompiler() noexcept {
    CHECK_DX_RESULT(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&idxc_compiler)));
    CHECK_DX_RESULT(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&idxc_utils)));
    CHECK_DX_RESULT(idxc_utils->CreateDefaultIncludeHandler((&idxc_include_handler)));
}

ShaderCompiler::~ShaderCompiler() noexcept {}

void ShaderCompiler::Compile(const Container::String &blob, const ShaderCompilationArgs &compile_args) {
    ShaderCompiler::get().InternalCompile(blob, compile_args);
}

struct ShaderCompilationSetting {
    const std::filesystem::path path; // key to index the shader_text map
    ShaderCompilationArgs args; // args
};

struct FileNode {
    std::optional<std::filesystem::path> shader_path;
    bool need_compile;
    u64 graph_node_index;
    Container::Array<std::filesystem::path> header_files;
};

static std::unordered_set<std::pair<u64, u64>> edge_list;
static u64 node_index;
static std::vector<std::filesystem::path> node_index_map;

static std::filesystem::path cached_project_dir = std::filesystem::temp_directory_path() / "horizon";
static std::filesystem::path cached_shader_dir = cached_project_dir / "shader";

// TODO(hyl5): looking for better enum casting method
// TODO(hylu): replace std::strcmp
ShaderTargetProfile GetShaderTargetProfile(const char *str, ShaderModuleVersion version) {
    constexpr ShaderModuleVersion base_version = ShaderModuleVersion::SM_6_0;
    ShaderTargetProfile base_profile;
    if (std::strcmp(str, "VS_MAIN") == 0) {
        base_profile = ShaderTargetProfile::VS_6_0;
    } else if (std::strcmp(str, "PS_MAIN") == 0) {
        base_profile = ShaderTargetProfile::PS_6_0;
    } else if (std::strcmp(str, "CS_MAIN") == 0) {
        base_profile = ShaderTargetProfile::CS_6_0;
    } else if (std::strcmp(str, "GS_MAIN") == 0) {
        base_profile = ShaderTargetProfile::GS_6_0;
    } else if (std::strcmp(str, "GS_MAIN") == 0) {
        base_profile = ShaderTargetProfile::HS_6_0;
    } else if (std::strcmp(str, "DS_MAIN") == 0) {
        base_profile = ShaderTargetProfile::DS_6_0;
    } else if (std::strcmp(str, "MS_MAIN") == 0) {
        base_profile = ShaderTargetProfile::MS_6_0;
    } else {
        LOG_ERROR("invalid shader type");
        return ShaderTargetProfile::INVALID;
    }

    u32 version_offset = static_cast<u32>(version) - static_cast<u32>(base_version);
    u32 target_version = static_cast<u32>(base_profile) + version_offset;
    return static_cast<ShaderTargetProfile>(target_version);
}

// we read once to build dependency graph and cache
void IterateHeaderFiles(const std::filesystem::path &path,
                         Container::HashMap<std::filesystem::path, FileNode> &dependency_map, u64 in_node, bool is_leaf_node) {
    // exist
    auto it = dependency_map.find(path);
    // ignore leaf
    if (it != dependency_map.end() && !it->second.shader_path.has_value()) {
        edge_list.emplace(std::make_pair(in_node, it->second.graph_node_index));
        return;
    }
    FileNode current_node;
    auto text = fs::read_text_file(Container::String{path.string()});
    // we use last write time instead of md5, but that might not accurate
    //auto md5_value = md5(text);
    std::filesystem::file_time_type last_mod_time = std::filesystem::last_write_time(path);
    auto s_last_mod_time = Container::String{std::to_string(to_time_t(last_mod_time))};
    auto cached_path = cached_shader_dir / path.filename(); // FIXME(hylu): filename may conflict
    if (std::filesystem::exists(cached_path) && (fs::read_text_file(cached_path) == s_last_mod_time)) {
        current_node.need_compile = false;
    } else {
        fs::write_text_file(Container::String{cached_path.string()}, s_last_mod_time.data(),
                            s_last_mod_time.size() * sizeof(char));
    }
    if (is_leaf_node) {
        current_node.shader_path = path;
    }
    current_node.graph_node_index = node_index;
    node_index_map.push_back(path);
    dependency_map.emplace(path, std::move(current_node));
    edge_list.emplace(std::make_pair(in_node, node_index));
    node_index++;

    // the following code is generated from ChatGPT.
    std::regex reg{"#include\\s+[\"<](.*)[\">]"};
    std::smatch match;
    // Search for all #include directives in the file
    std::string::const_iterator search_start(text.cbegin());
    while (std::regex_search(search_start, text.cend(), match, reg)) {
        std::filesystem::path abs_include_path = path.parent_path() / match[1].str();
        // Move the search start to the end of the matched string
        search_start = match.suffix().first;
        IterateHeaderFiles(std::filesystem::absolute(abs_include_path), dependency_map, node_index, false);
    }

    //std::sregex_iterator occurs(text.cbegin(), text.cend(), reg);
    //// iterate parent header file
    //for (std::sregex_iterator end; occurs != end; ++occurs) {
    //    Container::String matched_str{occurs->str()};
    //    auto pos = matched_str.find("\"") + 1;
    //    matched_str = matched_str.substr(pos,
    //                                     matched_str.length() - pos - 1); // remove #include " "
    //    std::filesystem::path abs_include_path = path.parent_path() / matched_str.c_str();
    //    IterateHeaderFiles(std::filesystem::absolute(abs_include_path), dependency_map, node_index, false);
    //}
}

void ShaderCompiler::CompileShaders(const ShaderCompilationSettings &settings) {

    // create cached directory
    if (!std::filesystem::exists(cached_project_dir)) {
        std::filesystem::create_directory(cached_project_dir);
    }
    if (!std::filesystem::exists(cached_shader_dir)) {
        std::filesystem::create_directory(cached_shader_dir);
    }

    // shader text map, value contains shader blob and cache stat.
    Container::HashMap<std::filesystem::path, std::pair<Container::String, bool>> shader_texts(settings.shader_list.size());

    for (auto &path : settings.shader_list) {
        shader_texts.emplace(std::filesystem::absolute(path), std::make_pair(fs::read_text_file(path.string().c_str()),
                                                                             true)); // TODO(hylu): handle file reading error
    }

    // extract args from shader text for compiling
    // each ShaderCompilationSetting
    Container::Array<ShaderCompilationSetting> shader_compilation_settings;
    for (auto &[path, text] : shader_texts) {
        std::regex entry{"[VPCHDM]S_MAIN"}; // vs, ps, cs, hs, ds, ms, TODO(hylu): rt
        std::sregex_iterator pos(text.first.cbegin(), text.first.cend(), entry);

        for (std::sregex_iterator end; pos != end; ++pos) {
            ShaderCompilationSetting scs{path.string()}; // shader text ref
            scs.args.entry_point = pos->str();
            scs.args.optimization_level = settings.optimization_level;
            scs.args.target_api = settings.target_api;
            scs.args.include_path = settings.input_dir / "include";
            Container::String output_file_name = Container::String{path.filename().string()} + "." +
                                                 Container::String{pos->str().substr(0, 2)} + ".hsb"; // add api
            scs.args.out_file_path = settings.output_dir / output_file_name;
            scs.args.target_profile = GetShaderTargetProfile(pos->str().c_str(), settings.sm_version);
            shader_compilation_settings.push_back(std::move(scs));
        }
    }

    Container::HashMap<std::filesystem::path, FileNode> shader_node_map;
    // build shader dependency graph
    for (auto &[path, text] : shader_texts) {
        IterateHeaderFiles(path, shader_node_map, node_index, true);
    }

    boost::adjacency_list<boost::setS, boost::vecS, boost::directedS> shader_dependency_graph(edge_list.begin(),
                                                                                               edge_list.end(), edge_list.size());
    // iterate graph
    for (u32 idx = 0; idx < node_index_map.size(); idx++) {
    
        const auto &v = vertex(idx, shader_dependency_graph);
        
    }
    //for () {
    //    if (curretn_node.shader_path.has_value()) {

    //        auto parent_path = shader_path;
    //        shader_texts[parent_path].second &= curretn_node.need_compile;
    //    }
    //}

#ifndef NDBUG
    for (auto &idx : node_index_map) {
        Container::String text = shader_node_map[idx].shader_path.has_value() ? "is leaf node" : "not leaf node";
        LOG_INFO("{}, {}", idx.filename().string(), text);
    }
#endif // ! NDBUG

    // multithread shader compiling
    tbb::parallel_for(tbb::blocked_range<u32>(0, static_cast<u32>(shader_compilation_settings.size())),
                      [&shader_compilation_settings, &shader_texts](const tbb::blocked_range<u32> &r) {
                          for (u32 v = r.begin(); v < r.end(); v++) {
                              auto &shader = shader_compilation_settings[v];
                              auto &[shader_text, need_compile] = shader_texts[shader.path];
                              if (need_compile == true) {

                                  ShaderCompiler::Compile(shader_text, shader.args);
                              }
                          }
                      });

    // single thread version
    //for (auto &shader : shader_list) {
    //    if (!shader.need_compile) {
    //        continue;
    //    }

    //    ShaderCompiler::Compile(shader_texts[shader.path], shader.args);
    //}
}


void ShaderCompiler::InternalCompile(const Container::String &hlsl_text, const ShaderCompilationArgs &compile_args) {
    IDxcBlobEncoding *hlsl_blob{};
    CHECK_DX_RESULT(idxc_utils->CreateBlob(hlsl_text.data(), static_cast<u32>(hlsl_text.size()), 0, &hlsl_blob));

    auto stack_memory = Memory::GetStackMemoryResource(512);
    Container::Array<LPCWSTR> compilation_arguments(&stack_memory);

    // entry point
    Container::WString ep(compile_args.entry_point.begin(), compile_args.entry_point.end(), &stack_memory);
    compilation_arguments.push_back(L"-E");
    compilation_arguments.push_back(ep.c_str());
    // target profile
    compilation_arguments.push_back(L"-T");
    const wchar_t *tp = ToDxcTargetProfile(compile_args.target_profile);
    compilation_arguments.push_back(tp);

    compilation_arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS); // warning are errors
    compilation_arguments.push_back(DXC_ARG_ALL_RESOURCES_BOUND);
    compilation_arguments.push_back(DXC_ARG_ALL_RESOURCES_BOUND);
    //compilation_arguments.push_back(L"Fre");
    //Container::WString ref_file =
    //    Container::WString{compile_args.output_file_name.begin(), compile_args.output_file_name.end()} + L".ref";

    //compilation_arguments.push_back(ref_file.c_str());

    auto ws = compile_args.include_path.wstring();
    Container::WString ip(ws.begin(), ws.end(), &stack_memory);

    compilation_arguments.push_back(L"I");
    compilation_arguments.push_back(ip.c_str());

    if (USE_ROW_MAJOR_MATRIX) {
        compilation_arguments.push_back(DXC_ARG_PACK_MATRIX_ROW_MAJOR);
    }

    if (compile_args.optimization_level == ShaderOptimizationLevel::DEBUG) {
        compilation_arguments.push_back(DXC_ARG_DEBUG);

        //TODO(hylu): debug
        //compilation_arguments.push_back(L"-Fo");
        //Container::WString pdb_name = out_file_name / ".hlslpdb";
        //compilation_arguments.push_back(pdb_name.c_str());
    } else if (compile_args.optimization_level == ShaderOptimizationLevel::O3) {
        compilation_arguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);
    }

    if (compile_args.target_api == ShaderTargetAPI::SPIRV) {
        compilation_arguments.push_back(L"-spirv");
        compilation_arguments.push_back(L"-fspv-target-env=vulkan1.3");
    }

    DxcBuffer source_buffer{hlsl_blob->GetBufferPointer(), hlsl_blob->GetBufferSize(), 0u};

    IDxcResult *compile_result{};
    CHECK_DX_RESULT(idxc_compiler->Compile(&source_buffer, compilation_arguments.data(),
                                           static_cast<u32>(compilation_arguments.size()), idxc_include_handler,
                                           IID_PPV_ARGS(&compile_result)));

    // handle errors

    // Get compilation errors (if any).
    IDxcBlobUtf8 *errors{};
    CHECK_DX_RESULT(compile_result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
    if (errors && errors->GetStringLength() > 0) {
        const LPCSTR errorMessage = errors->GetStringPointer();
        LOG_ERROR("shader compilation failed {}", errorMessage);
    }

    // save result to disk
    // TODO(hylu): prevent expose std::iostream to handle IO
    std::ofstream os;
    os.open(compile_args.out_file_path, std::ios::binary | std::ios::out);
    struct ShaderBlobHeader {
        //
        // version
        // magic number
        u32 size;
        u32 reflection_blob;
        u32 dxil_offset;
        u32 spirv_offset;
    } header{};
    os.write(reinterpret_cast<const char *>(&header), sizeof(ShaderBlobHeader));

    // IR/IL
    //TODO(hylu) save both dxil and spirv, for changing backend in runtime?
    if (compile_args.target_api == ShaderTargetAPI::DXIL) {

    } else if (compile_args.target_api == ShaderTargetAPI::SPIRV) {
        IDxcBlob *spirv_code;
        compile_result->GetResult(&spirv_code);
        os.write(static_cast<const char *>(spirv_code->GetBufferPointer()), spirv_code->GetBufferSize());
    }

    // create reflection data

    os.close();
    //IDxcBlob *reflection_blob{};
    //CHECK_DX_RESULT(compile_result->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflection_blob), nullptr));

    //DxcBuffer reflectionBuffer{reflection_blob->GetBufferPointer(), reflection_blob->GetBufferSize(), 0};

    //ID3D12ShaderReflection *shaderReflection{};
    //idxc_utils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&shaderReflection));
    //D3D12_SHADER_DESC shaderDesc{};
    //shaderReflection->GetDesc(&shaderDesc);

    // release resources

}

} // namespace Horizon
