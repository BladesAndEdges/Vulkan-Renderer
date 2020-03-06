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

				/*Extract the integer values from the face attribute strings*/
				integerFaceData = parseSubstringToIntegers(miniVertexArray);

				/*Convert the data to a format we can use for computing separate vertices*/
				data = convertIntegerDataToObjVertexData(integerFaceData);

				/*Triangulate the face*/
				std::vector<TriangleFacePosition> triangulatedFace = triangulate(data);

				/*Add the data to one large array of triangulated data from the file*/
				triangles.insert(triangles.end(), triangulatedFace.begin(), triangulatedFace.end());

			}


		}

		OutputDebugString("Hello there");

		vertices = computePositionVertices();
		indices = computePositionIndices();

		//indices = computeIndices();

		//assert(indices.size() == faceVerticesIndexed.size());

		///*Make sure none of the arrays is empty*/
		//assert(vertexPositions.size() != 0);
		//assert(vertexTextureCoordinates.size() == 0);
		//assert(vertexNormals.size() != 0);
		//assert(data.size() != 0);


		ifs.close(); // Close the file
	}

	return true;
}

/*
	Parses strings in the form of
	value//value and stores all integers inside a continous array
*/
std::vector<uint32_t> OBJReaderClass::parseSubstringToIntegers(std::vector<std::string>& substrings)
{
	/*The extracted integer values from the obj file*/
	std::vector<uint32_t> integerData;

	std::string backslash = "/"; // Character we wish to split the strings at
	std::string whitespace = " ";// Character we wish to replace the baskslash with

	/*Split up the data by replacing / instances with a whitespace*/
	for (size_t substring = 0; substring < substrings.size(); substring++)
	{
		std::replace(substrings[substring].begin(), substrings[substring].end(), '/', ' '); 
	}

	std::stringstream ss; // A stringstream t
	std::string tempString;

	std::vector<std::string> stringInts;

	for (size_t string = 0; string < substrings.size(); string++)
	{
		std::stringstream temp;
		ss.swap(temp);
		ss << substrings[string];

		while (!ss.eof())
		{
			ss >> tempString;
			stringInts.push_back(tempString);
		}

		ss.str(std::string());
	}

	/*Convert to uint32_t*/
	for (unsigned int string = 0; string < stringInts.size(); string++)
	{
		/*Convert value to integer*/
		uint32_t tempInt = std::stoi(stringInts[string]);
		integerData.push_back(tempInt);
	}

	return integerData;
}

/*
	Gets an integer array, representing indices from a face line from and obj file.

	Made to parse values in the form of v/vt/vn (This must be further extended to support obj files with missing values!)
*/
std::vector<objVertexData> OBJReaderClass::convertIntegerDataToObjVertexData(const std::vector<uint32_t>& integerData) const
{
	std::vector<objVertexData> convertedData;

	/*Loop over all integers, each 2 is a position and normal*/
	for (unsigned int values = 0; values < integerData.size(); values += 3)
	{
		
		/*Extract the three attrbutes per vertex*/
		uint32_t v = integerData[values]; // position
		uint32_t vt = integerData[values + 1]; // texture coord
		uint32_t vn = integerData[values + 2]; // normal

		/*Store the vertex attributes for a single vertex*/
		objVertexData objVertex(v, vt,  vn);

		/*Push the data back to the arrya*/
		convertedData.push_back(objVertex);

	}

	assert(convertedData.size() != 0);

	return convertedData;
}

std::vector<TriangleFacePosition> OBJReaderClass::triangulate(const std::vector<objVertexData> face)
{
	/*Effectively the face we begin with*/
	std::vector<TriangleFacePosition> trianglesAfterTriangulation;

	/*Go through each vertex in the face*/
	for (unsigned int vertex = 1; vertex < face.size() - 1; vertex++) // Start at 1 as our first vertex will get reused and should not be included in the calculation as to avoid a triangle with it being twice
	{
		/*A main vertex which can be any random one from them*/
		uint32_t main = face[0].v;

		/*The two other vertices that would form a triangle with the main one*/
		uint32_t first = face[vertex].v;
		uint32_t second = face[vertex + 1].v;

		/*Create a new face from the old one*/
		TriangleFacePosition tempTriangle(main, first, second);

		/*Add the new triangles produced after triangulation to the array*/
		trianglesAfterTriangulation.push_back(tempTriangle);
	}

	/*Should return a list of triangles achieved via triangulation of the face data from the file */
	return trianglesAfterTriangulation;
}

glm::vec3 OBJReaderClass::retrieveVertexPosition(uint32_t index)
{
	return vertexPositions[index];
}

glm::vec3 OBJReaderClass::retrieveVertexNormal(uint32_t index)
{
	return vertexNormals[index];
}

glm::vec2 OBJReaderClass::retrieveVertexTextureCoordinate(uint32_t index)
{
	return vertexTextureCoordinates[index];
}

std::vector<Vertex> OBJReaderClass::convertToVertexObjects()
{
	/*Store the vertices array*/
	std::vector<Vertex> verts;

	/*Loop through the indeices*/
	for (size_t objVertex = 0; objVertex < data.size(); objVertex++)
	{
		/*Get the index of the position*/
		uint32_t positionIndex = data[objVertex].v;
		uint32_t textureIndex = data[objVertex].vt;
		uint32_t normalIndex = data[objVertex].vn;

		/*Store the postion*/
		glm::vec3 position = retrieveVertexPosition(positionIndex - 1);
		glm::vec2 textureCoord = retrieveVertexTextureCoordinate(textureIndex - 1);
		glm::vec3 normal = retrieveVertexNormal(normalIndex - 1);

		/*Create a Vertex object and add to the array*/
		Vertex vert(position, textureCoord, normal);
		verts.push_back(vert);
	}

	assert(data.size() == verts.size());

	return verts;
}

std::vector<Vertex> OBJReaderClass::computePositionVertices()
{

	std::vector < Vertex > vertexData;

	OutputDebugString("Computing Vertices");

	for (unsigned int vertexIndex = 0; vertexIndex < vertexPositions.size(); vertexIndex++)
	{
		vertexData.push_back(vertexPositions[vertexIndex]);
	}

	return vertexData;
}

std::vector<uint32_t> OBJReaderClass::computePositionIndices()
{
	std::vector<uint32_t> indices;

	/*Go through each face, and push back the array to indices*/
	for (unsigned int index = 0; index < triangles.size(); index++)
	{
		indices.push_back(triangles[index].mainVertexIndex - 1); // v0
		indices.push_back(triangles[index].firstVertexIndex - 1); // v1
		indices.push_back(triangles[index].secondVertexIndex - 1); // v2
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
