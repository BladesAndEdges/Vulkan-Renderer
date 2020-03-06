#pragma once

#include <iostream>

struct TriangleFaceNormal
{
	uint32_t mainNormalIndex;
	uint32_t firstNormalIndex;
	uint32_t secondNormalIndex;

	TriangleFaceNormal(uint32_t main, uint32_t first, uint32_t second);
};