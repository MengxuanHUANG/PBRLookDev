#include "buffer.h"

#include "Core/core.h"

namespace PBRLookDev
{
	OpenglBuffer::OpenglBuffer(int type, int usage)
		:m_Type(type), m_Usage(usage)
	{
		ASSERT(type == GL_ARRAY_BUFFER || type == GL_ELEMENT_ARRAY_BUFFER);
		glGenBuffers(1, &m_BufferId);
	}

	OpenglBuffer::~OpenglBuffer()
	{
		glDeleteBuffers(1, &m_BufferId);
	}

	void OpenglBuffer::BufferData(const void* data, size_t size)
	{
		Bind();
		glBufferData(m_Type, size, data, GL_STATIC_DRAW);
	}
}