#ifndef MATRIX_H
#define MATRIX_H

#include "Vector.h"

//////////////////////////////////////////////////////////////////////
// Matrix 4x4
//////////////////////////////////////////////////////////////////////

class CMatrix
{
public:
    union
    {
        struct
        {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };

        struct
        {
            CVector i; float _14;
            CVector j; float _24;
            CVector k; float _34;
            CVector c; float _44;
        };

        float m[4][4];
    };

public:
    CMatrix();
    ~CMatrix();

    void SetIdentity();

    // Orthogonal Projection
    void OrthoOffCenterLH( float fLeft, float fRight, float fBottom, float fTop, float fZNear, float fZFar );
};

#endif // !MATRIX_H
