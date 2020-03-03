#pragma once

#include<iostream>

struct objVertexData
{
	uint32_t v;
	uint32_t vt;
	uint32_t vn;

	objVertexData(uint32_t pos, uint32_t tex, uint32_t norm); // For the Duck, which has texture coordinates

	objVertexData(uint32_t pos, uint32_t norm); // For the cube used for testing, which only has position and normal
};

bool operator == (const objVertexData& obj1, const objVertexData& obj2);
