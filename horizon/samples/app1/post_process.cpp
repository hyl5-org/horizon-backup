#include "post_process.h"

extern Container::HashMap<ShaderList, Container::Array<u8>> shader_map;

PostProcessData::PostProcessData(Backend::RHI *rhi) noexcept {
    // PP PASS
    post_process_cs = rhi->CreateShader(ShaderType::COMPUTE_SHADER, shader_map[ShaderList::POST_PROCESS_CS]);
    post_process_pass = rhi->CreateComputePipeline(ComputePipelineCreateInfo{});
    pp_color_texture = rhi->CreateTexture(TextureCreateInfo{
        DescriptorType::DESCRIPTOR_TYPE_RW_TEXTURE, ResourceState::RESOURCE_STATE_UNORDERED_ACCESS,
        TextureType::TEXTURE_TYPE_2D, TextureFormat::TEXTURE_FORMAT_RGBA8_UNORM, _width, _height, 1, false});
    exposure_constants_buffer =
        rhi->CreateBuffer(BufferCreateInfo{DescriptorType::DESCRIPTOR_TYPE_CONSTANT_BUFFER,
                                           ResourceState::RESOURCE_STATE_SHADER_RESOURCE, sizeof(ExposureConstant)});
    post_process_pass->SetShader(post_process_cs);
}
