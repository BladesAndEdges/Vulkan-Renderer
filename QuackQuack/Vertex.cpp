#include "Vertex.h"

Vertex::Vertex()
{
}

Vertex::Vertex(const glm::vec3 & position) : pos(position) // Delete this
{
}

Vertex::Vertex(const glm::vec3 & position, const glm::vec2& textureCoordinate, const glm::vec3& normal) : pos(position), texCoord(textureCoordinate), norm(normal)
{
}

VkVertexInputBindingDescription Vertex::getBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 3> atributeDescriptions = {};

	atributeDescriptions[0].binding = 0;
	atributeDescriptions[0].location = 0;
	atributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	atributeDescriptions[0].offset = offsetof(Vertex, pos);

	atributeDescriptions[1].binding = 0;
	atributeDescriptions[1].location = 1;
	atributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	atributeDescriptions[1].offset = offsetof(Vertex, texCoord);

	atributeDescriptions[2].binding = 0;
	atributeDescriptions[2].location = 2;
	atributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	atributeDescriptions[2].offset = offsetof(Vertex, norm);

	return atributeDescriptions;
}

bool operator == (const Vertex& vertex1, const Vertex& vertex2)
{
	return ( (vertex1.pos == vertex2.pos) && (vertex1.texCoord == vertex2.texCoord) && (vertex1.norm == vertex2.norm) );
}
