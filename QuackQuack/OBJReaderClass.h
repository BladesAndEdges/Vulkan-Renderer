#pragma once

#include <iostream>
#include <string.h>

#include <vector>
#include <glm.hpp>

#include "objVertexData.h"

#include "Vertex.h"

#include "TriangleFacePosition.h"
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

	/*Output data*/
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	/*std::vector<TriangleFacePosition> trianglePositions;*/

	/*Functions*/

	/*Reads the data from the obj file and stores it in one of the member arrays*/
	bool readObjFile(); // Reads the obj file

	std::vector<Vertex> figureOutHowTheFuckVerticesAraMade( std::vector<std::string>& line);
	std::vector<uint32_t> parseSubstringToIntegers(std::vector<std::string>& substrings);
	std::vector<objVertexData> convertIntegerDataToObjVertexData(const std::vector<uint32_t>& integerData) const;


	std::vector<Vertex> triangulate(const std::vector<objVertexData> face);

	std::vector<Vertex> computePositionVertices();
	std::vector<uint32_t> computePositionIndices(const std::vector<TriangleFacePosition>& trianglePositions);

	/*The objVertex data is in the form of indices, which match positions in the vertex postions, texture corodinates and normals arrays*/
	/*to avoid confusion, and risking to calculate correctly he indices, these functions were created to retrieve values based on an index for us*/
	//glm::vec3 retrieveVertexPosition(uint32_t index);
	//glm::vec3 retrieveVertexNormal(uint32_t index);
	//glm::vec2 retrieveVertexTextureCoordinate(uint32_t index);

public:

	OBJReaderClass();
	OBJReaderClass(const std::string& file);

	/*Getters*/
	std::string getFileName() const { return fileName; };
	std::vector<glm::vec3> getPositions() const { return vertexPositions; };
	std::vector<glm::vec2> geTextureCoordinates() const { return vertexTextureCoordinates; };
	std::vector<glm::vec3> getNormals() const { return vertexNormals; };
	
	/*The important methods*/
	std::vector<Vertex> getVertices() const { return vertices; };
	std::vector<uint32_t> getIndices() const { return indices; };

	~OBJReaderClass();
};

