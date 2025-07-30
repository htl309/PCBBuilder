#pragma once
#include "Graffiti/Render/Shader.h"
#include "Graffiti/Graphic/Vulkan/VulkanRenderAPI.h"
#include "VulkanDevice.h"
#include "VulkanPipeline.h"
namespace Graffiti {
	class VulkanShader :public Shader {
	public:
		VulkanShader(const std::string& shadername);
		VulkanShader(const std::string& shadername, std::initializer_list<std::pair<ShaderType, const std::string>> list);

		~VulkanShader();

		virtual void Bind() const override;
		virtual void PCBBind(std::string & ID) const override;

		virtual void Unbind() const override;

		virtual void Load(ShaderType shadertype, const std::string& filepath)  override;

        virtual void AddPipeline(std::shared_ptr<PipelineConfigInfo> configInfo) override;

		//Link��Vulkan�Ĺ��ߴ�������������У�
		//�����е�Shader����������ϣ���ô�Ϳ��԰󶨵���������
		//����ʹ������ߣ������Ĺ����������VulkanRenderAPI�У�����ͱ���һ������
        //ע�⣡�������������ﴴ��������ͨ�Ĺ���
		virtual void Link() override;

		virtual void UploadSceneData() override;
		virtual void SetSceneData(const SceneData& scenedata) override;
		virtual void UploadTransform() override;
		virtual void SetTransform(const PushconstData& pushconstdata) override;


		virtual void SetUniformBuffer(const std::string& name, const void* value) override;
		virtual void UploadUniformBuffer(const std::string& name, uint32_t size, uint32_t count ,uint32_t set, uint32_t binding ) override;
		
		virtual void SetStorageBuffer(const std::string& name, const void* value) override;
		virtual void UploadStorageBuffer(const std::string& name, uint32_t size, uint32_t count, uint32_t set, uint32_t binding) override;

		virtual void UploadTexture(const std::string& name, std::shared_ptr<Texture> texture, uint32_t set, uint32_t binding) override;
        virtual void SetTexture(std::shared_ptr<Texture> texture, uint32_t set, uint32_t binding, const std::string modelname) override;

		virtual void ClearPipeline() { m_PipelineConfigInfos.clear(); }
	private:
		std::vector<char> readFile(const std::string& filepath);
		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

	private:
		
		std::string m_Name;

		std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
		std::vector<VkShaderModule> m_ShaderModule;
		VulkanRenderAPI* m_RenderAPI;

        std::shared_ptr<VulkanPipelineLayout> m_PipelineLayout;
		std::vector<std::shared_ptr<PipelineConfigInfo>> m_PipelineConfigInfos;
	};
	
}


