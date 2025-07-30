#include"Graffiti/Render/RenderAPI.h"

namespace Graffiti {

    class OpenGLRenderAPI :public RenderAPI {
        friend class OpenGLShader;
    public:
        virtual void Init(uint32_t window = 0) override;

        virtual void SetDepthtest(bool set) override;
        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
        virtual void SetClearColor(const glm::vec4& color) override;
        virtual void Clear() override;
        virtual void DrawIndex(const std::shared_ptr<VertexArray>& vertexarray)  override;
        virtual void MeshShaderDraw(uint32_t taskcount) override;

        virtual void WireframeMode() override;
        virtual void PolygonMode() override;
        virtual void addLineLetset(std::string name, void* data, uint32_t size) override;
        virtual void addCircleLetset(std::string name, void* data, uint32_t size) override;
  
         
    private:
        void setData(std::string& ID);
        std::unordered_map <std::string, int> m_LetType;
        bool modeli = 0;
       
    };
}