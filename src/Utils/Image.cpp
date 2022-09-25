#include "pch.h"
#include "Utils/Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Image::Image()
{
	unsigned char pixels[] = { 0, 0, 0, 0 };

	data = pixels;
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
	if (data)
		stbi_image_free(data);
	else
		free(data);
}