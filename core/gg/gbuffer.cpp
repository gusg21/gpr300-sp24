#include "gbuffer.h"

#include "../ew/external/glad.h"

gg::GBuffer::GBuffer(uint32_t width, uint32_t height)
{
	this->width = width;
	this->height = height;

	glCreateFramebuffers(1, &this->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);

	int formats[GBUFFER_COLOR_BUFFER_COUNT] = {
		GL_RGB32F, //0 = World Position 
		GL_RGB16F, //1 = World Normal
		GL_RGB16F  //2 = Albedo
	};

	//Create 3 color textures
	for (size_t i = 0; i < GBUFFER_COLOR_BUFFER_COUNT; i++)
	{
		glGenTextures(1, &this->colorBuffers[i]);
		glBindTexture(GL_TEXTURE_2D, this->colorBuffers[i]);
		glTexStorage2D(GL_TEXTURE_2D, 1, formats[i], this->width, this->height);
		//Clamp to border so we don't wrap when sampling for post processing
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//Attach each texture to a different slot.
		//GL_COLOR_ATTACHMENT0 + 1 = GL_COLOR_ATTACHMENT1, etc
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, this->colorBuffers[i], 0);
	}

	//Explicitly tell OpenGL which color attachments we will draw to
	const GLenum drawBuffers[GBUFFER_COLOR_BUFFER_COUNT] = {
		GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2
	};
	glDrawBuffers(GBUFFER_COLOR_BUFFER_COUNT, drawBuffers);
	//TODO: Add texture2D depth buffer
	//TODO: Check for completeness

	//Clean up global state
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void gg::GBuffer::Begin() {
	glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
	glViewport(0, 0, this->width, this->height);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gg::GBuffer::End()
{
	glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
}
