#pragma once
#include"Graffiti/Render/RenderAPI.h"
#include"Graffiti/base/core.h"
#include"VulkanContext.h"
#include"VulkanPipeline.h"
namespace Graffiti {

    class VulkanRenderAPI :public RenderAPI {
        friend class VulkanShader;
        friend class VulkanVertexBuffer;
        friend class VulkanIndexBuffer;
    public:
   
        virtual void Init(uint32_t contextindex = 0) override;

        virtual void SetDepthtest(bool set) override;
        virtual void SetMeshShader(bool set) override;


        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
        virtual void SetClearColor(const glm::vec4& color) override;
        virtual void Clear() override;
        virtual void DrawIndex(const std::shared_ptr<VertexArray>& vertexarray)  override;
        virtual void MeshShaderDraw(uint32_t taskcount) override;
        virtual void WireframeMode() override;
        virtual void PolygonMode() override;


        virtual void addLineLetset(std::string name, void* data, uint32_t size)  override {
           
            std::shared_ptr<StorageBuffer> buffer = StorageBuffer::Create(40, size, 1, 0);
            buffer->SetData(data);
        
            m_LetBuffer.insert({ name ,buffer });
            
       
              
            std::shared_ptr<VulkanStorageBuffer> vkbuffer = std::dynamic_pointer_cast<VulkanStorageBuffer>(buffer);
            std::shared_ptr<VulkanStorageBuffer> buffer1 = std::dynamic_pointer_cast<VulkanStorageBuffer>(m_IndexBuffer["LineIndex1"]);
            std::shared_ptr<VulkanStorageBuffer> buffer2 = std::dynamic_pointer_cast<VulkanStorageBuffer>(m_IndexBuffer["LineIndex2"]);

           auto setwriter= std::make_shared<VulkanDescriptorWriter>(
                *m_PipelineMap["Hybrid_Line"][0].GetDescriptorSetLayout(1), 
               *m_PipelineMap["Hybrid_Line"][0].GetDescriptorPool(1)); 

           setwriter->writeBuffer(
               vkbuffer->m_Binding,
                    &vkbuffer->GetDescriptorInfo());  
           setwriter->writeBuffer(
               buffer1->m_Binding,
                &buffer1->GetDescriptorInfo()); 
           setwriter->writeBuffer(
               buffer2->m_Binding,
                &buffer2->GetDescriptorInfo()); 

            VkDescriptorSet sets;
            setwriter->build(sets);   
            m_new_DescriptorSet.insert({ name,sets });    
        }
        virtual void addCircleLetset(std::string name, void* data, uint32_t size)  override {

           
            std::shared_ptr<StorageBuffer> buffer = StorageBuffer::Create(24, size, 1, 0); 
            buffer->SetData(data); 
          
            m_LetBuffer.insert({ name ,buffer });
            
        
            std::shared_ptr<VulkanStorageBuffer> vkbuffer = std::dynamic_pointer_cast<VulkanStorageBuffer>(buffer);
            std::shared_ptr<VulkanStorageBuffer> buffer1 = std::dynamic_pointer_cast<VulkanStorageBuffer>(m_IndexBuffer["CircleIndex1"]);
            std::shared_ptr<VulkanStorageBuffer> buffer2 = std::dynamic_pointer_cast<VulkanStorageBuffer>(m_IndexBuffer["CircleIndex2"]);
            std::shared_ptr<VulkanStorageBuffer> buffer3 = std::dynamic_pointer_cast<VulkanStorageBuffer>(m_IndexBuffer["CircleIndex3"]);
        
            auto setwriter = std::make_shared<VulkanDescriptorWriter>(
                *m_PipelineMap["Hybrid_Circle"][0].GetDescriptorSetLayout(1),
                *m_PipelineMap["Hybrid_Circle"][0].GetDescriptorPool(1));

   
            setwriter->writeBuffer(
                vkbuffer->m_Binding,
                &vkbuffer->GetDescriptorInfo());
            setwriter->writeBuffer(
                buffer1->m_Binding,
                &buffer1->GetDescriptorInfo());
            setwriter->writeBuffer(
                buffer2->m_Binding,
                &buffer2->GetDescriptorInfo());
            setwriter->writeBuffer(
                buffer3->m_Binding,
                &buffer3->GetDescriptorInfo());
           
            VkDescriptorSet sets;
            setwriter->build(sets);
            m_new_DescriptorSet.insert({ name,sets });
        }

  
    private:
        void BindPipeline(const std::string& PipelineName);
        void PCBBindPipeline(const std::string& PipelineName, const std::string ID);
        void SetTexture(std::shared_ptr<Texture> texture, uint32_t set, uint32_t binding, const std::string& PipelineName, const std::string& ModelName);


    private:
        std::shared_ptr<VulkanContext> m_VulkanContext;

        //存放管线的地方
        std::unordered_map<std::string, std::vector<VulkanPipeline>> m_PipelineMap;

        std::unordered_map<std::string, VkDescriptorSet> m_new_DescriptorSet;

   

        PipelineState m_State = PipelineState::Common;


        //MeshShader的函数句柄
        PFN_vkCmdDrawMeshTasksNV vkCmdDrawMeshTasksNV;
    };
}