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

// project headers
#include "runtime/core/container/container.h"
#include "vector.hpp"
#include "matrix.hpp"

namespace Horizon::math {

enum class Side { LEFT = 0, RIGHT = 1, TOP = 2, BOTTOM = 3, BACK = 4, FRONT = 5 };

class Frustum {
  public:
    Frustum() noexcept;
    ~Frustum() noexcept;

    Frustum(const Frustum &rhs) noexcept = default;
    Frustum &operator=(const Frustum &rhs) noexcept = default;
    Frustum(Frustum &&rhs) noexcept = default;
    Frustum &operator=(Frustum &&rhs) noexcept = default;

    /**
	 * @brief Updates the frustums planes based on a matrix
	 * @param matrix The matrix to update the frustum on
	 */
    void update(const Matrix<4, 4> &matrix);

    /**
	 * @brief Checks if a sphere is inside the Frustum
	 * @param pos The center of the sphere
	 * @param radius The radius of the sphere
	 */
    bool check_sphere(const Vector<3> &pos, f32 radius);

    // bool CheckAABB(); TODO

    const Container::FixedArray<Vector<4>, 6> &get_planes() const;

  private:
    Container::FixedArray<Vector<4>, 6> planes;
};

} // namespace Horizon
