#include <stdint.h>

#define GBUFFER_COLOR_BUFFER_COUNT 3

namespace gg {
	class GBuffer {
	private:
		uint32_t width, height;
		uint32_t fbo;

		uint32_t colorBuffers[GBUFFER_COLOR_BUFFER_COUNT];

	public:
		GBuffer(uint32_t width, uint32_t height);

		void Begin();
		void End();

		uint32_t* GetColorBuffers() { return colorBuffers; };
		uint32_t GetWidth() { return width; };
		uint32_t GetHeight() { return height; };
	};
}