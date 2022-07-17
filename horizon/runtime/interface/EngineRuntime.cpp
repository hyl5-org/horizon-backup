#include "EngineRuntime.h"
#include <thread>
namespace Horizon {

EngineRuntime::EngineRuntime(const EngineConfig &config) noexcept {
    m_window = std::make_unique<Window>("horizon", config.width, config.height);
    m_render_system = std::make_unique<RenderSystem>(
        config.width, config.width, m_window.get(), config.render_backend);
    m_input_system = std::make_unique<InputSystem>(
        m_window.get(), m_render_system->GetMainCamera());
    tp = std::make_unique<ThreadPool>(std::thread::hardware_concurrency() - 1);
}

void EngineRuntime::BeginNewFrame() const noexcept {
    // TODO: wait for gpu execution?
    m_render_system->ResetCommandResources();
}

} // namespace Horizon
