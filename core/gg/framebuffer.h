#pragma once

#include <stdint.h>
#include "../ew/shader.h"

namespace gg {
		struct FrameBuffer {
			uint32_t fbo;
			uint32_t colorBuffer;
			uint32_t depthBuffer;
			uint32_t width;
			uint32_t height;
		};

		FrameBuffer CreateFrameBuffer(uint32_t width, uint32_t height, int32_t colorFormat);
		FrameBuffer CreateFrameBufferDepthOnly(uint32_t width, uint32_t height);
		void DestroyFrameBuffer(FrameBuffer* buffer);
		void BindFrameBuffer(FrameBuffer* buffer);
	};