#pragma once
#include"Graffiti/Render/Shader.h"
#include"Graffiti/Graphic/Opengl/OpenGLRenderAPI.h"
namespace Graffiti {
    
	class OpenGLShader :public Shader {
	public:

		OpenGLShader(const std::string& shadername);
		OpenGLShader(const std::string& shadername, std::initializer_list<std::pair<ShaderType, const std::string>> list);
		
		~OpenGLShader();

		virtual void Bind() const override;
        virtual void PCBBind(std::string& ID) const override;
		virtual void Unbind() const override;

		virtual void Load(ShaderType shadertype, const std::string& filepath)  override;
		virtual void Link() override;

		virtual void UploadSceneData() override;
		virtual void SetSceneData(const SceneData& scenedata) override;
		
		virtual void UploadTransform() override;
		virtual void SetTransform(const PushconstData& pushconstdata) override;

		virtual void SetUniformBuffer(const std::string& name, const void* value) override;
		virtual void UploadUniformBuffer(const std::string& name, uint32_t size, uint32_t count, uint32_t set = 0, uint32_t binding = 0) override;

		virtual void SetStorageBuffer(const std::string& name, const void* value) override;
		virtual void UploadStorageBuffer(const std::string& name, uint32_t size, uint32_t count, uint32_t set, uint32_t binding) override;

		//����OpenGLû��set����������binding�൱�ڲ���
		virtual void UploadTexture(const std::string& name, std::shared_ptr<Texture> texture, uint32_t set, uint32_t binding) override;
		virtual void SetTexture(std::shared_ptr<Texture> texture, uint32_t set, uint32_t binding, const std::string modelname) override;

	private:
		uint32_t m_RenderID;
		std::string m_Name;
		
		std::unordered_map<GLuint,ShaderType> m_OpenGLSourceCode;

        std::shared_ptr < UniformBuffer> m_SceneDataBuffer;

        std::unordered_map<std::string, std::shared_ptr<UniformBuffer>> m_UniformBuffer;
        std::unordered_map<std::string, std::shared_ptr<StorageBuffer>> m_StorageBuffer;
        uint32_t m_Binding = 0;

		OpenGLRenderAPI* m_RenderAPI;
	};
	
}


