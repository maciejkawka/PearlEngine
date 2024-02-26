#include"Core/Common/pearl_pch.h"
#include"Renderer/OpenGL/GLMesh.h"
#include"Renderer/OpenGl/GLVertexArray.h"
#include"Renderer/OpenGl/GLVertexBuffer.h"
#include"Renderer/OpenGl/GLIndexBuffer.h"
#include"Renderer/Core/Defines.h"

#include"Core/Filesystem/FileSystem.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include"tinyObj/tiny_obj_loader.h"

using namespace PrRenderer::OpenGL;

//Hash Function for map
template <typename T, typename... Rest>
void hash_combine(std::size_t& seed, const T& v, const Rest&... rest)
{
    seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hash_combine(seed, rest), ...);
}

struct Vertex
{
    PrCore::Math::vec3 pos;
    PrCore::Math::vec3 normal;
    PrCore::Math::vec2 uv;
    PrRenderer::Core::Color color;

    Vertex()
    {
        pos = PrCore::Math::vec3(0.f);
        normal = PrCore::Math::vec3(0.f);
        uv = PrCore::Math::vec2(0.f);
        color = PrRenderer::Core::Color::Black;
    }

    bool operator==(const Vertex& vertex) const
    {
        return vertex.uv == this->uv && vertex.pos == this->pos &&
            vertex.normal == this->normal && vertex.color == this->color;
    }
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            size_t seed = 0;
            hash_combine(seed, vertex.pos, (PrCore::Math::vec3)vertex.color, vertex.uv, vertex.normal);
            return seed;
        }
    };
}

void GLMesh::Bind()
{
    if (m_stateChanged)
        UpdateBuffers();
    m_VA->Bind();
}

void GLMesh::Unbind()
{
    m_VA->Unbind();
}

void GLMesh::RecalculateNormals()
{
    m_normals.clear();

    m_normals = CalculateNormals();
}

void GLMesh::RecalculateTangents()
{
    m_tangents.clear();

    m_tangents = CalculateTangents();
}

void GLMesh::PreLoadImpl()
{
}

bool GLMesh::LoadImpl()
{
    auto extension = m_name.substr(m_name.find_last_of("."));
    
    if (extension == ".obj")
    {
        if (!PopulateOBJ())
            return false;
    }
    else
    {
        PRLOG_ERROR("Renderer: Mesh {0} extension {1} not supported", m_name, extension);
        return false;
    }

    if (!ValidateBuffers())
    {
        PRLOG_ERROR("Renderer: Mesh {0} invalid", m_name);
        return false;
    }

    UpdateBuffers();
    return true;
}

void GLMesh::PostLoadImpl()
{
}

void GLMesh::PreUnloadImpl()
{
}

bool GLMesh::UnloadImpl()
{
    m_indices.clear();
    m_indicesCount = 0;

    m_vertices.clear();
    m_verticesCount = 0;

    m_colors.clear();
    m_normals.clear();
    m_tangents.clear();

    for(auto& uv : m_UVs)
        uv.clear();

    m_VA.reset();

    return true;
}

void GLMesh::PostUnloadImpl()
{
}

void GLMesh::LoadCorruptedResource()
{
}

void GLMesh::CalculateSize()
{
    m_size = sizeof(m_indicesCount) + sizeof(unsigned int) * m_indices.size() +
        sizeof(PrCore::Math::vec3) * m_vertices.size() + sizeof(m_vertices) +
        sizeof(PrCore::Math::vec3) * m_normals.size() +
        sizeof(PrCore::Math::vec3) * m_tangents.size() +
        sizeof(PrCore::Math::vec4) * m_colors.size() +
        sizeof(m_maxUVs) + sizeof(m_stateChanged);

    for (int i = 0; i < m_maxUVs; i++)
        m_size += sizeof(PrCore::Math::vec2) * m_UVs[i].size();
}

void GLMesh::UpdateBuffers()
{
    auto vertexBuffer = Buffers::VertexBuffer::Create();
    auto indexBuffer = Buffers::IndexBuffer::Create();
    Buffers::BufferLayout bufferLayout;
    
    //Vertex
    bufferLayout.AddElementBuffer({ "Vertex", Buffers::ShaderDataType::Float3 });
    
    //Normals
    if (m_normals.empty() && m_normals.size() > 2)
        m_normals = CalculateNormals(); 
    bufferLayout.AddElementBuffer({ "Normals", Buffers::ShaderDataType::Float3 });

    //Tangents
    if (m_tangents.empty() && m_normals.size() > 2)
        m_tangents = CalculateTangents();
    bufferLayout.AddElementBuffer({ "Tangents", Buffers::ShaderDataType::Float4 });
    
    //UVs
    for (int i = 0; i < m_maxUVs; i++)
        if (!m_UVs[i].empty())
            bufferLayout.AddElementBuffer({ ("UV" + std::to_string(i)), Buffers::ShaderDataType::Float2 });
   
    //Colors
    if (!m_colors.empty())
        bufferLayout.AddElementBuffer({ "Color", Buffers::ShaderDataType::Float4 });

    //Create VertexBuffer
    size_t bufferSize = bufferLayout.GetFloatStride() * m_verticesCount;
    std::vector<float> bufferVector;

    for (int i = 0; i < m_verticesCount; i++)
    {
        bufferVector.push_back(m_vertices[i].x);
        bufferVector.push_back(m_vertices[i].y);
        bufferVector.push_back(m_vertices[i].z);

        if (!m_normals.empty())
        {
            bufferVector.push_back(m_normals[i].x);
            bufferVector.push_back(m_normals[i].y);
            bufferVector.push_back(m_normals[i].z);
        }

        if (!m_tangents.empty())
        {
            bufferVector.push_back(m_tangents[i].x);
            bufferVector.push_back(m_tangents[i].y);
            bufferVector.push_back(m_tangents[i].z);
            bufferVector.push_back(m_tangents[i].w);
        }

        for (int j = 0; j < m_maxUVs; j++)
        {
            if (!m_UVs[j].empty())
            {
                bufferVector.push_back(m_UVs[j][i].x);
                bufferVector.push_back(m_UVs[j][i].y);
            }
        }


        if (!m_colors.empty())
        {
            bufferVector.push_back(m_colors[i].r);
            bufferVector.push_back(m_colors[i].g);
            bufferVector.push_back(m_colors[i].b);
            bufferVector.push_back(m_colors[i].a);
        }
    }

    //Pass Vertices to GPU
    vertexBuffer->SetBufferLayout(bufferLayout);
    vertexBuffer->SetData(static_cast<void*>(bufferVector.data()), bufferVector.size());
    indexBuffer->SetIndeces(static_cast<void*>(m_indices.data()), m_indicesCount);

    m_VA.reset(new GLVertexArray());
    m_VA->SetIndexBuffer(indexBuffer);
    m_VA->SetVertexBuffer(vertexBuffer);

    m_boxVolume = Core::BoxVolume(m_vertices);

    m_stateChanged = false;
}

bool GLMesh::PopulateOBJ()
{
    std::string dir = MESH_DIR;
    dir += ("/" + m_name);
    PrCore::Filesystem::FileStreamPtr file = PrCore::Filesystem::FileSystem::GetInstance().OpenFileStream(dir.c_str());
    if (file == nullptr)
        return false;

    char* data = new char[file->GetSize()];
    file->Read(data);
    std::string objFile = std::string(data, file->GetSize());
    delete[] data;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromString(objFile, std::string(), tinyobj::ObjReaderConfig()))
    {
        PRLOG_ERROR("Renderer: Cannot open mesh file {0}", dir);
        return false;
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();

    std::unordered_map<Vertex, unsigned int> vertexMap;

    for (const auto& shape : shapes)
    {
        for (const auto& face : shape.mesh.num_face_vertices)
        {
            if (face != 3)
            {
                PRLOG_ERROR("Renderer: PearlEngine supports only triangle mesh error in mesh {0}", m_name);
                return false;
            }
        }

        for (const auto& index : shape.mesh.indices)
        {
            Vertex vert;

            PrCore::Math::vec3 pos(0.f);
            pos.x = attrib.vertices[3 * index.vertex_index + 0];
            pos.y = attrib.vertices[3 * index.vertex_index + 1];
            pos.z = attrib.vertices[3 * index.vertex_index + 2];
            vert.pos = pos;

            if (!attrib.texcoords.empty())
            {
                PrCore::Math::vec2 uvs(0.f);
                uvs.x = attrib.texcoords[2 * index.texcoord_index + 0];
                uvs.y = attrib.texcoords[2 * index.texcoord_index + 1];
                vert.uv = uvs;
            }

            if (!attrib.normals.empty())
            {
                PrCore::Math::vec3 normal(0.f);
                normal.x = attrib.normals[3 * index.normal_index + 0];
                normal.y = attrib.normals[3 * index.normal_index + 1];
                normal.z = attrib.normals[3 * index.normal_index + 2];
                vert.normal = normal;
            }

            PrRenderer::Core::Color color(PrRenderer::Core::Color::White);
            if (!attrib.colors.empty())
            {
                color.r = attrib.colors[3 * index.vertex_index + 0];
                color.g = attrib.colors[3 * index.vertex_index + 1];
                color.b = attrib.colors[3 * index.vertex_index + 2];
            }
            vert.color = color;

            if (vertexMap.find(vert) == vertexMap.end())
            {
                m_vertices.push_back(vert.pos);

                if (!attrib.texcoords.empty())
                    m_UVs[0].push_back(vert.uv);

                if (!attrib.normals.empty())
                    m_normals.push_back(vert.normal);

                if (!attrib.colors.empty())
                    m_colors.push_back(vert.color);

                vertexMap[vert] = (unsigned int)m_vertices.size() - 1;
            }

            m_indices.push_back(vertexMap[vert]);
        }
    }

    m_indicesCount = m_indices.size();
    m_verticesCount = m_vertices.size();

    return true;
}
