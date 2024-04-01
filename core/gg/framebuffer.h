#pragma once

#include <stdint.h>
#include "../ew/shader.h"

#define MAX_COLOR_BUFFERS 8

namespace gg {
	class FrameBuffer {
	public:
		FrameBuffer(uint32_t fbo, uint32_t colorBuffer, uint32_t depthBuffer, uint32_t width, uint32_t height);
		FrameBuffer();

		uint32_t fbo;
		uint32_t colorBuffer[MAX_COLOR_BUFFERS];
		uint32_t depthBuffer;
		uint32_t width;
		uint32_t height;
	};

	FrameBuffer CreateFrameBuffer(uint32_t width, uint32_t height, int32_t colorFormat);
	FrameBuffer CreateFrameBufferDepthOnly(uint32_t width, uint32_t height);
	FrameBuffer CreateGBuffer(uint32_t width, uint32_t height);
	void DestroyFrameBuffer(FrameBuffer* buffer);
	void BindFrameBuffer(FrameBuffer* buffer);
};