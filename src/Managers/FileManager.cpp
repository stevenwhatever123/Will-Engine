#include "pch.h"
#include "Managers/FileManager.h"

std::tuple<bool, std::string> WillEngine::Utils::selectFile()
{
	OPENFILENAME ofn;

	wchar_t szFile[256];

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	//ofn.lpstrFilter = L"BVH Motion Data (*.bvh)\0*.bvh\0All (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = L"Select File";
	//ofn.lpstrDefExt = L"bvh";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	bool readSuccess = GetOpenFileName(&ofn);

	if (readSuccess)
	{
		char charPath[256];
		//std::wcstombs(charPath, szFile, 256);
		std::wcstombs(charPath, szFile, 256);
		return { true, std::string(charPath) };
	}
	else
	{
		return {false, ""};
	}
}

std::vector<char> WillEngine::Utils::readSprivShader(const char* filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		throw std::runtime_error("Can't read shader file");

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}