#pragma once
class Image
{
public:
	unsigned char* data;

public:
	Image();
	Image(unsigned char* color);
	~Image();

	void setImageColor(vec4 color);
	void setImageColor(f32 value);

	void readImage(const char* path, i32& width, i32& height, i32& numChannels);
	void freeImage();
};