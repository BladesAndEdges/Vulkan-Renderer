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

	static std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions();

};

bool operator == (const Vertex& vertex1, const Vertex& vertex2);

///*Black magic*/
///*Just taken and adjusted frrom the official C++ docs*/
//
//
//	struct MyHash
//	{
//		std::size_t operator()(Vertex const& v) const noexcept
//		{
//			std::size_t ph1 = std::hash<float>{}(v.pos.x);
//			std::size_t ph2 = std::hash<float>{}(v.pos.y);
//			std::size_t ph3 = std::hash<float>{}(v.pos.z);
//
//			std::size_t th1 = std::hash<float>{}(v.pos.x);
//			std::size_t th2 = std::hash<float>{}(v.pos.y);
//
//			std::size_t nh1 = std::hash<float>{}(v.pos.x);
//			std::size_t nh2 = std::hash<float>{}(v.pos.y);
//			std::size_t nh3 = std::hash<float>{}(v.pos.z);
//
//			return ph1 * 3 + nh1 + pow(ph2, nh3) * nh2 - th1 * pow(th2, ph3);
//		}
//	};
//
//
//	namespace std
//	{
//		template<> 
//		struct hash<Vertex>
//		{
//			std::size_t operator()(Vertex const& v) const noexcept
//			{
//				std::size_t h1 = std::hash<float>{}(v.pos.x);
//				std::size_t h2 = std::hash<float>{}(v.texCoord.x);
//				return h1 ^ (h2 << 1); // or use boost::hash_combine (see Discussion)
//			}
//		};
//	}

// class for hash function 
struct MyHashFunction {
public:
	// id is returned as hash function 
	size_t operator()(const Vertex& v) const
	{
		return v.pos.x;
	}
};
