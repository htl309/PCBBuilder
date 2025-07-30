#include"OpenGLRenderAPI.h"
#include<glad/glad.h>
namespace Graffiti {

   
    void OpenGLRenderAPI::Init(uint32_t contextindex)
    {


        RenderAPI::ContextIndex = contextindex;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        SetClearColor({ 0.12, 0.12, 0.12, 1.0 });
    }
   void OpenGLRenderAPI::SetDepthtest(bool set) {
       if (set) {
           glEnable(GL_DEPTH_TEST);
       }
       else
       {
           glDisable(GL_DEPTH_TEST);
       }
    }
    void OpenGLRenderAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
       glViewport(x, y, width, height);
    }
    void OpenGLRenderAPI::SetClearColor(const glm::vec4& color)
    {
        glDisable(GL_CULL_FACE);
        SetDepthtest(true);
        glDepthFunc(GL_LESS);

        glClearColor(color.r, color.g, color.b, color.w); 
    }
    void OpenGLRenderAPI::Clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    }
    void OpenGLRenderAPI::DrawIndex(const std::shared_ptr<VertexArray>& vertexarray)
    {
        glDrawElements(GL_TRIANGLES, vertexarray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
    }

    void OpenGLRenderAPI::MeshShaderDraw(uint32_t taskcount)
    {
        glDrawMeshTasksNV(0, taskcount); 
    }

    void OpenGLRenderAPI::WireframeMode()
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    void OpenGLRenderAPI::PolygonMode()
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    void OpenGLRenderAPI::addLineLetset(std::string name, void* data, uint32_t size)
    {
     
        //40是LineLet的大小
        auto buffer = StorageBuffer::Create(size*40, 1, 0, 1);
        buffer->SetData(data, buffer->m_Set);

            m_LetBuffer.insert({ name ,buffer });
        
    
        //等于0说明是直线段
        m_LetType.insert({name,0});
    }

    void OpenGLRenderAPI::addCircleLetset(std::string name, void* data, uint32_t size)
    {
     
        //24是circlelet的大小
        auto buffer = StorageBuffer::Create(size*24, 1, 0, 1);
        buffer->SetData(data, buffer->m_Set);
   
            m_LetBuffer.insert({ name ,buffer });
        
     
        m_LetType.insert({name,1});
    }

 
    

    void OpenGLRenderAPI::setData(std::string& ID)
    {

       
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_LetBuffer[ID]->GetID());
        
      
        //等于0说明是直线段
        if (m_LetType[ID] == 0) {
            
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_IndexBuffer["LineIndex1"]->GetID());
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_IndexBuffer["LineIndex2"]->GetID());
        }
        //等于1说明是圆
        else if (m_LetType[ID] == 1) {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_IndexBuffer["CircleIndex1"]->GetID());
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_IndexBuffer["CircleIndex2"]->GetID());
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_IndexBuffer["CircleIndex3"]->GetID());
        }
    }


}