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
        inline void BindRenderBuffer(unsigned int w, unsigned int h) 
        {
            glBindRenderbuffer(GL_RENDERBUFFER, m_DepthRenderBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w * m_DevicePixelRatio, h * m_DevicePixelRatio);
        }

    public:
        GLuint m_FrameBuffer;
        GLuint m_DepthRenderBuffer;

        unsigned int m_Width, m_Height, m_DevicePixelRatio;
        unsigned int m_TextureSlot;
    };

    class SimpleFrameBuffer : public OpenglFrameBuffer
    {
    public:
        SimpleFrameBuffer(unsigned int w, unsigned int h, unsigned int devicePixelRatio);
        ~SimpleFrameBuffer();

        virtual void Resize(unsigned int w, unsigned int h, unsigned int devicePixelRatio) override;
        virtual void BindToTextureSlot(unsigned int slot) override;

        float* GetRenderedImage();

    public:
        unsigned int m_RenderedImage;
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

    class GFrameBuffer : public OpenglFrameBuffer
    {
    public:
        GFrameBuffer(unsigned int w, unsigned int h, unsigned int devicePixelRatio);
        ~GFrameBuffer();

        virtual void Resize(unsigned int w, unsigned int h, unsigned int devicePixelRatio) override;
        virtual void BindToTextureSlot(unsigned int slot) override {}

    public:
        unsigned int m_ColTextureId;
        unsigned int m_PosTextureId;
        unsigned int m_NorTextureId;
        unsigned int m_DepthRoughnessTextureId;
    };
}