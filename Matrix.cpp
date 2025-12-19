#include <string.h>
#include "Matrix.h"

CMatrix::CMatrix()
{
	memset( m, 0, sizeof( m ) );
}

CMatrix::~CMatrix()
{
}

void CMatrix::SetIdentity()
{
	_11 = 1.0f;
	_22 = 1.0f;
	_33 = 1.0f;
	_44 = 1.0f;
}

void CMatrix::OrthoOffCenterLH( float fLeft, float fRight, float fBottom, float fTop, float fZNear, float fZFar )
{
	m[ 0 ][ 0 ] = 2.0f / (fRight - fLeft);
	m[ 1 ][ 1 ] = 2.0f / (fTop - fBottom);
	m[ 2 ][ 2 ] = 1.0f / (fZFar - fZNear);
	m[ 3 ][ 0 ] = -1.0f - 2.0f * fLeft / (fRight - fLeft);
	m[ 3 ][ 1 ] = 1.0f + 2.0f * fTop / (fBottom - fTop);
	m[ 3 ][ 2 ] = fZNear / (fZNear - fZFar);
	m[ 3 ][ 3 ] = 1.0f;
}
