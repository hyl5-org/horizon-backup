project(third_party)

set(third_party_folder "third_party")

add_library(${PROJECT_NAME} 3rd_party.cpp)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Horizon/ThirdParty")



# inih

# find_path(INIH_INCLUDE_DIRS "ini.h")
# get_filename_component(INIH_LIB_DIR ${INIH_INCLUDE_DIRS} DIRECTORY)
# set(INIH_LIB "${INIH_LIB_DIR}/lib/inih.lib")

# target_link_libraries(${PROJECT_NAME} PUBLIC ${INIH_LIB})

# directx 12

#target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/dx12)

# find vulkan
find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES})

# glfw
find_package(glfw3 REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)


# directxtk
# find_package(directxtk12 REQUIRED)
# target_link_libraries(${PROJECT_NAME} PUBLIC Microsoft::DirectXTK12)


add_subdirectory(SPIRV-Cross)
target_link_libraries(${PROJECT_NAME} PRIVATE spirv-cross-core spirv-cross-reflect)

# directx shader compiler
# find_package(directx-dxc REQUIRED)
# target_link_directories(${PROJECT_NAME} PUBLIC )

# target_link_libraries(${PROJECT_NAME} PUBLIC dxc)
# find_package(directx-dxc REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC DirectXShaderCompiler/include)
target_link_directories(${PROJECT_NAME} PUBLIC DirectXShaderCompiler/lib)
target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/DirectXShaderCompiler/lib/dxcompiler.lib)

# spdlog
find_package(spdlog REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog spdlog::spdlog_header_only)

# vma
find_package(unofficial-vulkan-memory-allocator REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC unofficial::vulkan-memory-allocator::vulkan-memory-allocator)
# d3dma
# find_package(d3d12-memory-allocator REQUIRED)
# target_link_libraries(${PROJECT_NAME} PUBLIC unofficial::d3d12-memory-allocator)

# doctest
find_package(doctest CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC doctest::doctest)

find_path(STB_INCLUDE_DIRS "stb_c_lexer.h")
target_include_directories(${PROJECT_NAME} PUBLIC ${STB_INCLUDE_DIRS})

find_package(assimp CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE assimp::assimp)

# find_package(EnTT CONFIG REQUIRED)
# target_link_libraries(${PROJECT_NAME} PRIVATE EnTT::EnTT)

find_package(TBB CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE TBB::tbb)

find_package(meshoptimizer CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE meshoptimizer::meshoptimizer)

# find_package(imgui CONFIG REQUIRED)
# target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)

# find_package(unofficial-nativefiledialog CONFIG REQUIRED)
# target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::nativefiledialog::nfd)

# find_path(CGLTF_INCLUDE_DIRS "cgltf.h")
# target_include_directories(${PROJECT_NAME} PRIVATE ${CGLTF_INCLUDE_DIRS})

find_package(mimalloc CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE mimalloc mimalloc)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

find_package(cryptopp CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE cryptopp::cryptopp)

# find_package(unofficial-spirv-reflect CONFIG REQUIRED)
# target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::spirv-reflect::spirv-reflect)

