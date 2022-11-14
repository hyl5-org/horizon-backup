![](docs/figs/horizon_224.png)

# [Horizon](https://github.com/v4vendeta/horizon/)

horizon is a real time render framework.

---

[![Discord](https://badgen.net/badge/icon/discord?icon=discord&label)](https://discord.gg/sc33JSBKVQ)

---

# Build From Source

**NOTES:** Horizon is not designed for cross-platform and portability, so build or run correctly on other platform is not guaranteed.

On Windows:

- Vulkan SDK 1.3
- CMake 3.18
- Git
- vcpkg

install the required package with vcpkg, you can refer to ```./horizon/3rd_party/CMakeLists.txt```.

clone the repo with

~~~
git clone https://github.com/v4vendeta/horizon.git
~~~

use CMkae to generate solution file

~~~
cmake . -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
~~~

build and run

# Features

- Physically Based Rendering
  - physical light unit
  - physical camera and exposure
  - pbr shading with energy compensation
  - diffuse irradiance with spherical harmonics
  - prefiltered irradiance enviroment map and split sum approximation
  - (disney principled brdf)
 
![](docs/figs/samples/pbs.png)

- SSAO
  - ssao
  - gaussian blur
 
![](docs/figs/samples/ssao.png)

- Precomputed Atmospheric Scattering
  - Eric Bruneton version
 
![](docs/figs/samples/atmosphere.png)
