#include "OBJReaderClass.h"

#include <iostream>
#include <fstream>
#include <assert.h>
#include <string>
#include <sstream>
#include <algorithm>

#include <iterator>
#include <windows.h>

struct Vertex;


bool OBJReaderClass::readObjFile()
{

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
				std::vector<Vertex> intermediateVertexArray = makeVertices(miniVertexArray);

				/*Triangulate the vertices and return an array of triangualted faces*/
				std::vector<Vertex> triangulatedFace = triangulate(intermediateVertexArray);

				/*Add every face produced after triangulation to a larger face, which is ordered */
				duplicateVertices.insert(duplicateVertices.end(), triangulatedFace.begin(), triangulatedFace.end());

			}
		}

		/*Create the indices array*/
		uniqueIndexData = createIndices(duplicateVertices);

		ifs.close(); // Close the file
	}

	return true;
}

std::vector<Vertex> OBJReaderClass::makeVertices(std::vector<std::string>& line)
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
		uint32_t intgerVertexPositionIndex = 0;
		uint32_t integerVertexTextureCoordinateIndex = 0; // Default values in case one of them is missing as obj files usually do
		uint32_t intgerVertexNormalIndex = 0;

		/*Default values in case values are missing*/
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec2 textureCoordinate = glm::vec2(0.0f, 0.0f);
		glm::vec3 normal = glm::vec3(0.0f, 0.0f, 0.0f);

		/*Ohterwise set to the values read from file*/
		if (vertexPositions.size() != 0)
		{
			intgerVertexPositionIndex = std::stoi(vertexPositionIndex) - 1;
			position = vertexPositions[intgerVertexPositionIndex];
		}

		if (vertexTextureCoordinates.size() != 0)
		{
			integerVertexTextureCoordinateIndex = std::stoi(vertexTextureCoordinateIndex) - 1;
			textureCoordinate = vertexTextureCoordinates[integerVertexTextureCoordinateIndex];
		}

		if (vertexNormals.size() != 0)
		{
			intgerVertexNormalIndex = std::stoi(vertexNormalIndex) - 1;
			normal = vertexNormals[intgerVertexNormalIndex];
		}

		/*Create a new vertex and add it to the whole array*/
		Vertex vertex(position, textureCoordinate, normal);
		vertices.push_back(vertex);
	}

	return vertices;
}

/*
Accepts the amount of vertex data that is present in the line, in the form of a std::vector<Vertex>
It then performs a triangulation technique, which is quite common.

Uses a random chosen vertex, and simply loops over all other vertices, building up faces
as it goes around them.
*/
std::vector<Vertex> OBJReaderClass::triangulate(const std::vector<Vertex>& face)
{
	std::vector<Vertex> triangulatedVertexData;

	/*Go through the faces*/
	for (unsigned int vertexIndex = 1; vertexIndex < face.size() - 1; vertexIndex++)
	{
		/*Create a new face*/
		Vertex randomVertex = face[0];
		Vertex firstNeighbour = face[vertexIndex];
		Vertex secondNeighbour = face[vertexIndex + 1];

		/*Add the new face to a*/
		triangulatedVertexData.push_back(randomVertex);
		triangulatedVertexData.push_back(firstNeighbour);
		triangulatedVertexData.push_back(secondNeighbour);
	}

	return triangulatedVertexData;
}

std::vector<uint32_t> OBJReaderClass::createIndices(const std::vector<Vertex>& facesAfterTriangulation)
{
	std::vector<uint32_t> indexedArray;
	std::unordered_map<Vertex, uint32_t, KeyHasher> verticesToIndexesMap;

	/*Make sure we are not receiving an empty mesh*/
	assert(facesAfterTriangulation.size() != 0);

	/*Triangulated data should be divisible by 3*/
	assert((facesAfterTriangulation.size() % 3) == 0);


	/*Loop over all vertices from the triangulated mesh that got computed*/
	for (const Vertex& vertex : facesAfterTriangulation)
	{
		const std::unordered_map<Vertex, uint32_t, KeyHasher>::iterator checkIfPresentIterator = verticesToIndexesMap.find(vertex); // Search the map if the vertex is in the map

																												/*If an element is not found, iterator returns the end of the map*/
		if (checkIfPresentIterator == verticesToIndexesMap.end())
		{
			/*Add to the unique vertices array, which will get passed to Vulkan*/
			uniqueVertexData.push_back(vertex);

			const uint32_t index = (uint32_t)(uniqueVertexData.size() - 1);

			std::pair<Vertex, uint32_t> pair(vertex, index);

			/*Add the element to the map as well*/
			verticesToIndexesMap[vertex] = index;
		}

		/*This iterator will find the position of where the element was added in order to get the index for the indices array*/
		const std::unordered_map<Vertex, uint32_t, KeyHasher>::iterator findIndexIterator = verticesToIndexesMap.find(vertex);

		indexedArray.push_back(findIndexIterator->second);
	}


	assert(uniqueVertexData.size() != 0);
	assert(indexedArray.size() != 0);

	return indexedArray;;
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
