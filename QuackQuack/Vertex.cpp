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

std::array<VkVertexInputAttributeDescription, 1> Vertex::getAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 1> atributeDescriptions = {};

	atributeDescriptions[0].binding = 0;
	atributeDescriptions[0].location = 0;
	atributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	atributeDescriptions[0].offset = offsetof(Vertex, pos);

	return atributeDescriptions;
}

bool operator == (const Vertex& vertex1, const Vertex& vertex2)
{
	return ( (vertex1.pos == vertex2.pos) && (vertex1.texCoord == vertex2.texCoord) && (vertex1.norm == vertex2.norm) );
}
