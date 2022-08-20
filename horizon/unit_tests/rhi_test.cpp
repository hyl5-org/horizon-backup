#include "config.h"

#include <doctest/doctest.h>

namespace TEST::RHI {
using namespace Horizon;

class RHITest {
  public:
    RHITest() {
        EngineConfig config{};
        config.width = 800;
        config.height = 600;
        // config.asset_path =
        config.render_backend = RenderBackend::RENDER_BACKEND_VULKAN;
        config.offscreen = false;
        engine = std::make_unique<Engine>(config);

        width = config.width;
        height = config.height;
    }

  public:
    std::unique_ptr<Engine> engine{};
    std::string asset_path = "D:/codes/horizon/horizon/assets/";
    u32 width, height;
};

TEST_CASE_FIXTURE(RHITest, "buffer creation test") {

    auto rhi = engine->m_render_system->GetRhi();
    rhi->CreateBuffer(BufferCreateInfo{DescriptorType::DESCRIPTOR_TYPE_CONSTANT_BUFFER,
                                       ResourceState::RESOURCE_STATE_SHADER_RESOURCE, 32});
}

TEST_CASE_FIXTURE(RHITest, "texture creation test") {

    auto rhi = engine->m_render_system->GetRhi();
    rhi->CreateTexture(TextureCreateInfo{DescriptorType::DESCRIPTOR_TYPE_TEXTURE,
                                         ResourceState::RESOURCE_STATE_SHADER_RESOURCE, TextureType::TEXTURE_TYPE_2D,
                                         TextureFormat::TEXTURE_FORMAT_RGBA8_UNORM, 4, 4, 1});
}

TEST_CASE_FIXTURE(RHITest, "buffer upload, dynamic") {

    auto rhi = engine->m_render_system->GetRhi();

    Resource<Buffer> buffer{
        rhi->CreateBuffer(BufferCreateInfo{DescriptorType::DESCRIPTOR_TYPE_CONSTANT_BUFFER,
                                           ResourceState::RESOURCE_STATE_SHADER_RESOURCE, sizeof(Math::float3)})};

    // dynamic buffer, cpu pointer not change, cpu data change, gpu data
    // change
    for (u32 i = 0; i < 10; i++) {
        engine->BeginNewFrame();

        Math::float3 data{static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                          static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                          static_cast<float>(rand()) / static_cast<float>(RAND_MAX)};

        auto transfer = rhi->GetCommandList(CommandQueueType::TRANSFER);

        transfer->BeginRecording();

        // cpu -> stage
        transfer->UpdateBuffer(buffer.get(), &data, sizeof(data));

        transfer->EndRecording();

        // stage -> gpu
        rhi->SubmitCommandLists(CommandQueueType::TRANSFER, std::vector{transfer});
        engine->EndFrame();
    }
}

TEST_CASE_FIXTURE(RHITest, "shader compile test") {

    auto rhi = engine->m_render_system->GetRhi();
    std::string file_name = asset_path + "shaders/hlsl/shader.hlsl";
    auto shader_program = rhi->CreateShaderProgram(ShaderType::VERTEX_SHADER, "vs_main", 0, file_name);
    rhi->DestroyShaderProgram(shader_program);
}

TEST_CASE_FIXTURE(RHITest, "spirv shader reflection test") {

    auto rhi = engine->m_render_system->GetRhi();
    std::string file_name = asset_path + "shaders/hlsl/"
                                         "ps_descriptor_set_reflect.hlsl";
    auto shader_program = rhi->CreateShaderProgram(ShaderType::COMPUTE_SHADER, "cs_main", 0, file_name);
    rhi->DestroyShaderProgram(shader_program);
}

TEST_CASE_FIXTURE(RHITest, "pipeline creation test") {}

TEST_CASE_FIXTURE(RHITest, "dispatch test") {

    auto rhi = engine->m_render_system->GetRhi();
    // Horizon::RDC::StartFrameCapture();
    std::string file_name = asset_path + "shaders/hlsl/cs.hlsl";
    auto shader{rhi->CreateShaderProgram(ShaderType::COMPUTE_SHADER, "cs_main", 0, file_name)};
    ComputePipelineCreateInfo info;
    auto pipeline = rhi->CreateComputePipeline(info);

    auto cl = rhi->GetCommandList(CommandQueueType::COMPUTE);
    pipeline->SetComputeShader(shader);
    cl->BeginRecording();
    cl->BindPipeline(pipeline);
    cl->Dispatch(1, 1, 1);
    cl->EndRecording();

    rhi->SubmitCommandLists(COMPUTE, std::vector{cl});
    // Horizon::RDC::EndFrameCapture();
    engine->EndFrame();
}

TEST_CASE_FIXTURE(RHITest, "descriptor set cache") {

    auto rhi = engine->m_render_system->GetRhi();
    Horizon::RDC::StartFrameCapture();
    std::string file_name = asset_path + "shaders/hlsl/cs_descriptor_set_cache.hlsl";

    auto shader = rhi->CreateShaderProgram(ShaderType::COMPUTE_SHADER, "cs_main", 0, file_name);

    ComputePipelineCreateInfo info;
    auto pipeline = rhi->CreateComputePipeline(info);

    Resource<Buffer> cb1{rhi->CreateBuffer(BufferCreateInfo{
        DescriptorType::DESCRIPTOR_TYPE_CONSTANT_BUFFER, ResourceState::RESOURCE_STATE_SHADER_RESOURCE, sizeof(f32)})};

    Resource<Buffer> cb2{rhi->CreateBuffer(BufferCreateInfo{
        DescriptorType::DESCRIPTOR_TYPE_CONSTANT_BUFFER, ResourceState::RESOURCE_STATE_SHADER_RESOURCE, sizeof(f32)})};
    Resource<Buffer> cb3{rhi->CreateBuffer(BufferCreateInfo{
        DescriptorType::DESCRIPTOR_TYPE_CONSTANT_BUFFER, ResourceState::RESOURCE_STATE_SHADER_RESOURCE, sizeof(f32)})};

    Resource<Buffer> cb4{rhi->CreateBuffer(BufferCreateInfo{
        DescriptorType::DESCRIPTOR_TYPE_CONSTANT_BUFFER, ResourceState::RESOURCE_STATE_SHADER_RESOURCE, sizeof(f32)})};
    Resource<Buffer> rwb1{rhi->CreateBuffer(BufferCreateInfo{
        DescriptorType::DESCRIPTOR_TYPE_RW_BUFFER, ResourceState::RESOURCE_STATE_SHADER_RESOURCE, sizeof(f32)})};

    Resource<Buffer> rwb2{rhi->CreateBuffer(BufferCreateInfo{
        DescriptorType::DESCRIPTOR_TYPE_RW_BUFFER, ResourceState::RESOURCE_STATE_SHADER_RESOURCE, sizeof(f32)})};
    Resource<Buffer> rwb3{rhi->CreateBuffer(BufferCreateInfo{
        DescriptorType::DESCRIPTOR_TYPE_RW_BUFFER, ResourceState::RESOURCE_STATE_SHADER_RESOURCE, sizeof(f32)})};

    Resource<Buffer> rwb4{rhi->CreateBuffer(BufferCreateInfo{
        DescriptorType::DESCRIPTOR_TYPE_RW_BUFFER, ResourceState::RESOURCE_STATE_SHADER_RESOURCE, sizeof(f32)})};

    f32 data[4] = {5, 6, 7, 7};

    auto transfer = rhi->GetCommandList(CommandQueueType::TRANSFER);

    transfer->BeginRecording();

    // cpu -> stage
    transfer->UpdateBuffer(cb1.get(), &data[0], sizeof(f32)); // 5

    transfer->UpdateBuffer(cb2.get(), &data[1], sizeof(f32)); // 6

    transfer->UpdateBuffer(cb3.get(), &data[2], sizeof(f32)); // 7

    transfer->UpdateBuffer(cb4.get(), &data[3], sizeof(f32)); // 7

    transfer->EndRecording();

    rhi->SubmitCommandLists(CommandQueueType::TRANSFER, std::vector{transfer});

    auto cl = rhi->GetCommandList(CommandQueueType::COMPUTE);

    // barrier trans -> compute

    pipeline->SetComputeShader(shader);

    rhi->SetResource(cb1.get(), pipeline, 0, 0); // set, binding
    rhi->SetResource(cb2.get(), pipeline, 0, 1);
    rhi->SetResource(cb3.get(), pipeline, 2, 0); // set, binding
    rhi->SetResource(cb4.get(), pipeline, 2, 1);
    rhi->SetResource(rwb1.get(), pipeline, 1, 0); // set, binding
    rhi->SetResource(rwb2.get(), pipeline, 1, 1);
    rhi->SetResource(rwb3.get(), pipeline, 3, 0); // set, binding
    rhi->SetResource(rwb4.get(), pipeline, 3, 1);

    cl->BeginRecording();
    cl->BindPipeline(pipeline);

    cl->Dispatch(1, 1, 1);
    cl->EndRecording();

    rhi->SubmitCommandLists(COMPUTE, std::vector{cl});

    Horizon::RDC::EndFrameCapture();
    engine->EndFrame();
}

TEST_CASE_FIXTURE(RHITest, "bindless descriptors") {

    // Horizon::RDC::StartFrameCapture();
    // std::string file_name = "D:/codes/horizon/horizon/assets/shaders/hlsl/"
    //                         "cs_bindless_descriptor.hlsl";
    // auto shader{rhi->CreateShaderProgram(
    //     ShaderType::COMPUTE_SHADER, "cs_main", 0, file_name)};
    // auto pipeline{rhi->CreatePipeline(
    //     PipelineCreateInfo{PipelineType::COMPUTE})};

    // auto buffer = rhi->CreateBuffer(
    //     BufferCreateInfo{BufferUsage::BUFFER_USAGE_UNIFORM_BUFFER |
    //                          BufferUsage::BUFFER_USAGE_DYNAMIC_UPDATE,
    //                      sizeof(Math::float4)});
    ////auto texture;
    // Math::float4 data{5.0f};

    // auto transfer =
    //     rhi->GetCommandList(CommandQueueType::TRANSFER);

    // transfer->BeginRecording();

    //// cpu -> stage
    // transfer->UpdateBuffer(buffer.get(), &data, sizeof(data));

    // transfer->EndRecording();

    //// stage -> gpu
    // rhi->SubmitCommandLists(CommandQueueType::TRANSFER,
    //                                             std::vector{transfer});

    // auto cl =
    //     rhi->GetCommandList(CommandQueueType::COMPUTE);
    // pipeline->SetShader(shader);
    // rhi->SetResource(buffer.get());
    // rhi->UpdateDescriptors();
    // cl->BeginRecording();
    // cl->BindPipeline(pipeline);
    // cl->Dispatch(1, 1, 1);
    // cl->EndRecording();

    // rhi->SubmitCommandLists(COMPUTE, std::vector{cl});
    // Horizon::RDC::EndFrameCapture();
}

TEST_CASE_FIXTURE(RHITest, "multi thread command list recording") {
    auto &tp = engine->tp;
    auto rhi = engine->m_render_system->GetRhi();
    constexpr u32 cmdlist_count = 20;
    std::vector<CommandList *> cmdlists(cmdlist_count);
    std::vector<std::future<void>> results(cmdlist_count);

    std::vector<Resource<Buffer>> buffers;

    for (u32 i = 0; i < cmdlist_count; i++) {
        buffers.emplace_back(
            rhi->CreateBuffer(BufferCreateInfo{DescriptorType::DESCRIPTOR_TYPE_CONSTANT_BUFFER,
                                               ResourceState::RESOURCE_STATE_SHADER_RESOURCE, sizeof(Math::float3)}));

        results[i] = std::move(tp->submit([&rhi, &cmdlists, &buffers, i]() {
            Math::float3 data{static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                              static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                              static_cast<float>(rand()) / static_cast<float>(RAND_MAX)};

            auto transfer = rhi->GetCommandList(CommandQueueType::TRANSFER);

            transfer->BeginRecording();

            // cpu -> stage
            transfer->UpdateBuffer(buffers[i].get(), &data,
                                   sizeof(data)); // TODO: multithread

            transfer->EndRecording();
            // stage -> gpu
            cmdlists[i] = transfer;
        }));
    }

    for (auto &res : results) {
        res.wait();
    }
    LOG_INFO("all task done");

    rhi->SubmitCommandLists(CommandQueueType::TRANSFER, cmdlists);

    engine->EndFrame();
}

TEST_CASE_FIXTURE(RHITest, "draw") {

    auto rhi = engine->m_render_system->GetRhi();
    std::string vs_path = asset_path + "shaders/hlsl/graphics_pass.hlsl";
    std::string ps_path = asset_path + "shaders/hlsl/graphics_pass.hlsl";

    auto vs = rhi->CreateShaderProgram(ShaderType::VERTEX_SHADER, "vs_main", 0, vs_path);

    auto ps = rhi->CreateShaderProgram(ShaderType::PIXEL_SHADER, "ps_main", 0, ps_path);

    GraphicsPipelineCreateInfo info{};
    info.view_port_state.width = width;
    info.view_port_state.height = height;
    info.depth_stencil_state.depth_func = DepthFunc::LESS;

    auto pipeline = rhi->CreateGraphicsPipeline(info);

    Mesh mesh(*rhi, MeshDesc{VertexAttributeType::POSTION | VertexAttributeType::NORMAL});
    // rhi->GetVertexBuffer(mesh);
    mesh.LoadMesh(BasicGeometry::BasicGeometry::CUBE);
    auto vertexbuffer = mesh.GetVertexBuffer();
    auto indexbuffer = mesh.GetIndexBuffer();

    auto view =
        Math::LookAt(Math::float3(0.0f, 0.0f, -5.0f), Math::float3(0.0f, 0.0f, 0.0f), Math::float3(0.0f, 1.0f, 0.0f));
    auto projection = Math::Perspective(90.0f, (float)width / (float)height, 0.1f, 100.0f);
    auto vp = projection * view;
    vp = vp.Transpose();

    auto vp_buffer = rhi->CreateBuffer(BufferCreateInfo{DescriptorType::DESCRIPTOR_TYPE_CONSTANT_BUFFER,
                                                        ResourceState::RESOURCE_STATE_SHADER_RESOURCE, sizeof(vp)});

    for (u32 frame = 0; frame < 3; frame++) {
        engine->BeginNewFrame();

        auto transfer = rhi->GetCommandList(CommandQueueType::TRANSFER);

        transfer->BeginRecording();

        transfer->UpdateBuffer(vp_buffer.get(), &vp, sizeof(vp));
        transfer->EndRecording();

        rhi->SubmitCommandLists(CommandQueueType::TRANSFER, std::vector{transfer});
        rhi->WaitGpuExecution(CommandQueueType::TRANSFER); // wait for upload done

        auto cl = rhi->GetCommandList(CommandQueueType::GRAPHICS);
        pipeline->SetGraphicsShader(vs, ps);

        rhi->SetResource(vp_buffer.get(), pipeline, 0, 0);

        cl->BeginRecording();

        RenderPassBeginInfo begin_info{};
        cl->BeginRenderPass(begin_info);

        cl->BindPipeline(pipeline);
        cl->BindVertexBuffer(1, &vertexbuffer, 0);
        cl->BindIndexBuffer(indexbuffer, 0);

        for (auto &node : mesh.GetNodes()) {
            for (auto &m : node.mesh_primitives) {
                cl->DrawIndexedInstanced(m->index_count, m->index_offset, 0);
            }
        }

        cl->EndRenderPass();
        cl->EndRecording();
        rhi->SubmitCommandLists(GRAPHICS, std::vector{cl});
        // Horizon::RDC::EndFrameCapture();
        engine->EndFrame();
    }
}

} // namespace TEST::RHI