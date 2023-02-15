
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "config.h"

namespace TEST::ShaderCompilationTest {

using namespace Horizon;

class ShaderCompilationTest {
  public:
    ShaderCompilationTest() {}

  public:
};


TEST_CASE_FIXTURE(ShaderCompilationTest, "dependency") {
    ShaderCompilationSettings settings{};
    settings.input_dir = "C:/FILES/horizon/horizon/assets/hlsl";
    settings.output_dir = "C:/FILES/horizon/horizon/assets/hlsl/generated";
    settings.optimization_level = ShaderOptimizationLevel::DEBUG;
    settings.sm_version = ShaderModuleVersion::SM_6_6;
    settings.target_api = ShaderTargetAPI::SPIRV;
    Container::Array<std::filesystem::path> list;
    list.push_back("C:/FILES/horizon/horizon/assets/hlsl/ssao.comp.hlsl");
    list.push_back("C:/FILES/horizon/horizon/assets/hlsl/ssao_blur.comp.hlsl");
    list.push_back("C:/FILES/horizon/horizon/assets/hlsl/taa.comp.hlsl");
    list.push_back("C:/FILES/horizon/horizon/assets/hlsl/post_process.comp.hlsl");
    list.push_back("C:/FILES/horizon/horizon/assets/hlsl/geometry.gfx.hlsl");
    list.push_back("C:/FILES/horizon/horizon/assets/hlsl/shading.comp.hlsl");
    list.push_back("C:/FILES/horizon/horizon/assets/hlsl/gpu_mesh_culling.comp.hlsl");
    list.push_back("C:/FILES/horizon/horizon/assets/hlsl/gpu_triangle_culling.comp.hlsl");
    list.push_back("C:/FILES/horizon/horizon/assets/hlsl/compact_index_buffer.comp.hlsl");
    list.push_back("C:/FILES/horizon/horizon/assets/hlsl/test0.comp.hlsl");
    for (u32 i = 0; i < 303; i++) {
        std::string path = "C:/FILES/horizon/horizon/assets/hlsl/test0.comp copy " + std::to_string(i) + ".hlsl";
        list.push_back(path);
    }
    settings.shader_list = std::move(list);

    auto tp1 = std::chrono::high_resolution_clock::now();

    ShaderCompiler::CompileShaders(settings);
    auto tp2 = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(tp2 - tp1).count();

    LOG_INFO("spend {} ms to copmile {} shader", dur, settings.shader_list.size());
    // threading
}

// threading

// dependency

} // namespace TEST::ShaderCompilationTest