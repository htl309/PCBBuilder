#pragma once
#include"VertexArray.h"

#include<glm/glm.hpp>

namespace Graffiti {

	
	class RenderAPI {
	public:
        enum class API {
            OpenGL = 0,
            Vulkan = 1
        };

        static uint32_t ContextIndex;
        RenderAPI();
        virtual ~RenderAPI() = default;
    public:
        virtual void Init(uint32_t  contextindex = 0) = 0;
		inline static API GetRenderAPI() { return s_API; }

        virtual void SetDepthtest(bool set) =0;
        virtual void SetMeshShader(bool set) {};
        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) =0;
        virtual void SetClearColor(const glm::vec4& color) = 0;
        virtual void Clear() = 0;
        virtual void DrawIndex(const std::shared_ptr<VertexArray>& vertexarray) = 0;
        virtual void MeshShaderDraw(uint32_t taskcount) = 0;
        //线框模式还是多边形模式，默认是多边形模式
        virtual void WireframeMode() = 0;
        virtual void PolygonMode() = 0;

        virtual void addLineLetset(std::string name, void* data, uint32_t size) = 0;
        virtual void addCircleLetset(std::string name, void* data, uint32_t size) = 0;




        std::vector<uint8_t> LineIndex1;
        std::vector<uint8_t> LineIndex2;
        std::vector<uint8_t> CircleIndex1;
        std::vector<uint8_t> CircleIndex2;
        std::vector<uint8_t> CircleIndex3;

        std::unordered_map<std::string, std::shared_ptr< StorageBuffer > > m_IndexBuffer;
        std::unordered_map<std::string, std::shared_ptr< StorageBuffer > > m_LetBuffer;

       // std::vector<std::string > m_lastname;
	private:
		static API s_API;
        
	};
}