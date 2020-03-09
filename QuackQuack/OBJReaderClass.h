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
	std::vector<Vertex> duplicateVertices;

	/*Output data*/

	std::vector<Vertex> uniqueVertexData;
	std::vector<uint32_t> uniqueIndexData;

	/*std::vector<TriangleFacePosition> trianglePositions;*/

	/*Functions*/

	/*Reads the data from the obj file and stores it in one of the member arrays*/
	bool readObjFile(); // Reads the obj file

	/*Creates Vertex data from the faces read*/
	std::vector<Vertex> makeVertices( std::vector<std::string>& line);

	/*Creates the faces of the mesh via triangulation*/
	std::vector<Vertex> triangulate(const std::vector<Vertex>& face);

	/*Create a hash map between vertices and a uint32_t ID*/
	std::unordered_map<Vertex, uint32_t, KeyHasher> createHashmapForIndices(const std::vector<Vertex>& facesAfterTriangulation);

	/*Creates the indices array that will be passed to Vulkan*/
	std::vector<uint32_t> createIndices(const std::unordered_map<Vertex, uint32_t, KeyHasher>, const std::vector<Vertex>& triangulatedFaces);

public:

	OBJReaderClass();
	OBJReaderClass(const std::string& file);

	/*Getters*/
	std::string getFileName() const { return fileName; };
	std::vector<glm::vec3> getPositions() const { return vertexPositions; };
	std::vector<glm::vec2> geTextureCoordinates() const { return vertexTextureCoordinates; };
	std::vector<glm::vec3> getNormals() const { return vertexNormals; };
	
	/*The important methods*/
	std::vector<Vertex> getVertices() const { return uniqueVertexData; };
	std::vector<uint32_t> getIndices() const { return uniqueIndexData; };

	~OBJReaderClass();
};

