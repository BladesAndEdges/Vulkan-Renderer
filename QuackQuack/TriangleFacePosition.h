#pragma once

#include <iostream>


struct TriangleFacePosition
{
	uint32_t mainVertexIndex;
	uint32_t firstVertexIndex;
	uint32_t secondVertexIndex;

	TriangleFacePosition(uint32_t main, uint32_t first, uint32_t second);
};