#include "scene.h"

#include <random>

SceneData::SceneData(SceneManager *scene_manager, Backend::RHI *rhi) noexcept {
    m_scene_manager = scene_manager;

    scene_manager->CreateBuiltInResources();

    auto [camera, controller] = m_scene_manager->AddCamera(CameraSetting{ProjectionMode::PERSPECTIVE, CameraType::FLY, true}, math::Vector3f(0.0, 0.0, 10.0_m),
                                             math::Vector3f(0.0, 0.0, 0.0),
                                          math::Vector3f(0.0, 1.0_m, 0.0));

    scene_camera = camera;
    scene_camera_controller = controller;

    scene_camera->SetCameraSpeed(0.1);
    
    scene_camera->SetExposure(16.0f, 1 / 125.0f, 100.0f);

    scene_camera->SetPerspectiveProjectionMatrix(90.0_deg, (float)_width / (float)_height, 0.1f, 100.0f);
    // cam->SetLensProjectionMatrix(18.0, (float)width / (float)height, 0.1f, 100.0f);

    auto sponza = scene_manager->resource_manager->LoadMesh(MeshDesc{VertexAttributeType::POSTION | VertexAttributeType::NORMAL |
                                    VertexAttributeType::UV0 | VertexAttributeType::TANGENT},
                           asset_path / "models/Sponza/glTF/Sponza.gltf");

    auto sphere =
        scene_manager->resource_manager->LoadMesh(MeshDesc{VertexAttributeType::POSTION | VertexAttributeType::NORMAL |
                                                           VertexAttributeType::UV0 | VertexAttributeType::TANGENT},
        asset_path / "models/Cauldron-Media/MetalRoughSpheres/glTF/MetalRoughSpheres.gltf");
    sphere->transform.SetPosition(math::Vector3f{0, 10, 0});

    //auto tree1 = scene_manager->resource_manager->LoadMesh(
    //    MeshDesc{VertexAttributeType::POSTION | VertexAttributeType::NORMAL | VertexAttributeType::UV0 |
    //             VertexAttributeType::TANGENT},
    //    asset_path / "models/PKG_B_Ivy/NewSponza_IvyGrowth_glTF.gltf");

    scene_manager->AddMesh(sponza);
    scene_manager->AddMesh(sphere);
    //scene_manager->AddMesh(tree1);

    scene_manager->CreateMeshResources();

    auto decal02 = scene_manager->resource_manager->LoadDecal(asset_path / "materials/decals/decal02.json");

    scene_manager->AddDecal(decal02);

    scene_manager->CreateDecalResources();
    // light

    std::random_device seed;
    std::ranlux48 engine(seed());
    std::uniform_real_distribution<f32> random_position(-10.0, 10.0);
    std::uniform_real_distribution<f32> random_color(0.5, 1.0);

    for (unsigned int i = 0; i < 64; i++) {
        // calculate slightly random offsets
        float xPos = random_position(engine);
        float yPos = random_position(engine);
        float zPos = random_position(engine) + 2.0f;

        // also calculate random color
        float rColor = random_color(engine);
        float gColor = random_color(engine);
        float bColor = random_color(engine);

        math::Vector3f pos(xPos, yPos, zPos);
        math::Vector3f col(rColor, gColor, bColor);

        scene_manager->AddPointLight(col, 1000000.0_lm, pos, 10.0);
    }
    scene_manager->AddDirectionalLight(math::Vector3f(1.0, 1.0, 1.0), 120000.0_lux, math::Vector3f(0.0, 0.0, -1.0));

    
    scene_manager->CreateLightResources();
    scene_manager->CreateCameraResources();
}
