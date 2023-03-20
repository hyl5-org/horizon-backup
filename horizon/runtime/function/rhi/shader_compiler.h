/*****************************************************************/ /**
 * \file   FILE_NAME
 * \brief  BRIEF
 * 
 * \author XXX
 * \date   XXX
 *********************************************************************/

#pragma once

// standard libraries

// third party libraries
#include <directx-dxc/d3d12shader.h>
#include <directx-dxc/dxcapi.h>

// project headers
#include "runtime/core/singleton/public_singleton.h"
#include "runtime/function/rhi/rhi_utils.h"
#include "runtime/function/rhi/shader.h"

namespace Horizon {

static constexpr Container::FixedArray<char, 4> hsb_header{'h', 's', 'b', '1'};

struct ShaderBinaryHeader {
    Container::FixedArray<char, 4> header;
    u32 shader_blob_offset;
    u32 shader_blob_size;
    //u32 pdb_offset;
};

class ShaderCompiler : public Singleton<ShaderCompiler> {
  public:
    ShaderCompiler() noexcept;
    virtual ~ShaderCompiler() noexcept;

    constexpr ShaderCompiler(const ShaderCompiler &rhs) noexcept = delete;
    constexpr ShaderCompiler &operator=(const ShaderCompiler &rhs) noexcept = delete;
    constexpr ShaderCompiler(ShaderCompiler &&rhs) noexcept = delete;
    constexpr ShaderCompiler &operator=(ShaderCompiler &&rhs) noexcept = delete;

    static void Compile(const Container::String &blob, const ShaderCompilationArgs &compile_args);

    static void CompileShaders(const ShaderCompilationSettings &settings);

  private:
    void InternalCompile(const Container::String &blob, const ShaderCompilationArgs &compile_args);

  private:
    IDxcUtils *idxc_utils;
    IDxcCompiler3 *idxc_compiler;
    IDxcIncludeHandler *idxc_include_handler;
};

} // namespace Horizon