#pragma once

class Image final
{
public:
	Image(int width, int height, int channels);
	Image(void* data, size_t size);
	Image(const Image& image);
	~Image();

	uint8_t* GetPixel(int x, int y) const;

	auto GetMemory() const { return m_memory; }
	auto GetWidth() const { return m_width; }
	auto GetHeight() const { return m_height; }
	auto GetChannels() const { return m_channels; }

private:
	void* m_memory;
	int   m_width;
	int   m_height;
	int   m_channels;
};


inline std::tuple<uint32_t, uint32_t, void*> LoadTextureFromSTBImage(const std::string& filename)
{
	int width = 0;
	int height = 0;
	void* memory = stbi_load(filename.c_str(), &width, &height, nullptr, 4);
	return { width, height, memory };
}