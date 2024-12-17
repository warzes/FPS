#include "stdafx.h"
#include "Image.h"
//=============================================================================
Image::Image(int width, int height, int channels) : m_width(width), m_height(height), m_channels(channels)
{
	auto size = width * height * channels;
	m_memory = malloc(size);
	memset(m_memory, 0, size);
}
//=============================================================================
Image::Image(void* data, size_t size)
{
	m_memory = stbi_load_from_memory((uint8_t*)data, (int)size, &m_width, &m_height, nullptr, 4);
	m_channels = 4; // TODO: make adaptive channels
}
//=============================================================================
Image::Image(const Image& image)
{
	m_width = image.GetWidth();
	m_height = image.GetHeight();
	m_channels = image.GetChannels();
	auto size = m_width * m_height * m_channels;
	m_memory = malloc(size);
	memcpy(m_memory, image.GetMemory(), size);
}
//=============================================================================
Image::~Image()
{
	free(m_memory);
}
//=============================================================================
uint8_t* Image::GetPixel(int x, int y) const
{
	return &((uint8_t*)m_memory)[((y * m_width) + x) * m_channels];
}
//=============================================================================