#include "OBJReaderClass.h"

#include <iostream>
#include <fstream>
#include <assert.h>
#include <string>
#include <sstream>
#include <algorithm>

#include <windows.h>

struct Vertex;


bool OBJReaderClass::readObjFile()
{
	std::vector<Vertex> allVertexData;

	std::ifstream ifs;
	ifs.open(fileName); // Open the file for reading

	/*If a problem occured with the file being opened for reading. Most likely misspelled name*/
	if (!ifs.is_open())
	{
		OutputDebugString("Could not find the file specified by 'path' for reading, please make sure the name was spelled correctly!");
		return false;
	}

	if (ifs.is_open()) // If file has been successfuly opened for reading
	{
		std::string string;

		/*Extract the file contents string by string*/
		while (ifs >> string)
		{
			//OutputDebugString(string.c_str());


			/*If EIF has  been met, do not store extra values*/
			if (ifs.eof())
			{
				break;
			}

			/*If it is a vertex positon*/
			if (string == "v")
			{
				/*Temporary values*/
				float x;
				float y;
				float z;

				/*Store the next 3 values as cartesian coordinates*/
				ifs >> x >> y >> z;
				glm::vec3 vertex(x, y, z);
				vertexPositions.push_back(vertex);
			}

			/*If it is a texture coordinate*/
			if (string == "vt")
			{
				float u;
				float v;

				/*Store the uv coordinates and add then to the texture array*/
				ifs >> u >> v;
				glm::vec2 texCoord(u, v);
				vertexTextureCoordinates.push_back(texCoord);
			}

			/*If it is a vertex normal*/
			if (string == "vn")
			{
				float x;
				float y;
				float z;

				/*Store the normals*/
				ifs >> x >> y >> z;
				glm::vec3 normal(x, y, z);
				vertexNormals.push_back(normal);
			}

			/*Read in the faces*/
			if (string == "f")
			{

				std::vector<std::string> miniVertexArray;
				std::string line;

				/*We now have the entire line after the line has been identified as describing a face*/
				getline(ifs, line);

				std::stringstream ss(line); // Create a stringstream from the string

				while (ss.good())
				{
					/*Get the indivdual whitespace separated values in the line*/
					std::string substring;

					std::getline(ss, substring, ' '); // Split the string into substrings whenever a whitespace is met

					if (substring != "") // There are numerous empty strings in between, we do not wish to store them
					{
						miniVertexArray.push_back(substring);
					}
				}

				/*Create the vertices*/
				std::vector<Vertex> intermediateVertexArray = figureOutHowTheFuckVerticesAraMade(miniVertexArray);

				///*Triangulate the face, and return the vertex data which makes up a face*/
				//std::vector<Vertex> intermediateVertexArray = triangulate(data);

				///*Add every face produced after triangulation to a larger face, which is ordered */
				//vertices.insert(allVertexData.end(), intermediateVertexArray.begin(), intermediateVertexArray.end());

			}


		}

		//indices = computePositionIndices(allPositions);

		/*Make sure none of the arrays is empty*/
		assert(vertexPositions.size() != 0);
		assert(vertexTextureCoordinates.size() != 0);
		assert(vertexNormals.size() != 0);
		//assert(data.size() != 0);

		ifs.close(); // Close the file
	}

	return true;
}

std::vector<Vertex> OBJReaderClass::figureOutHowTheFuckVerticesAraMade(std::vector<std::string>& line)
{
	std::vector<Vertex> vertices;

	const std::string backslash = "/"; // Character we wish to split the strings at
	const std::string whitespace = " ";// Character we wish to replace the baskslash with

	/*Split up the data by replacing / instances with a whitespace*/
	for (size_t index = 0; index < line.size(); index++)
	{
		/*Search for the two backslashes*/
		const size_t firstBackSlashIndex = line[index].find(backslash); // Find first backlsash
		const size_t secondBackSlashIndex = line[index].find(backslash, firstBackSlashIndex + 1); // The second back slash index to be found after the first one's position

		/*Extract the vertex attributes*/
		const std::string vertexPositionIndex = line[index].substr(0, firstBackSlashIndex); // position index
		const std::string vertexTextureCoordinateIndex = line[index].substr(firstBackSlashIndex + 1, secondBackSlashIndex - firstBackSlashIndex - 1); // texture coordinate index
		const std::string vertexNormalIndex = line[index].substr(secondBackSlashIndex + 1); // normal index

		/*Convert the values to integers*/
		const uint32_t intgerVertexPositionIndex = std::stoi(vertexPositionIndex);
		const uint32_t integerVertexTextureCoordinateIndex = std::stoi(vertexTextureCoordinateIndex);
		const uint32_t intgerVertexNormalIndex = std::stoi(vertexNormalIndex);

		/*The values*/
		const glm::vec3 position = vertexPositions[intgerVertexPositionIndex - 1];
		const glm::vec2 textureCoordinate = vertexTextureCoordinates[integerVertexTextureCoordinateIndex - 1];
		const glm::vec3 normal = vertexNormals[intgerVertexNormalIndex - 1];


		/*Create a new vertex and add it to the whole array*/
		Vertex vertex(position, textureCoordinate, normal);
		vertices.push_back(vertex);
	}

	return vertices;
}

std::vector<Vertex> OBJReaderClass::triangulate(const std::vector<objVertexData> face)
{

	/*FIIIIIX THIS*/

	std::vector<Vertex> triangulatedVertexData;

	/*Go through the faces*/
	for (unsigned int vertexIndex = 1; vertexIndex < face.size() - 1; vertexIndex++)
	{
		Vertex randomVertex(vertexPositions[face[0].v], vertexTextureCoordinates[face[0].vt], vertexNormals[0]);
		Vertex firstNeighbourVertex(vertexPositions[vertexIndex], vertexTextureCoordinates[vertexIndex], vertexNormals[vertexIndex]);
		Vertex secondNeighbourVertex(vertexPositions[vertexIndex + 1], vertexTextureCoordinates[vertexIndex + 1], vertexNormals[vertexIndex + 1]);

		triangulatedVertexData.push_back(randomVertex);
		triangulatedVertexData.push_back(firstNeighbourVertex);
		triangulatedVertexData.push_back(secondNeighbourVertex);
	}

	return triangulatedVertexData;
}

//glm::vec3 OBJReaderClass::retrieveVertexPosition(uint32_t index)
//{
//	return vertexPositions[index];
//}
//
//glm::vec3 OBJReaderClass::retrieveVertexNormal(uint32_t index)
//{
//	return vertexNormals[index];
//}
//
//glm::vec2 OBJReaderClass::retrieveVertexTextureCoordinate(uint32_t index)
//{
//	return vertexTextureCoordinates[index];
//}

std::vector<Vertex> OBJReaderClass::computePositionVertices()
{

	std::vector < Vertex > vertexData;

	OutputDebugString("Computing Vertices");

	for (unsigned int vertexIndex = 0; vertexIndex < vertexPositions.size(); vertexIndex++)
	{
		/*Change this to accept all data, currently using only the position constructor*/
		vertexData.push_back(vertexPositions[vertexIndex]);
	}

	return vertexData;
}

std::vector<uint32_t> OBJReaderClass::computePositionIndices(const std::vector<TriangleFacePosition>& trianglePositions)
{
	std::vector<uint32_t> indices;

	/*Go through each face, and push back the array to indices*/
	for (unsigned int index = 0; index < trianglePositions.size(); index++)
	{
		indices.push_back(trianglePositions[index].randomVertexIndex - 1); // v0
		indices.push_back(trianglePositions[index].firstVertexIndex - 1); // v1
		indices.push_back(trianglePositions[index].secondVertexIndex - 1); // v2
	}

	return indices;
}

OBJReaderClass::OBJReaderClass()
{
}

OBJReaderClass::OBJReaderClass(const std::string & file) : fileName(file)
{
	readObjFile();
}


OBJReaderClass::~OBJReaderClass()
{
}
