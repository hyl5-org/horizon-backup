#include "shader_compiler.h"

#include <regex>
// #include <CXXGraph/include/CXXGraph.hpp>

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

void IterateIncludeFiles() {}

void BuildShaderDependencyGraph(const ShaderCompilationSettings &settings) {

    //CXXGRAPH::Graph<u32> graph;
}

void CheckMd5() {}

struct ShaderList {
    bool need_compile = true;
    ShaderCompilationArgs args;
    u32 shader_text_index;
};

ShaderTargetProfile GetShaderTargetProfile(const char* str, ShaderModuleVersion version) {
    ShaderType shader_type;
    if (str == "VS_MAIN") {
        switch (version) {
        case Horizon::ShaderModuleVersion::SM_6_0:
            return ShaderTargetProfile::VS_6_0;
        case Horizon::ShaderModuleVersion::SM_6_1:
            return ShaderTargetProfile::VS_6_1;
        case Horizon::ShaderModuleVersion::SM_6_2:
        case Horizon::ShaderModuleVersion::SM_6_3:
        case Horizon::ShaderModuleVersion::SM_6_4:
        case Horizon::ShaderModuleVersion::SM_6_5:
        case Horizon::ShaderModuleVersion::SM_6_6:
        case Horizon::ShaderModuleVersion::SM_6_7:
        default:
            break;
        }
    } else if (str == "PS_MAIN") {
    
    }


}

void ShaderCompiler::CompileShaders(const ShaderCompilationSettings &settings) {

    Container::Array<Container::String> shader_texts(settings.shader_list.size());
    for (auto &shader : settings.shader_list) {
        shader_texts.push_back(fs::read_text_file(shader.string().c_str())); // TODO(hylu): handle error
    }

    Container::Array<ShaderList> shader_list;

    u32 index = 0;
    for (auto &shader_text : shader_texts) {
    {
        std::regex entry{"[VPCHDM]S_MAIN"}; // vs, ps, cs, hs, ds, ms, TODO(hylu): rt
        std::sregex_iterator pos(shader_text.cbegin(), shader_text.cend(), entry);

        for (std::sregex_iterator end; pos != end; ++pos) {
            LOG_INFO("entry point:{}", pos->str());
            ShaderList l;
            l.shader_text_index = index;
            l.args.entry_point = pos->str();
            l.args.optimization_level = settings.optimization_level;
            l.args.target_api = settings.target_api;
            l.args.include_path = settings.input_dir / "include";
            l.args.output_file_name = settings.output_dir / / ".hsb";
            l.args.target_profile = GetShaderTargetProfile(pos->str().c_str(), settings.sm_version);
            shader_list.push_back(std::move(l));
        }
        index++;
    }

    if (ENABLE_SHADER_CACHE) {
        Container::HashSet<std::filesystem::path> include_file_list;

        // process include dependency graph
        std::regex reg{"#include \"*.*\""};
        std::sregex_iterator occurs(shader_text.cbegin(), shader_text.cend(), reg);

        for (std::sregex_iterator end; occurs != end; ++occurs) {
            Container::String matched_str{occurs->str()};
            matched_str = matched_str.substr(matched_str.find("\"") + 1,
                                             matched_str.length() - pos - 2); // remove #include " "
            std::filesystem::path abs_include_path = settings.input_dir / matched_str;
            include_file_list.emplace(abs_include_path);
        }

        // blob = readfile;
    }
    // iterate graph,
    // if leaf_need_recompile
    // compileshaders
    // cached md5 value
    CheckMd5();
    }

    // compile shaders

    for (auto &shader : shader_list) {
        if (!shader.need_compile) {
            continue;
        }

        ShaderCompiler::Compile(shader_texts[shader.shader_text_index], shader.args);
    }
}

void ShaderCompiler::InternalCompile(const Container::String &hlsl_text, const ShaderCompilationArgs &compile_args) {
    IDxcBlobEncoding *hlsl_blob{};
    CHECK_DX_RESULT(idxc_utils->CreateBlob(hlsl_text.data(), static_cast<u32>(hlsl_text.size()), 0, &hlsl_blob));

    auto stack_memory = Memory::GetStackMemoryResource(512);
    Container::Array<LPCWSTR> compilation_arguments(&stack_memory);

    // entry point
    Container::WString ep(&stack_memory);
    ep = {compile_args.entry_point.begin(), compile_args.entry_point.end()};
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

    Container::WString ip(&stack_memory);
    ip = {compile_args.include_path.begin(), compile_args.include_path.end()};
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
    os.open(compile_args.output_file_name, std::ios::binary | std::ios::out);
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
