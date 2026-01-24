#pragma once

#include "Renderer/Resources/Material.h"
#include "Renderer/Resources/Mesh.h"

namespace PrRenderer::Core {

    struct MeshRenderObject {

        MeshPtr     mesh;
        MaterialPtr material;

        PrCore::Math::mat4 worldMat;
        PrCore::Math::vec3 position;

        bool castShadow = false;
    };

    struct InstancedMeshObject {

        MeshPtr     mesh;
        MaterialPtr material;

        size_t i                        nstanceCount;
        std::vector<PrCore::Math::mat4> wordMatrices;

        bool castShadow = false;
    };
}