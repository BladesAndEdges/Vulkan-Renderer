#include "objVertexData.h"

objVertexData::objVertexData(uint32_t pos, uint32_t tex, uint32_t norm) : v(pos), vt(tex), vn(norm) 
{

}

objVertexData::objVertexData(uint32_t pos, uint32_t norm) : v(pos), vt(0), vn(norm) 
{

}

bool operator == (const objVertexData& obj1, const objVertexData& obj2)
{
	return ((obj1.v == obj2.v) && (obj1.vt == obj2.vt) && (obj1.vn == obj2.vn));
}
