#include "vector.h"

CVector2::CVector2()
{
	x = y = 0.0f;
}
CVector2::CVector2(float value)
{
	x = y = value;
}

CVector2::CVector2(float _x, float _y)
{
	x = _x;
	y = _y;
}

/////////////////////////////////////////////////////////////////////

CVector::CVector()
{
	x = y = z = 0.0f;
}

CVector::CVector(float value)
{
	x = y = z = value;
}

CVector::CVector(float _x, float _y, float _z)
{
	x = _x;
	y = _y;
	z = _z;
}
