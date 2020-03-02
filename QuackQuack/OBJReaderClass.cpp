#include "OBJReaderClass.h"

#include <iostream>
#include <fstream>
#include <assert.h>
#include <string>
#include <sstream>
#include <algorithm>

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

		/*Extract the integer values from the face attribute strings*/
		integerFaceData = parseSubstringToIntegers(stringFaceData);

		/*Convert the integer values to objVertexData*/
		data = convertIntegerDataToObjVertexData(integerFaceData);

		uniqueData = clearObjVertexDataDuplicates(data);

		/*Make sure none of the arrays is empty*/
		assert(vertexPositions.size() != 0);
		assert(vertexTextureCoordinates.size() == 0);
		assert(vertexNormals.size() != 0);
		assert(data.size() != 0);


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
std::vector<objVertexData> OBJReaderClass::convertIntegerDataToObjVertexData(const std::vector<uint32_t>& integerData)
{
	std::vector<objVertexData> convertedData;

	/*Loop over all integers, each 2 is a position and normal*/
	for (unsigned int values = 0; values < integerData.size(); values += 2)
	{
		
		/*Extract the two values*/
		uint32_t v = integerData[values];
		uint32_t vn = integerData[values + 1];

		/*Store the vertex attributes for a single vertex*/
		objVertexData objVertex(v, vn);

		/*Push the data back to the arrya*/
		convertedData.push_back(objVertex);

	}

	assert(convertedData.size() != 0);

	return convertedData;
}

std::vector<objVertexData> OBJReaderClass::clearObjVertexDataDuplicates(const std::vector<objVertexData>& data)
{
	/*Make sure we are not given an empty array*/
	assert(data.size() != 0);

	std::vector<objVertexData> uniqueObjVertexData;
	uniqueObjVertexData.push_back(data[0]);

	/*Loop over all data */
	for (unsigned int i = 0; i < data.size(); i++)
	{

		/*Loop over the unique data in the array*/
		for (unsigned int j = 0; j < uniqueObjVertexData.size(); j++)
		{

			bool notFound = false;


			/*If the data was already present, leave*/
			if (data[i] == uniqueObjVertexData[j])
			{
				notFound = false;
				break;
			}
			/*Else add the newly-found index*/
			else
			{
				notFound = true;
			}

			if (notFound)
			{
				OutputDebugString("Added");
				uniqueObjVertexData.push_back(data[i]);
			}
		}
		// Figure out why it is infinitely adding new values

	}

	assert(uniqueObjVertexData.size() != 0);


	OutputDebugString(std::to_string(uniqueObjVertexData.size()).c_str());


	return uniqueObjVertexData;
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
