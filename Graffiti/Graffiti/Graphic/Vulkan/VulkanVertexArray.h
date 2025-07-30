#pragma once
#include"Graffiti/Render/VertexArray.h"
namespace Graffiti {

	class VulkanVertexArray: public VertexArray
	{
	public:
		VulkanVertexArray();
		virtual ~VulkanVertexArray();
		virtual void Bind() const override;
		virtual void Unbind() const override;


		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
		virtual void AddIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;

		virtual const std::shared_ptr<VertexBuffer>& GetVertexBuffers() const override { return m_VertexBuffers; }
		virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer()  const override { return m_IndexBuffer; }
	private:
		void bindPipeline();
		uint32_t m_RenderID;
		uint32_t m_VertexBufferIndex = 0;
		std::shared_ptr<VertexBuffer> m_VertexBuffers;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
	};
}


