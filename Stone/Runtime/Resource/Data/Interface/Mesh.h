#pragma once
#include <string>
#include <vector>
#include <map>
#include <Function/Render/VertexArray.h>
namespace Stone
{
#define VERTEX_ONLY 0
#define VERTEX_WITH_BONE 1

#define MAX_BONENUM_PER_VERTEX 4
	struct Vertex
	{
        float px;
        float py;
        float pz;
        float nx;
        float ny;
        float nz;
        float u;
        float v;
        int32_t boneIDs[MAX_BONENUM_PER_VERTEX] = { 0 };
        float weights[MAX_BONENUM_PER_VERTEX] = { 0.0 };
        float type = VERTEX_ONLY;

        void addBoneData(uint32_t boneID, float weight)
        {
            for (size_t i = 0; i < MAX_BONENUM_PER_VERTEX; i++) {
                if (weights[i] == 0.0) {
                    boneIDs[i] = boneID;
                    weights[i] = weight;
                    return;
                }
            }

            // should never get here - more bones than we have space for
            assert(0);
        }
	};

    class Mesh
    {
    public:
        Mesh() = default;
        Mesh(const std::string& path);
        virtual void update();
        virtual void updateBuffer() {}
        virtual ~Mesh();

        

    public:
        std::shared_ptr<VertexArray> m_VAO = nullptr;
        std::shared_ptr<VertexBuffer> m_VBO = nullptr;
        std::shared_ptr<IndexBuffer> m_EBO = nullptr;
        std::vector<Vertex> m_V;
        std::vector<uint32_t> m_I;
        std::string m_Path;
        std::string m_Directory;
        std::string m_MeshName;

        bool operator==(const Mesh& other) const { return other.m_Path == m_Path; }
    };
}