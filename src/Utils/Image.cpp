#include "pch.h"
#include "Utils/Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Image::Image()
{
	unsigned char pixel[] = { 0, 0, 0, 0 };

	// Allocate memory for this 1x1 texture
	data = (unsigned char*)malloc(sizeof(pixel));

	memcpy(data, pixel, sizeof(pixel));
}

Image::Image(unsigned char* color):
	data(color)
{
	
}

Image::~Image()
{

}

void Image::setImageColor(vec4 color)
{
	unsigned char colorToImage[] = { color.x * 255, color.y * 255, color.z * 255, color.w * 255 };

	// Allocate memory for this 1x1 texture
	data = (unsigned char*) malloc(sizeof(colorToImage));

	memcpy(data, colorToImage, sizeof(colorToImage));
}

void Image::setImageColor(f32 value)
{
	unsigned char valueToImage[] = { value * 255, 0, 0, 1 };

	// Allocate memory for this 1x1 texture
	data = (unsigned char*)malloc(sizeof(valueToImage));

	memcpy(data, valueToImage, sizeof(valueToImage));
}

void Image::readImage(const char* path, i32& width, i32& height, i32& numChannels)
{
	data = stbi_load(path, &width, &height, &numChannels, 4);

	if (!data)
	{
		printf("Cannot load texture\n");
	}
}

void Image::freeImage()
{
	free(data);
}