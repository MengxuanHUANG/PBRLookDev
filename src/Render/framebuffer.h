#pragma once

#include "GL/glew.h"

namespace PBRLookDev
{
    class OpenglFrameBuffer 
    {
    public:
        OpenglFrameBuffer(unsigned int w, unsigned int h, unsigned int devicePixelRatio);
        virtual ~OpenglFrameBuffer() = default;
        virtual void Resize(unsigned int w, unsigned int h, unsigned int devicePixelRatio) = 0;
        virtual void BindToTextureSlot(unsigned int slot) = 0;

        inline void BindFrameBuffer() { glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer); }
        inline void BindRenderBuffer() { glBindRenderbuffer(GL_RENDERBUFFER, m_DepthRenderBuffer); }

    public:
        GLuint m_FrameBuffer;
        GLuint m_DepthRenderBuffer;

        unsigned int m_Width, m_Height, m_DevicePixelRatio;
        unsigned int m_TextureSlot;
    };

    class CubeMapFrameBuffer : public OpenglFrameBuffer 
    {
    public:
        CubeMapFrameBuffer(unsigned int w, unsigned int h, unsigned int devicePixelRatio, bool mipmap);
        ~CubeMapFrameBuffer();

        virtual void Resize(unsigned int w, unsigned int h, unsigned int devicePixelRatio) override;
        virtual void BindToTextureSlot(unsigned int slot) override;

        void GenMipMaps();

    public:
        unsigned int m_CubeMapId;
        bool m_Mipmap;
    };
}