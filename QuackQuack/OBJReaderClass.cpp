#include "OBJReaderClass.h"

#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <fstream>
#include <assert.h>
#include <string>

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
				uint32_t v;
				uint32_t vt;
				uint32_t vn;

				uint32_t v1;
				uint32_t vt1;
				uint32_t vn1;

				uint32_t v2;
				uint32_t vt2;
				uint32_t vn2;


				/*Read in the indices*/
				ifs >> v >> vt >> vn;;

				objVertexData vertexData(v, vt, vn);


				data.push_back(vertexData);
			}
		}

		/*Make sure they are not empty*/
		assert(vertexPositions.size() != 0 && vertexTextureCoordinates.size() == 0 && vertexNormals.size() != 0 && data.size() != 0);

		ifs.close(); // Close the file
	}

	return true;
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
