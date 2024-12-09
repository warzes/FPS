﻿#include "stdafx.h"
#include "Image.h"

using namespace Graphics;

Image::Image(int width, int height, int channels) : mWidth(width), mHeight(height), mChannels(channels)
{
	auto size = width * height * channels;
	mMemory = malloc(size);
	memset(mMemory, 0, size);
}

Image::Image(void* data, size_t size)
{
	mMemory = stbi_load_from_memory((uint8_t*)data, (int)size, &mWidth, &mHeight, nullptr, 4);
	mChannels = 4; // TODO: make adaptive channels
}

Image::Image(const Image& image)
{
	mWidth = image.GetWidth();
	mHeight = image.GetHeight();
	mChannels = image.GetChannels();
	auto size = mWidth * mHeight * mChannels;
	mMemory = malloc(size);
	memcpy(mMemory, image.GetMemory(), size);
}

Image::~Image()
{
	free(mMemory);
}

uint8_t* Image::GetPixel(int x, int y) const
{
	return &((uint8_t*)mMemory)[((y * mWidth) + x) * mChannels];
}