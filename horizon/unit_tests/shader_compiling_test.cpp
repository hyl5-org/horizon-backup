
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
    //list.push_back("C:/FILES/horizon/horizon/assets/hlsl/ssao.comp.hlsl");
    //list.push_back("C:/FILES/horizon/horizon/assets/hlsl/ssao_blur.comp.hlsl");
    //list.push_back("C:/FILES/horizon/horizon/assets/hlsl/taa.comp.hlsl");
    //list.push_back("C:/FILES/horizon/horizon/assets/hlsl/post_process.comp.hlsl");
    list.push_back("C:/FILES/horizon/horizon/assets/hlsl/geometry.gfx.hlsl");
    list.push_back("C:/FILES/horizon/horizon/assets/hlsl/shading.comp.hlsl");
    list.push_back("C:/FILES/horizon/horizon/assets/hlsl/gpu_mesh_culling.comp.hlsl");
    list.push_back("C:/FILES/horizon/horizon/assets/hlsl/gpu_triangle_culling.comp.hlsl");
    list.push_back("C:/FILES/horizon/horizon/assets/hlsl/compact_index_buffer.comp.hlsl");
    settings.shader_list = std::move(list);
    ShaderCompiler::CompileShaders(settings);
    // threading
}

// threading

// dependency

} // namespace TEST::ShaderCompilationTest