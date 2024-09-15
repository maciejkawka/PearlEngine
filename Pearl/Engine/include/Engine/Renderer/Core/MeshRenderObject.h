#pragma once

#include "Renderer/Resources/Material.h"
#include "Renderer/Resources/Mesh.h"

namespace PrRenderer::Core {

    struct MeshRenderObject {

        Resources::MeshPtr mesh;
        Resources::MaterialPtr material;

        PrCore::Math::mat4 worldMat;
        PrCore::Math::vec3 position;

        bool castShadow = false;
    };

    struct InstancedMeshObject {

        Resources::MeshPtr mesh;
        Resources::MaterialPtr material;

        size_t instanceCount;
        std::vector<PrCore::Math::mat4> wordMatrices;

        bool castShadow = false;
    };
}