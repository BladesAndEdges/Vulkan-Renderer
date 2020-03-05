#pragma once

#include <iostream>
#include <string.h>

#include <vector>
#include <glm.hpp>

#include "objVertexData.h"

#include "Vertex.h"

#include <unordered_map>

/*
	Consider that you may actually NOT need any paramaters as all teh data is available inside the class
*/
class OBJReaderClass
{

private:

	/*Raw data extracted from file*/

	std::string fileName; // The filename of the Obj mesh
	std::vector<glm::vec3> vertexPositions; // positons of vertices
	std::vector<glm::vec3> vertexNormals; // normals per vertex
	std::vector<glm::vec2> vertexTextureCoordinates; // texture coordinate per vertex

	/*Intermediate stages, which we only care about in the objReaderClass*/

	std::vector<std::string> stringFaceData; // USed to accept the input as strings first, and pass the individual vertex attributes to get extracted later on
	std::vector<uint32_t> integerFaceData; // All of the integers inside the face section of the obj file, in one continuous array
	std::vector<objVertexData> data; // The face data in the form of objVertexData (v/vt/vn)
	std::vector<objVertexData> triangles;


	std::vector<Vertex> faceVerticesIndexed; // The face data from the OBJ where each vertex is described with 3 uint32_t, as an index to a position, texture coord and normal 

	/*Output data*/
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	/*Functions*/

	/*Reads the data from the obj file and stores it in one of the member arrays*/
	bool readObjFile(); // Reads the obj file

	std::vector<uint32_t> parseSubstringToIntegers(std::vector<std::string>& substrings);  
	std::vector<objVertexData> convertIntegerDataToObjVertexData(const std::vector<uint32_t>& integerData) const;
	std::vector<objVertexData> clearObjVertexDataDuplicates(const std::vector<objVertexData>& data) const;

	std::vector<objVertexData> createTriangles();

	std::vector<Vertex> convertToVertexObjects();


	std::vector<Vertex> computeVertices();
	std::vector<uint32_t> computeIndices();

	/*The objVertex data is in the form of indices, which match positions in the vertex postions, texture corodinates and normals arrays*/
	/*to avoid confusion, and risking to calculate correctly he indices, these functions were created to retrieve values based on an index for us*/
	glm::vec3 retrieveVertexPosition(uint32_t index);
	glm::vec3 retrieveVertexNormal(uint32_t index);
	glm::vec2 retrieveVertexTextureCoordinate(uint32_t index);




	//void createMap();
	//std::unordered_map<Vertex, uint32_t> map;

public:

	OBJReaderClass();
	OBJReaderClass(const std::string& file);

	/*Getters*/
	std::string getFileName() const { return fileName; };
	std::vector<glm::vec3> getPositions() const { return vertexPositions; };
	std::vector<glm::vec2> geTextureCoordinates() const { return vertexTextureCoordinates; };
	std::vector<glm::vec3> getNormals() const { return vertexNormals; };
	std::vector<objVertexData> getFaceData() const { return data; };
	
	/*The important methods*/
	std::vector<Vertex> getVertices() const { return vertices; };
	std::vector<uint32_t> getIndices() const { return indices; };

	~OBJReaderClass();
};

