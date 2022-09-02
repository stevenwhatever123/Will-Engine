#pragma once

namespace WillEngine::Utils
{
	// Read Success? , filename
	std::tuple<bool, std::string> selectFile();
	
	// For reading shader
	std::vector<char> readSprivShader(const char* filename);
}