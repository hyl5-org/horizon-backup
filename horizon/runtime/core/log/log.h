/*****************************************************************//**
 * \file   log.h
 * \brief  log functions
 * 
 * \author hylu
 * \date   November 2022
 *********************************************************************/

#pragma once

// standard libraries

// third party libraries
#include <d3d12.h>
#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>

// project headers
#include "runtime/core/singleton/public_singleton.h"
#include "runtime/core/utils/definations.h"
#include "runtime/core/container/container.h"

namespace Horizon {

class Log : public Singleton<Log> {
  public:
    enum loglevel : u8 { debug, info, warn, error, fatal };

  public:
    Log() noexcept;
    ~Log() noexcept;

    template <typename... args> inline void Debug(args &&..._args) const noexcept {
        m_logger->debug(std::forward<args>(_args)...);
    }

    template <typename... args> inline void Info(args &&..._args) const noexcept {
        m_logger->info(std::forward<args>(_args)...);
    }

    template <typename... args> inline void Warn(args &&..._args) const noexcept {
        m_logger->warn(std::forward<args>(_args)...);
    }

    template <typename... args> inline void Error(args &&..._args) const noexcept {
        m_logger->error(std::forward<args>(_args)...);
    }
    template <typename... args> inline void Fatal(args &&..._args) const noexcept {
        // m_logger->fatal(std::forward<args>(_args)...);
    }
    void CheckVulkanResult(VkResult _res, const char *func_name, int line) const noexcept;

    void CheckDXResult(HRESULT hr, const char *func_name, int line) const noexcept;

  private:
    std::shared_ptr<spdlog::logger> m_logger;
};

#define LOG_DEBUG(...) Log::get().Debug("[" + Container::String(__FUNCTION__) + "] " + __VA_ARGS__);

#define LOG_INFO(...) Log::get().Info("[" + Container::String(__FUNCTION__) + "] " + __VA_ARGS__);

#define LOG_WARN(...) Log::get().Warn("[" + Container::String(__FUNCTION__) + "] " + __VA_ARGS__);

#define LOG_ERROR(...) Log::get().Error("[" + Container::String(__FUNCTION__) + "] " + __VA_ARGS__);

#define LOG_FATAL(...) Log::get().Fatal("[" + Container::String(__FUNCTION__) + "] " + __VA_ARGS__);

#define CHECK_VK_RESULT(res) Log::get().CheckVulkanResult(res, __FUNCTION__, __LINE__);

#define CHECK_DX_RESULT(res) Log::get().CheckDXResult(res, __FUNCTION__, __LINE__);

} // namespace Horizon
