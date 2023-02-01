#pragma once

#include "runtime/core/utils/definations.h"
#include "vector.hpp"

namespace Horizon::math {

class AABB {
  public:
    AABB() noexcept = default;
    ~AABB() noexcept = default;

    AABB(const AABB &rhs) noexcept = default;
    AABB &operator=(const AABB &rhs) noexcept = default;
    AABB(AABB &&rhs) noexcept = default;
    AABB &operator=(AABB &&rhs) noexcept = default;

  public:
    Vector<3> min;
    Vector<3> max;
};

} // namespace Horizon