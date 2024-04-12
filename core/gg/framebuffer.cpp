#include <stdint.h>
#include <stdio.h>
#include "../ew/external/glad.h"
#include "framebuffer.h"

namespace gg {
	FrameBuffer::FrameBuffer() {
		this->colorBuffer[0] = 0;
		this->depthBuffer = 0;
		this->width = 0;
		this->height = 0;
		this->fbo = 0;
	}

	FrameBuffer::FrameBuffer(uint32_t fbo, uint32_t colorBuffer, uint32_t depthBuffer, uint32_t width, uint32_t height) {
		this->colorBuffer[0] = colorBuffer;
		this->depthBuffer = depthBuffer;
		this->height = height;
		this->width = width;
		this->fbo = fbo;
	}

	FrameBuffer CreateFrameBuffer(uint32_t width, uint32_t height, int32_t colorFormat) {
		uint32_t fbo, colorBuffer, depthBuffer;

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(1, &colorBuffer);
		glBindTexture(GL_TEXTURE_2D, colorBuffer);
		glTexStorage2D(GL_TEXTURE_2D, 1, colorFormat, width, height);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colorBuffer, 0);

		glGenTextures(1, &depthBuffer);
		glBindTexture(GL_TEXTURE_2D, depthBuffer);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, width, height);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);

		GLenum fboCompleteness = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (fboCompleteness != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("Framebuffer Incomplete: %d", fboCompleteness);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return gg::FrameBuffer(fbo, colorBuffer, depthBuffer, width, height);
	}

	FrameBuffer CreateFrameBufferDepthOnly(uint32_t width, uint32_t height)
	{
		uint32_t fbo, depthBuffer;

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(1, &depthBuffer);
		glBindTexture(GL_TEXTURE_2D, depthBuffer);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, width, height);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		float boarderColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, boarderColor);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthBuffer, 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		GLenum fboCompleteness = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (fboCompleteness != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("Framebuffer Incomplete: %d", fboCompleteness);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return gg::FrameBuffer(fbo, 0, depthBuffer, width, height);
	}

	FrameBuffer CreateGBuffer(uint32_t width, uint32_t height)
	{
		uint32_t fbo, colorBuffer[3], depthBuffer;

		glCreateFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		uint32_t formats[3] = {
			GL_RGB32F, //0 = World Position 
			GL_RGB16F, //1 = World Normal
			GL_RGB16F  //2 = Albedo
		};
		//Create 3 color textures
		for (size_t i = 0; i < 3; i++)
		{
			glGenTextures(1, &colorBuffer[i]);
			glBindTexture(GL_TEXTURE_2D, colorBuffer[i]);
			glTexStorage2D(GL_TEXTURE_2D, 1, formats[i], width, height);
			//Clamp to border so we don't wrap when sampling for post processing
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			//Attach each texture to a different slot.
			//GL_COLOR_ATTACHMENT0 + 1 = GL_COLOR_ATTACHMENT1, etc
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, colorBuffer[i], 0);
		}
		//Explicitly tell OpenGL which color attachments we will draw to
		const GLenum drawBuffers[3] = {
				GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2
		};
		glDrawBuffers(3, drawBuffers);

		glGenTextures(1, &depthBuffer);
		glBindTexture(GL_TEXTURE_2D, depthBuffer);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, width, height);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		float borderColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthBuffer, 0);


		GLenum fboCompleteness = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (fboCompleteness != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("Framebuffer Incomplete: %d", fboCompleteness);
		}

		//Clean up global state
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		FrameBuffer fb(fbo, 0, depthBuffer, width, height);
		fb.colorBuffer[0] = colorBuffer[0];
		fb.colorBuffer[1] = colorBuffer[1];
		fb.colorBuffer[2] = colorBuffer[2];
		return fb;

	}

	void DestroyFrameBuffer(FrameBuffer* buffer) {

	}

	void BindFrameBuffer(const FrameBuffer& buffer) {
		glBindFramebuffer(GL_FRAMEBUFFER, buffer.fbo);
	}
}
