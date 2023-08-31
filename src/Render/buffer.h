#pragma once

#include "GL/glew.h"

namespace PBRLookDev
{
	class OpenglBuffer
	{
	public:
		OpenglBuffer(int type, int usage = GL_STATIC_DRAW);
		~OpenglBuffer();

		void BufferData(const void* data, size_t size);

		inline void Bind() { glBindBuffer(m_Type, m_BufferId); }
		inline void UnBind() { { glBindBuffer(m_Type, 0); } }

	public:
		int m_Type;
		int m_Usage;
		GLuint m_BufferId;
	};
}