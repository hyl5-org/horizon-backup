#pragma once

#include "runtime/function/rhi/buffer.h"
#include "runtime/function/rhi/descriptor_set.h"
#include "runtime/function/rhi/rhi_utils.h"
#include "runtime/function/rhi/sampler.h"
#include "runtime/function/rhi/shader.h"
#include "runtime/function/rhi/texture.h"

namespace Horizon::Backend {

class Pipeline {
  public:
    Pipeline() noexcept;
    virtual ~Pipeline() noexcept;

    Pipeline(const Pipeline &rhs) noexcept = delete;
    Pipeline &operator=(const Pipeline &rhs) noexcept = delete;
    Pipeline(Pipeline &&rhs) noexcept = delete;
    Pipeline &operator=(Pipeline &&rhs) noexcept = delete;

    PipelineType GetType() const noexcept;

    virtual void SetComputeShader(Shader *vs) = 0;
    virtual void SetGraphicsShader(Shader *vs, Shader *ps) = 0;

    virtual DescriptorSet *GetDescriptorSet(ResourceUpdateFrequency frequency) = 0;

  protected:
    //void ParseRootSignature(); 
    //virtual void ParseRootSignatureFromShader(Shader* shader) = 0;  

  protected:
    // array contain all kinds of shaders
    Container::FixedArray<Shader *, static_cast<u64>(ShaderType::SHADER_TYPE_COUNT)> shaders{};
    PipelineCreateInfo m_create_info{};
};
} // namespace Horizon::Backend
