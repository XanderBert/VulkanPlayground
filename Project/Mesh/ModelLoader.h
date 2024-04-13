#pragma once
#include <string>
#include <vector>

struct Vertex;

namespace ObjLoader
{
	void LoadObj(const std::string& filePath, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
}
