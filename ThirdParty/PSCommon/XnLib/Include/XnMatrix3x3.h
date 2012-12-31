/*****************************************************************************
*                                                                            *
*  PrimeSense PSCommon Library                                               *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of PSCommon.                                            *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/
#ifndef _XN_MATRIX_3X3_H_
#define _XN_MATRIX_3X3_H_

#include "XnVector3D.h"
#include "XnSymmetricMatrix3x3.h"

namespace xnl
{

class Matrix3x3
{
public:
	static Matrix3x3 Zero();
	static Matrix3x3 Identity();
	static Matrix3x3 Diagonal(const Vector3D& diag);
	static Matrix3x3 CrossProductMatrix(const Vector3D& vec);
	static Matrix3x3 XRotation(XnFloat angle);
	static Matrix3x3 YRotation(XnFloat angle);
	static Matrix3x3 ZRotation(XnFloat angle);
	static Matrix3x3 XRotationDerivative(XnFloat angle);
	static Matrix3x3 YRotationDerivative(XnFloat angle);
	static Matrix3x3 ZRotationDerivative(XnFloat angle);
	static Matrix3x3 FixedXYZRotation(XnFloat xAngle, XnFloat yAngle, XnFloat zAngle);
	static Matrix3x3 FixedXYZRotation(const Vector3D& angles);
	static Matrix3x3 MovingXYZRotation(XnFloat xAngle, XnFloat yAngle, XnFloat zAngle);
	static Matrix3x3 MovingXYZRotation(const Vector3D& angles);
	static Matrix3x3 FixedXYZRotationDerivative(const Vector3D &angles, const Vector3D &vec);

	Matrix3x3();
	Matrix3x3(XnFloat x00, XnFloat x01, XnFloat x02, XnFloat x10, XnFloat x11, XnFloat x12, XnFloat x20, XnFloat x21, XnFloat x22)
	{
		Set(x00, x01, x02, x10, x11, x12, x20, x21, x22);
	}
	Matrix3x3(const Vector3D& col0, const Vector3D& col1, const Vector3D& col2)
	{
		Set(col0, col1, col2);
	}
	explicit Matrix3x3(const SymmetricMatrix3x3& mat)
	{
		elements[0] = mat(0,0); elements[1] = mat(0,1); elements[2] = mat(0,2);
		elements[3] = mat(0,1); elements[4] = mat(1,1); elements[5] = mat(1,2);
		elements[6] = mat(0,2); elements[7] = mat(1,2); elements[8] = mat(2,2);
	}

	Matrix3x3& operator+=(const Matrix3x3& other);
	Matrix3x3& operator=(const Matrix3x3& other);
	XnFloat operator()(XnInt32 x, XnInt32 y) const { XN_ASSERT(0<=x && x<3 && 0<=y && y<3); return elements[3*x+y]; }
	XnFloat& operator()(XnInt32 x, XnInt32 y) { XN_ASSERT(0<=x && x<3 && 0<=y && y<3); return elements[3*x+y]; }
	XnFloat operator[](XnInt32 index) const { XN_ASSERT(0<=index && index<9); return elements[index]; }
	XnFloat& operator[](XnInt32 index) { XN_ASSERT(0<=index && index<9); return elements[index]; }

	void SetZero();
	void Set(XnFloat x00, XnFloat x01, XnFloat x02, XnFloat x10, XnFloat x11, XnFloat x12, XnFloat x20, XnFloat x21, XnFloat x22)
	{
		elements[0] = x00; elements[1] = x01; elements[2] = x02;
		elements[3] = x10; elements[4] = x11; elements[5] = x12;
		elements[6] = x20; elements[7] = x21; elements[8] = x22;
	}
	void Set(const Vector3D& col0, const Vector3D& col1, const Vector3D& col2)
	{
		Set(col0.x, col1.x, col2.x,
			col0.y, col1.y, col2.y,
			col0.z, col1.z, col2.z);
	}

	Vector3D GetColumn(XnInt32 col) const { return Vector3D((*this)(0,col),(*this)(1,col),(*this)(2,col)); }
	void SetColumn(XnInt32 col, const Vector3D& vec) { for(int i=0;i<3;i++) (*this)(i,col)=vec[i]; }
	Matrix3x3 Transposed() const
	{
		return Matrix3x3(elements[0], elements[3], elements[6],
			elements[1], elements[4], elements[7],
			elements[2], elements[5], elements[8]);

	}

	void ScaleColumn(XnInt32 col, XnFloat scale);

	Matrix3x3 operator-() const;

	Matrix3x3 operator+(const Matrix3x3& other) const;
	Matrix3x3 operator-(const Matrix3x3& other) const;
	Matrix3x3 operator*(const Matrix3x3& other) const
	{
		return Matrix3x3(
			elements[0]*other.elements[0] + elements[1]*other.elements[3] + elements[2]*other.elements[6],
			elements[0]*other.elements[1] + elements[1]*other.elements[4] + elements[2]*other.elements[7],
			elements[0]*other.elements[2] + elements[1]*other.elements[5] + elements[2]*other.elements[8],

			elements[3]*other.elements[0] + elements[4]*other.elements[3] + elements[5]*other.elements[6],
			elements[3]*other.elements[1] + elements[4]*other.elements[4] + elements[5]*other.elements[7],
			elements[3]*other.elements[2] + elements[4]*other.elements[5] + elements[5]*other.elements[8],

			elements[6]*other.elements[0] + elements[7]*other.elements[3] + elements[8]*other.elements[6],
			elements[6]*other.elements[1] + elements[7]*other.elements[4] + elements[8]*other.elements[7],
			elements[6]*other.elements[2] + elements[7]*other.elements[5] + elements[8]*other.elements[8]);

	}
	Matrix3x3 operator*(XnFloat a) const;
	Matrix3x3 operator/(XnFloat a) const;
	friend Matrix3x3 operator*(XnFloat a, const Matrix3x3 &mat);

	Vector3D operator*(const Vector3D& vec) const;
	Vector3D TransposeTimes(const Vector3D& vec) const;
	
	Matrix3x3& operator-=(const Matrix3x3& other);
	Matrix3x3& operator*=(const Matrix3x3& other);
	Matrix3x3& operator*=(XnFloat a);

	// this*other^T
	Matrix3x3 MultipliedWithTranspose(const Matrix3x3& other) const;
	void MultiplywithTranspose(const Matrix3x3& other);

	XnFloat MaxAbs() const;
	XnFloat MinAbs() const;

	Matrix3x3 CofactorMatrix() const;
	Matrix3x3 AdjugateMatrix() const;
	XnFloat Determinant() const;

	XnBool Invert(XnFloat tolerance=1e-8);
	Matrix3x3 Inverse(XnFloat tolerance=1e-8);

	XnFloat Trace() const;

	Vector3D GetDiagonal() const;
	void SetDiagonal(const Vector3D& diag);

	Vector3D LargestColumnNormalized() const;

	void Transpose();

	void Rotate180AroundX();
	void Rotate180AroundY();
	void Rotate180AroundZ();

	Matrix3x3 PolarDecompositionRotationPart() const;
	void PolarDecompositionRotationAndScale(Matrix3x3& rotation, Vector3D& scales, Matrix3x3& scaleAxes) const;

	Matrix3x3 PseudoInverse(XnFloat tolerance=1e-8) const;
	void SingularValueDecomposition(Matrix3x3& U, Vector3D& singularValues, Matrix3x3& sV, XnFloat tolerance=1e-8) const;
	void ActualSingularValueDecomposition(Matrix3x3 &U, Vector3D &singularValues, Matrix3x3 &V, XnFloat tolerance=1e-8) const;

	SymmetricMatrix3x3 NormalEquationsMatrix() const;
	SymmetricMatrix3x3 ToSymmetricMatrix() const
	{
		return SymmetricMatrix3x3((*this)(0,0), (*this)(0,1), (*this)(0,2), (*this)(1,1), (*this)(1,2), (*this)(2,2));
	}
	XnBool IsSymmetric() const;

	XnInt32 GetEigenValues(XnFloat& val1, XnFloat& val2, XnFloat& val3) const;
	XnInt32 GetEigenValues(XnFloat* pEigenValues) const;
	XnInt32 GetEigenValues(Vector3D& eigenValues) const;
	Vector3D GetEigenVector(XnFloat eigenValue) const;
private:
	XnInt32 GetEigenValuesNoScaling(XnFloat& val1, XnFloat& val2, XnFloat& val3) const;
	XnInt32 GetEigenValuesWithScaling(XnFloat& val1, XnFloat& val2, XnFloat& val3) const;
public:
	void GetFixedXYZRotationAngles(XnFloat &xAngle, XnFloat &yAngle, XnFloat &zAngle) const;
	void GetFixedXYZRotationAngles(Vector3D &angles) const;
	void GetMovingXYZRotationAngles(XnFloat &xAngle, XnFloat &yAngle, XnFloat &zAngle) const;
	void GetMovingXYZRotationAngles(Vector3D &angles) const;
	// Aliases
	static Matrix3x3 FixedZYXRotation(XnFloat xAngle, XnFloat yAngle, XnFloat zAngle);
	static Matrix3x3 FixedZYXRotation(const Vector3D &angles);
	static Matrix3x3 MovingZYXRotation(XnFloat xAngle, XnFloat yAngle, XnFloat zAngle);
	static Matrix3x3 MovingZYXRotation(const Vector3D &angles);
	static Matrix3x3 MovingZYXRotationDerivative(const Vector3D &angles, const Vector3D &vec);
	void GetMovingZYXRotationAngles(XnFloat &xAngle, XnFloat &yAngle, XnFloat &zAngle) const;
	void GetMovingZYXRotationAngles(Vector3D &angles) const;

	static Matrix3x3 AxialRotation(const Vector3D &v, XnFloat radians);
	XnFloat GetRotationCosineAngle() const;
	XnFloat GetRotationAngle() const;
	void FillColumnUsingCrossProduct(XnInt32 col);
	XnBool IsOrthogonal() const;
	XnBool IsOrthonormal() const;

	static Matrix3x3 PitchRollMatrix(XnFloat pitch, XnFloat roll);
	static Matrix3x3 OuterProduct(const Vector3D &v);
	static Matrix3x3 OrthonormalMatrixFromOneAxis(const Vector3D &x, XnInt32 axis=0);

	static XnBool OrthonormalMatrixFromTwoDirections(const Vector3D &vec1, const Vector3D &vec2, Matrix3x3 &matrix, XnFloat epsilon = 1e-2);
private:
	XnFloat elements[9];

};

inline XnBool IsNaN(const Matrix3x3 &mat) { 
	for(XnInt32 i=0; i<9; i++) if(Math::IsNaN(mat[i])) return true;
	return false;
}

inline Matrix3x3 operator*(const SymmetricMatrix3x3 &A, const SymmetricMatrix3x3 &B)
{
	return Matrix3x3(A) * Matrix3x3(B);
}

inline SymmetricMatrix3x3 Algb_ABAt(const Matrix3x3 &A, const SymmetricMatrix3x3 &B)
{
	// TODO: make optimized version
	return (A * Matrix3x3(B) * A.Transposed()).ToSymmetricMatrix();
}

inline SymmetricMatrix3x3 Algb_ApAt(const Matrix3x3 &A)
{
	return SymmetricMatrix3x3(2*A(0,0), A(0,1)+A(1,0), A(0,2)+A(2,0),
		2*A(1,1), A(1,2)+A(2,1),
		2*A(2,2));
}

//ARCCHANGE: static Matrix3X3<T> OuterProduct(const Vector3D<T> &v1, const Vector3D<T> &v2)
inline Matrix3x3 OuterProduct(const Vector3D &v1, const Vector3D &v2)
{
	return Matrix3x3(v1.x*v2.x, v1.x*v2.y, v1.x*v2.z,
		v1.y*v2.x, v1.y*v2.y, v1.y*v2.z,
		v1.z*v2.x, v1.z*v2.y, v1.z*v2.z);
}

// result = u + Av
inline void Algb_upAv(const Vector3D &u, const Matrix3x3 &A, const Vector3D &v, Vector3D &result)
{
	result.Set(u.x + A[0]*v.x+A[1]*v.y+A[2]*v.z,
		u.y + A[3]*v.x+A[4]*v.y+A[5]*v.z,
		u.z + A[6]*v.x+A[7]*v.y+A[8]*v.z);
}
inline Vector3D Algb_upAv(const Vector3D &u, const Matrix3x3 &A, const Vector3D &v)
{
	return Vector3D(u.x + A[0]*v.x+A[1]*v.y+A[2]*v.z,
		u.y + A[3]*v.x+A[4]*v.y+A[5]*v.z,
		u.z + A[6]*v.x+A[7]*v.y+A[8]*v.z);
}
// result = u - Av
inline void Algb_umAv(const Vector3D &u, const Matrix3x3 &A, const Vector3D &v, Vector3D &result)
{
	result.Set(u.x - (A[0]*v.x+A[1]*v.y+A[2]*v.z),
		u.y - (A[3]*v.x+A[4]*v.y+A[5]*v.z),
		u.z - (A[6]*v.x+A[7]*v.y+A[8]*v.z));
}

// Uses Gram-Schmidt algorithm to orthonormalize matrix
inline void ApplyGramSchmidt(Matrix3x3 &A)
{
	Vector3D c[3];
	for(XnInt32 i=0; i<3; i++) c[i] = A.GetColumn(i);
	for(XnInt32 i=0; i<3; i++) {
		for(XnInt32 j=0; j<i; j++) c[i] -= c[i].ProjectedOnUnitVector(c[j]);
		c[i].Normalize();
	}
	A.Set(c[0], c[1], c[2]);
}

} // xnl


#endif // _XN_MATRIX_3X3_H_
