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

	std::vector<objVertexData> data; // The face data in the form of objVertexData (v/vt/vn)
	std::vector<objVertexData> uniqueData; // The data, but with erasing duplciate values

	std::vector<std::string> stringFaceData; // USed to accept the input as strings first, and pass the individual vertex attributes to get extracted later on
	std::vector<uint32_t> integerFaceData; // All of the integers inside the face section of the obj file, in one continuous array

	/*Reads the data from the obj file and stores it in one of the member arrays*/
	bool readObjFile(); // Reads the obj file

	std::vector<uint32_t> parseSubstringToIntegers(std::vector<std::string>& substrings);  
	std::vector<objVertexData> convertIntegerDataToObjVertexData(const std::vector<uint32_t>& integerData) const;
	std::vector<objVertexData> clearObjVertexDataDuplicates(const std::vector<objVertexData>& data) const;

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

