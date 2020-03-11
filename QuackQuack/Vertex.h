#pragma once

#include <glm.hpp> // glm::vec3
#include <vulkan\vulkan.h>
#include <array>

#include <iostream>
#include <iomanip>
#include <functional>
#include <string>
#include <unordered_set>

struct Vertex
{
	glm::vec3 pos;
	glm::vec2 texCoord;
	glm::vec3 norm;

	Vertex();
	Vertex(const glm::vec3& position); // Delete this 
	Vertex(const glm::vec3& position, const glm::vec2& textureCoordinate, const glm::vec3& normal);

	static VkVertexInputBindingDescription getBindingDescription();

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

};

bool operator == (const Vertex& vertex1, const Vertex& vertex2);

// class for hash function 
struct KeyHasher
{
	std::size_t operator()(const Vertex& v) const
	{
		using std::size_t;
		using std::hash;
		using std::string;

		return ((hash<float>()(v.pos.x)
			^ (hash<float>()(v.texCoord.y) << 1)) >> 1)
			^ (hash<float>()(v.norm.z) << 1);
	}
};
