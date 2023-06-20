#pragma once

#include "Renderer/Resources/Material.h"
#include "Renderer/Resources/Mesh.h"

namespace PrRenderer::Core {

    struct MeshRenderObject {
        MeshRenderObject() = default;

        Resources::MeshPtr mesh;
        Resources::MaterialPtr material;

        PrCore::Math::mat4 worldMat;
        PrCore::Math::vec3 position;

        bool castShadow = false;
    };
}