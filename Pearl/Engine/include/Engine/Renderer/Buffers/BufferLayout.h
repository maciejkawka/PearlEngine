#pragma once
#include<cstdint>
#include<initializer_list>
#include<string>

#include"Core/Utils/Logger.h"

namespace PrRenderer::Buffers {

	enum class ShaderDataType{
		Unknown = 0,
		Float,
		Float2,
		Float3,
		Float4,
		Mat3,
		Mat4,
		Int,
		Int2,
		Int3,
		Int4,
		Bool
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

		return 0;
	}

	struct BufferElement
	{
		std::string name;
		ShaderDataType type;
		uint32_t size;
		size_t offset;
		bool normalized;

		BufferElement() = default;

		BufferElement(ShaderDataType p_type, const std::string& p_name, bool p_normalized = false)
			: name(p_name), type(p_type), size(ShaderDataTypeSize(p_type)), offset(0), normalized(p_normalized)
		{
		}

		uint32_t GetTypeSize() const
		{
			switch (type)
			{
			case ShaderDataType::Float:   return 1;
			case ShaderDataType::Float2:  return 2;
			case ShaderDataType::Float3:  return 3;
			case ShaderDataType::Float4:  return 4;
			case ShaderDataType::Mat3:    return 3;
			case ShaderDataType::Mat4:    return 4;
			case ShaderDataType::Int:     return 1;
			case ShaderDataType::Int2:    return 2;
			case ShaderDataType::Int3:    return 3;
			case ShaderDataType::Int4:    return 4;
			case ShaderDataType::Bool:    return 1;
			}

			PRLOG_ERROR("Renderer: ShaderDataType {0} UNKNOWN!", name);
			return 0;
		}
	};

	class BufferLayout {
	public:
		BufferLayout():
		m_stride(0)
		{}

		BufferLayout(std::initializer_list<BufferElement> p_elements)
			:m_elements(p_elements)
		{
			CalculateLayout();
		}

		inline size_t GetStride() const { return m_stride; }
		inline const std::vector<BufferElement>& GetElements() const { return m_elements; }

		inline std::vector<BufferElement>::iterator Begin() { return m_elements.begin(); }
		inline std::vector<BufferElement>::iterator End() { return m_elements.end(); }

	private:
		void CalculateLayout()
		{
			size_t offset = 0;
			m_stride = 0;

			for (auto& element : m_elements)
			{
				element.offset = offset;
				offset += element.size;
				m_stride += element.size;
			}
		}

		std::vector<BufferElement> m_elements;
		size_t m_stride;
	};
}