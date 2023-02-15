#include "shader_compiler.h"

#include <regex>

// #include <CXXGraph/include/CXXGraph.hpp>
#include <boost/graph/edge_list.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include "runtime/core/encryption/md5.h"
#include "runtime/core/io/file_system.h"
#include "runtime/core/log/log.h"
#include "runtime/core/memory/allocators.h"
#include "runtime/core/platform/platform.h"
#include "runtime/core/utils/functions.h"

namespace Horizon {

ShaderCompiler::ShaderCompiler() noexcept {
    CHECK_DX_RESULT(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&idxc_compiler)));
    CHECK_DX_RESULT(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&idxc_utils)));
    CHECK_DX_RESULT(idxc_utils->CreateDefaultIncludeHandler((&idxc_include_handler)));
}

ShaderCompiler::~ShaderCompiler() noexcept {}

void ShaderCompiler::Compile(const Container::String &blob, const ShaderCompilationArgs &compile_args) {
    ShaderCompiler::get().InternalCompile(blob, compile_args);
}

struct ShaderList {
    const std::filesystem::path path;
    ShaderCompilationArgs args;
    bool need_compile = true;
};

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


void IterateIncludeFiles(const std::filesystem::path &path,Container::HashMap<std::filesystem::path, Container::String>& include_map) {
    // exist
    if (include_map.find(path) != include_map.end())
        return;
    auto text = fs::read_text_file(Container::String{path.string()});
    include_map.emplace(path, text);
    std::regex reg{"#include \"*.*\""};
    std::sregex_iterator occurs(text.cbegin(), text.cend(), reg);

    for (std::sregex_iterator end; occurs != end; ++occurs) {
        Container::String matched_str{occurs->str()};
        auto pos = matched_str.find("\"") + 1;
        matched_str = matched_str.substr(pos,
                                         matched_str.length() - pos - 1); // remove #include " "
        std::filesystem::path abs_include_path = path.parent_path() / matched_str.c_str();
        IterateIncludeFiles(std::filesystem::absolute(abs_include_path), include_map);
    }
}

void CheckMd5() {}


void ShaderCompiler::CompileShaders(const ShaderCompilationSettings &settings) {

    Container::HashMap<std::filesystem::path, Container::String> shader_texts(settings.shader_list.size());
    Container::HashSet<std::filesystem::path> include_file_list;
    for (auto &path : settings.shader_list) {
        shader_texts.emplace(std::filesystem::absolute(path),
                             fs::read_text_file(path.string().c_str())); // TODO(hylu): handle error
    }

    Container::Array<ShaderList> shader_list;

    boost::adjacency_list<boost::listS, boost::vecS, boost::directedS> shader_dependency_graph;

    for (auto &[path,  text] : shader_texts) {
    {
        //boost::graph::add_vertex(shader_dependency_graph);
        std::regex entry{"[VPCHDM]S_MAIN"}; // vs, ps, cs, hs, ds, ms, TODO(hylu): rt
            std::sregex_iterator pos(text.cbegin(), text.cend(), entry);

        for (std::sregex_iterator end; pos != end; ++pos) {
            ShaderList l{path.string()}; // shader text ref=
            l.args.entry_point = pos->str();
            l.args.optimization_level = settings.optimization_level;
            l.args.target_api = settings.target_api;
            l.args.include_path = settings.input_dir / "include";
            Container::String output_file_name =
                Container::String{path.filename().string()} + "." + Container::String{pos->str().substr(0, 2)} + ".hsb"; // add api

            l.args.out_file_path = settings.output_dir / output_file_name;
            l.args.target_profile = GetShaderTargetProfile(pos->str().c_str(), settings.sm_version);
            shader_list.push_back(std::move(l));
        }
    }

    // build shader dependency graph

    
    // process include dependency graph
    std::regex reg{"#include \"*.*\""};
    std::sregex_iterator occurs(text.cbegin(), text.cend(), reg);

    for (std::sregex_iterator end; occurs != end; ++occurs) {
        Container::String matched_str{occurs->str()};
        auto pos = matched_str.find("\"") + 1;
        matched_str = matched_str.substr(pos,
                                         matched_str.length() - pos - 1); // remove #include " "
        std::filesystem::path abs_include_path = settings.input_dir / matched_str.c_str();
        include_file_list.emplace(std::filesystem::absolute(abs_include_path));
    }
    }

    Container::HashMap<std::filesystem::path, Container::String> include_file_text; 

    for (auto &include_file : include_file_list) {
        IterateIncludeFiles(include_file, include_file_text);
    }
    // iterate graph,
    // if leaf_need_recompile
    // compileshaders
    // cached md5 value
    //CheckMd5();


    // multithread shader compiling
    tbb::parallel_for(tbb::blocked_range<u32>(0, static_cast<u32>(shader_list.size())),
                      [&shader_list, &shader_texts](const tbb::blocked_range<u32> &r) {
                          for (u32 v = r.begin(); v < r.end(); v++) {
                              auto &shader = shader_list[v];
                              if (!shader.need_compile) {
                                  continue;
                              }

                              ShaderCompiler::Compile(shader_texts[shader.path], shader.args);
                          }
                      });

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
