#include "OBJReaderClass.h"

#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <fstream>
#include <assert.h>
#include <string>
#include <sstream>

#include <windows.h>

bool OBJReaderClass::readObjFile(const std::string & path, std::vector<glm::vec3> positions, std::vector<glm::vec3> normals, std::vector<glm::vec2> textureCoords)
{
	std::ifstream ifs;
	ifs.open(path); // Open the file for reading

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
						stringFaceData.push_back(substring); // Store the individual string representations of each vertex's attributes
					}
				}
			}


		}

		data = convertIntegerDataToObjVertexData(integerFaceData);

		/*Make sure they are not empty*/
		//assert(vertexPositions.size() != 0 && vertexTextureCoordinates.size() == 0 && vertexNormals.size() != 0 && data.size() != 0);

		assert(vertexPositions.size() != 0 && vertexTextureCoordinates.size() == 0 && vertexNormals.size() != 0 && data.size() == 0);


		ifs.close(); // Close the file
	}

	return true;
}

/*
	Parses strings in the form of
	value//value and stores all integers inside a continous array
*/
std::vector<uint32_t> OBJReaderClass::parseSubstringToIntegers(const std::vector<std::string>& substrings)
{
	return std::vector<uint32_t>();
}

/*
	Gets an integer array, representing indices from a face line from and obj file.

	Made to parse values in the form of v/vt/vn (This must be further extended to support obj files with missing values!)
*/
std::vector<objVertexData> OBJReaderClass::convertIntegerDataToObjVertexData(const std::vector<uint32_t> integerData)
{
	std::vector<objVertexData> convertedData;
	return convertedData;
}

OBJReaderClass::OBJReaderClass()
{
}

OBJReaderClass::OBJReaderClass(const std::string & file) : fileName(file)
{
	readObjFile(file, vertexPositions, vertexNormals, vertexTextureCoordinates);
}


OBJReaderClass::~OBJReaderClass()
{
}
