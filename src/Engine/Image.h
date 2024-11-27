#pragma once

namespace Graphics
{
	class Image
	{
	public:
		Image(int width, int height, int channels);
		Image(void* data, size_t size);
		Image(const Image& image);
		~Image();

		uint8_t* GetPixel(int x, int y) const;

		auto GetMemory() const { return mMemory; }
		auto GetWidth() const { return mWidth; }
		auto GetHeight() const { return mHeight; }
		auto GetChannels() const { return mChannels; }

	private:
		void* mMemory;
		int mWidth;
		int mHeight;
		int mChannels;
	};
}