#pragma once

#include"Graffiti/base/core.h"
#include "Vertex.h"
#include"Graffiti/base/Texture.h"
namespace Graffiti {

    struct BoundingBox {
            glm::vec3 minPos{ FLT_MAX }; 
            glm::vec3 maxPos{ -FLT_MAX };
    };

    struct Material {
        // ���� ID��OpenGL Texture Handle �� Vulkan ImageView Index��
        int baseColorTexID = -1;  // Albedo/Base Color
        int normalTexID = -1;  // Normal Map
        int metallicRoughTexID = -1; // Metallic-Roughness (ARM)
        int occlusionTexID = -1;  // AO
        int emissiveTexID = -1;  // �Է���
    };
    class Model {
    public:
        std::string m_Name;
        std::vector<Vertex>  m_Vertices;
        std::vector<uint32_t>  m_Indices;
        BoundingBox boundingbox;
        glm::mat4 m_Transform = glm::mat4(1.0);
        Material m_Material;
        static void Model::LoadGLTFModel(std::string filepath, std::shared_ptr<Model>& model);
        static void Model::LoadGLTFModel(std::string filepath, std::shared_ptr<Model>& model, std::shared_ptr<Texture>& texture );
   };
}