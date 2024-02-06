#pragma once

#include <stdint.h>
#include "../ew/shader.h"

namespace gg {
	namespace fb {
		struct framebuffer {
			uint32_t fbo;
			uint32_t colorBuffer;
			uint32_t depthBuffer;
			uint32_t width;
			uint32_t height;
		};

		framebuffer Create(uint32_t width, uint32_t height, int32_t colorFormat);
		void Destroy(framebuffer* buffer);
		void Bind(framebuffer* buffer);
	};
};