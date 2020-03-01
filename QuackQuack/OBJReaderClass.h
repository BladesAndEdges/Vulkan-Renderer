#pragma once

#include <iostream>
#include <string.h>

#include <vector>
#include <glm.hpp>


struct objVertexData
{
	uint32_t v;
	uint32_t vt;
	uint32_t vn;

	objVertexData(uint32_t pos, uint32_t tex, uint32_t norm) : v(pos), vt(tex), vn(norm) {};
};

class OBJReaderClass
{

private:

	std::string fileName; // The filename of the Obj mesh
	std::vector<glm::vec3> vertexPositions; // positons of vertices
	std::vector<glm::vec3> vertexNormals; // normals per vertex
	std::vector<glm::vec2> vertexTextureCoordinates; // texture coordinate per vertex
	std::vector<objVertexData> data;

	/*Reads the data from the obj file and stores it in one of the member arrays*/
	bool readObjFile(const std::string& path, std::vector<glm::vec3> positions, std::vector<glm::vec3> normals, std::vector<glm::vec2> textureCoords);

public:

	OBJReaderClass();
	OBJReaderClass(const std::string& file);

	/*Getters*/
	std::string getFileName() const { return fileName; };
	std::vector<glm::vec3> getPositions() const { return vertexPositions; };
	std::vector<glm::vec2> geTextureCoordinates() const { return vertexTextureCoordinates; };
	std::vector<glm::vec3> getNormals() const { return vertexNormals; };
	std::vector<objVertexData> getFaceData() const { return data; };

	~OBJReaderClass();
};

