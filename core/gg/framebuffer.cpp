#include <stdint.h>
#include <stdio.h>
#include "../ew/external/glad.h"
#include "framebuffer.h"

namespace gg {
	namespace fb {
		framebuffer Create(uint32_t width, uint32_t height, int32_t colorFormat) {
			framebuffer buffer = {0};

			//Create Framebuffer Object
			glGenFramebuffers(1, &buffer.fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, buffer.fbo);
			//Create 8 bit RGBA color buffer
			glGenTextures(1, &buffer.colorBuffer);
			glBindTexture(GL_TEXTURE_2D, buffer.colorBuffer);
			glTexStorage2D(GL_TEXTURE_2D, 1, colorFormat, width, height);
			//Attach color buffer to framebuffer
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, buffer.colorBuffer, 0);

			buffer.width = width;
			buffer.height = height;

			glGenTextures(1, &buffer.depthBuffer);
			glBindTexture(GL_TEXTURE_2D, buffer.depthBuffer);
			//Create 16 bit depth buffer - must be same width/height of color buffer
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, width, height);
			//Attach to framebuffer (assuming FBO is bound)
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, buffer.depthBuffer, 0);
		
			/*glGenRenderbuffers(1, &buffer->rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, buffer->rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer->rbo);*/
		
			GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
				printf("Framebuffer incomplete: %d", fboStatus);
			}

			return buffer;
		}

		void Destroy(framebuffer* buffer) {

		}

		void Bind(framebuffer* buffer) {
			glBindFramebuffer(GL_FRAMEBUFFER, buffer->fbo);
		}
	}
}