#include "ssao.h"
extern Container::HashMap<ShaderList, Container::Array<u8>> shader_map;

SSAOData::SSAOData(Backend::RHI *rhi) noexcept {

    ssao_cs = rhi->CreateShader(ShaderType::COMPUTE_SHADER, shader_map[ShaderList::SSAO_CS]);
    ssao_blur_cs = rhi->CreateShader(ShaderType::COMPUTE_SHADER, shader_map[ShaderList::SSAO_BLUR_CS]);

    // AO PASS
    {
        ssao_pass = rhi->CreateComputePipeline(ComputePipelineCreateInfo{});
        ssao_blur_pass = rhi->CreateComputePipeline({});
    }

    ssao_factor_texture = rhi->CreateTexture(TextureCreateInfo{
        DescriptorType::DESCRIPTOR_TYPE_RW_TEXTURE, ResourceState::RESOURCE_STATE_UNORDERED_ACCESS,
        TextureType::TEXTURE_TYPE_2D, TextureFormat::TEXTURE_FORMAT_RGBA8_UNORM, _width, _height, 1, false});

    ssao_noise_tex = rhi->CreateTexture(
        TextureCreateInfo{DescriptorType::DESCRIPTOR_TYPE_TEXTURE, ResourceState::RESOURCE_STATE_SHADER_RESOURCE,
                          TextureType::TEXTURE_TYPE_2D, TextureFormat::TEXTURE_FORMAT_RG32_SFLOAT, SSAO_NOISE_TEX_WIDTH,
                          SSAO_NOISE_TEX_HEIGHT, 1, false});

    ssao_blur_texture = rhi->CreateTexture(TextureCreateInfo{
        DescriptorType::DESCRIPTOR_TYPE_RW_TEXTURE, ResourceState::RESOURCE_STATE_UNORDERED_ACCESS,
        TextureType::TEXTURE_TYPE_2D, TextureFormat::TEXTURE_FORMAT_RGBA8_UNORM, _width, _height, 1, false});

    ssao_constants_buffer =
        rhi->CreateBuffer(BufferCreateInfo{DescriptorType::DESCRIPTOR_TYPE_CONSTANT_BUFFER,
                                           ResourceState::RESOURCE_STATE_SHADER_RESOURCE, sizeof(SSAOConstant)});

    std::uniform_real_distribution<float> rnd_dist(0.0, 1.0); // random floats between [0.0, 1.0]
    std::default_random_engine generator;

    for (unsigned int i = 0; i < SSAO_KERNEL_SIZE; ++i) {
        math::Vector3f sample(rnd_dist(generator) * 2.0 - 1.0, rnd_dist(generator) * 2.0 - 1.0, rnd_dist(generator));
        sample = math::Normalize(sample);
        sample *= rnd_dist(generator);
        float scale = float(i) / float(SSAO_KERNEL_SIZE);
        sample *= Lerp(0.1f, 1.0f, scale * scale);
        ssao_constansts.kernels[i] = math::Vector4f(sample, 1.0);
    }
    // ssao noise tex
    for (u32 i = 0; i < ssao_noise_tex_val.size(); i++) {
        ssao_noise_tex_val[i] = math::Vector2f(rnd_dist(generator) * 2.0f - 1.0f, rnd_dist(generator) * 2.0f - 1.0f);
    }
    char *begin = reinterpret_cast<char *>(&ssao_noise_tex_val[0]);
    char *end = reinterpret_cast<char *>(&ssao_noise_tex_val[ssao_noise_tex_val.size() - 1]);
    ssao_noise_tex_data_desc.raw_data = {begin, end};

    ssao_pass->SetShader(ssao_cs);
    ssao_blur_pass->SetShader(ssao_blur_cs);

}

SSAOData::SSAOData() noexcept {}
