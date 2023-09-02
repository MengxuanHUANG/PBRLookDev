#include "framebuffer.h"

namespace PBRLookDev
{
    OpenglFrameBuffer::OpenglFrameBuffer(unsigned int w, unsigned int h, unsigned int devicePixelRatio)
        : m_Width(w), m_Height(h), m_DevicePixelRatio(devicePixelRatio)
    {}

    CubeMapFrameBuffer::CubeMapFrameBuffer(unsigned int w, unsigned int h, unsigned int devicePixelRatio, bool mipmap)
        : OpenglFrameBuffer(w, h, devicePixelRatio), m_Mipmap(mipmap)
    {
        glGenFramebuffers(1, &m_FrameBuffer);
        glGenRenderbuffers(1, &m_DepthRenderBuffer);
        glGenTextures(1, &m_CubeMapId);

        Resize(w, h, devicePixelRatio);
    }

    CubeMapFrameBuffer::~CubeMapFrameBuffer()
    {
        glDeleteTextures(1, &m_CubeMapId);
        glDeleteFramebuffers(1, &m_FrameBuffer);
        glDeleteRenderbuffers(1, &m_DepthRenderBuffer);
    }

    void CubeMapFrameBuffer::Resize(unsigned int w, unsigned int h, unsigned int devicePixelRatio)
    {
        m_Width = w;
        m_Height = h;
        m_DevicePixelRatio = devicePixelRatio;

        // Initialize depth buffer
        BindRenderBuffer(m_Width, m_Height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthRenderBuffer);

        BindFrameBuffer();
        // Set m_renderedTexture as the color output of our frame buffer
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_CubeMapId, 0);

        // Sets the color output of the fragment shader to be stored in GL_COLOR_ATTACHMENT0, which we previously set to m_renderedTextures[i]
        GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, drawBuffers); // "1" is the size of drawBuffers

        glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapId);
        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, m_Width, m_Height, 0, GL_RGB, GL_FLOAT, nullptr);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        if (m_Mipmap)
        {
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        }
        else
        {
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (m_Mipmap) {
            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        }

        // reset to the default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void CubeMapFrameBuffer::BindToTextureSlot(unsigned int slot)
    {
        m_TextureSlot = slot;
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapId);
    }

    void CubeMapFrameBuffer::GenMipMaps()
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapId);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }

    GFrameBuffer::GFrameBuffer(unsigned int w, unsigned int h, unsigned int devicePixelRatio)
        : OpenglFrameBuffer(w, h, devicePixelRatio)
    {
        glGenFramebuffers(1, &m_FrameBuffer);
        glGenRenderbuffers(1, &m_DepthRenderBuffer);
        glGenTextures(4, &m_ColTextureId);

        Resize(w, h, devicePixelRatio);
    }

    GFrameBuffer::~GFrameBuffer()
    {
        glDeleteTextures(4, &m_ColTextureId);
        glDeleteFramebuffers(1, &m_FrameBuffer);
        glDeleteRenderbuffers(1, &m_DepthRenderBuffer);
    }

    void GFrameBuffer::Resize(unsigned int w, unsigned int h, unsigned int devicePixelRatio)
    {
        m_Width = w;
        m_Height = h;
        m_DevicePixelRatio = devicePixelRatio;

        BindFrameBuffer();

        glBindTexture(GL_TEXTURE_2D, m_ColTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Width * m_DevicePixelRatio, m_Height * m_DevicePixelRatio, 0, GL_RGB, GL_FLOAT, (void*)(0));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColTextureId, 0);

        glBindTexture(GL_TEXTURE_2D, m_PosTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Width * m_DevicePixelRatio, m_Height * m_DevicePixelRatio, 0, GL_RGB, GL_FLOAT, (void*)(0));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_PosTextureId, 0);

        glBindTexture(GL_TEXTURE_2D, m_NorTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Width * m_DevicePixelRatio, m_Height * m_DevicePixelRatio, 0, GL_RGB, GL_FLOAT, (void*)(0));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_NorTextureId, 0);

        glBindTexture(GL_TEXTURE_2D, m_DepthRoughnessTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Width * m_DevicePixelRatio, m_Height * m_DevicePixelRatio, 0, GL_RGB, GL_FLOAT, (void*)(0));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_DepthRoughnessTextureId, 0);

        GLuint attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
        glDrawBuffers(4, attachments);

        // depth buffer
        BindRenderBuffer(m_Width, m_Height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthRenderBuffer);

        // reset to the default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}