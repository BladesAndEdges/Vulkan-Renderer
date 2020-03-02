#pragma once

#include <iostream>
#include <string.h>

#include <vector>
#include <glm.hpp>

#include "objVertexData.h"

class OBJReaderClass
{

private:

	std::string fileName; // The filename of the Obj mesh
	std::vector<glm::vec3> vertexPositions; // positons of vertices
	std::vector<glm::vec3> vertexNormals; // normals per vertex
	std::vector<glm::vec2> vertexTextureCoordinates; // texture coordinate per vertex

	std::vector<objVertexData> data; // The face data in the form of objVertexData
	std::vector<objVertexData> uniqueData;

	std::vector<std::string> stringFaceData;
	std::vector<uint32_t> integerFaceData;

	/*Reads the data from the obj file and stores it in one of the member arrays*/
	bool readObjFile(const std::string& path, std::vector<glm::vec3> positions, std::vector<glm::vec3> normals, std::vector<glm::vec2> textureCoords);

	std::vector<uint32_t> parseSubstringToIntegers(std::vector<std::string>& substrings); 
	std::vector<objVertexData> convertIntegerDataToObjVertexData(const std::vector<uint32_t>& integerData);
	std::vector<objVertexData> clearObjVertexDataDuplicates(const std::vector<objVertexData>& data);

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

