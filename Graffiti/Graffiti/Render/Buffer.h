#pragma once
#include "Graffiti/Scene/Vertex.h"

namespace Graffiti {

	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};
	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:    return 4;
		case ShaderDataType::Float2:   return 4 * 2;
		case ShaderDataType::Float3:   return 4 * 3;
		case ShaderDataType::Float4:   return 4 * 4;
		case ShaderDataType::Mat3:     return 4 * 3 * 3;
		case ShaderDataType::Mat4:     return 4 * 4 * 4;
		case ShaderDataType::Int:      return 4;
		case ShaderDataType::Int2:     return 4 * 2;
		case ShaderDataType::Int3:     return 4 * 3;
		case ShaderDataType::Int4:     return 4 * 4;
		case ShaderDataType::Bool:     return 1;
		}

		GF_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}
	struct BufferElement
	{
		std::string Name;
		uint32_t Offset;
		uint32_t Size;
		ShaderDataType Type;
		bool Normalized;

		BufferElement() {};
		BufferElement(ShaderDataType type, const std::string& name,bool normalized=false) :
			Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized){};

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
			case ShaderDataType::Float:   return 1;
			case ShaderDataType::Float2:  return 2;
			case ShaderDataType::Float3:  return 3;
			case ShaderDataType::Float4:  return 4;
			case ShaderDataType::Mat3:    return 3; // 3* float3
			case ShaderDataType::Mat4:    return 4; // 4* float4
			case ShaderDataType::Int:     return 1;
			case ShaderDataType::Int2:    return 2;
			case ShaderDataType::Int3:    return 3;
			case ShaderDataType::Int4:    return 4;
			case ShaderDataType::Bool:    return 1;
			}

			GF_CORE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}
	}; 
	class BufferLayout {
	public:
		BufferLayout() {}
		BufferLayout(const std::initializer_list < BufferElement>& elements) 
		:m_Elements(elements){ CalculateOffsetsAndStride(); };

		inline uint32_t GetStride() const { return m_Stride; }
		inline const std::vector<BufferElement> GetElements() {return m_Elements;}

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }
	private:
		void CalculateOffsetsAndStride()
		{
			size_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}
	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};
	class VertexBuffer {
	public:
		virtual ~VertexBuffer() {};
		virtual void Bind() const  = 0;
		virtual void Unbind() const  = 0;

		virtual void SetLayout(const BufferLayout& layout)  = 0;
		virtual const BufferLayout& GetLayout()const = 0;
		static std::shared_ptr<VertexBuffer>  Create(std::vector<Vertex>& vertices, uint32_t size);
	};

	class IndexBuffer {
	public:
		virtual ~IndexBuffer() {};
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetCount() const = 0;

		static std::shared_ptr<IndexBuffer>  Create(std::vector<uint32_t>& indices, uint32_t count);
	

	};

	class UniformBuffer {

	public:

		virtual ~UniformBuffer() {}
		virtual void SetData(const void* data, uint32_t size = 0, uint32_t offset = 0) = 0;

        virtual uint32_t  GetID() { return 0; };
		static std::shared_ptr<UniformBuffer> Create(uint32_t unitsize, uint32_t count=1, uint32_t set = 0, uint32_t binding = 0);
	
        //OpenGl的m_Set设置为Buffer的size
		uint32_t m_Set = 0;
		uint32_t m_Binding;
	};

	class StorageBuffer {

	public:

		virtual ~StorageBuffer() {}
		virtual void SetData(const void* data, uint32_t size = 0, uint32_t offset = 0) = 0;

		virtual uint32_t  GetID() { return 0; };

		static std::shared_ptr<StorageBuffer> Create(uint32_t unitsize, uint32_t count = 1, uint32_t set = 0, uint32_t binding = 0);

		//OpenGl的m_Set设置为Buffer的size
		uint32_t m_Set = 0;
		uint32_t m_Binding;
	};

}