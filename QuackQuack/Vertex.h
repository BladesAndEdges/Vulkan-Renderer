#pragma once

#include <glm.hpp> // glm::vec3
#include <vulkan\vulkan.h>
#include <array>

struct Vertex
{
	glm::vec3 pos;
	glm::vec2 texCoord;
	glm::vec3 norm;

	Vertex();
	Vertex(const glm::vec3& position); // Delete this 
	Vertex(const glm::vec3& position, const glm::vec2& textureCoordinate, const glm::vec3& normal);

	static VkVertexInputBindingDescription getBindingDescription();

	static std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions();
};

bool operator == (const Vertex& vertex1, const Vertex& vertex2);