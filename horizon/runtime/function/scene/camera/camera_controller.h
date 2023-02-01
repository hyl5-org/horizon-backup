/*****************************************************************//**
 * \file   camera_controller.h
 * \brief  
 * 
 * \author hylu
 * \date   November 2022
 *********************************************************************/

#pragma once

// standard libraries

// third party libraries

// project headers

#include "runtime/core/math/hmath.h"
#include "runtime/core/window/window.h"
#include "runtime/function/scene/camera/camera.h"

namespace Horizon {

class CameraController {
  public:
    CameraController(Camera *camera) noexcept;

    ~CameraController() noexcept;

    CameraController(const CameraController &rhs) noexcept = delete;

    CameraController &operator=(const CameraController &rhs) noexcept = delete;

    CameraController(CameraController &&rhs) noexcept = delete;

    CameraController &operator=(CameraController &&rhs) noexcept = delete;

  public:
    void ProcessInput(Window* window);

  private:
    // Window *m_window = nullptr;
    Camera *m_camera = nullptr;
};

} // namespace Horizon