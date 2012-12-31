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
#ifndef _XN_SYMMETRIC_MATRIX_3X3_H_
#define _XN_SYMMETRIC_MATRIX_3X3_H_

#include <XnPlatform.h>
#include "XnLib.h"
#include "XnVector3D.h"

namespace xnl
{

class Matrix3x3;

class SymmetricMatrix3x3
{
public:
	static SymmetricMatrix3x3 Zero() {return SymmetricMatrix3x3::Diagonal(Vector3D::Zero());}
	static SymmetricMatrix3x3 Identity() {return SymmetricMatrix3x3::Diagonal(Vector3D::All(1));}
	static SymmetricMatrix3x3 Diagonal(float x00, float x11, float x22) {return SymmetricMatrix3x3(x00, 0, 0, x11, 0, x22);}
	static SymmetricMatrix3x3 Diagonal(const Vector3D& vec) {return SymmetricMatrix3x3::Diagonal(vec.x, vec.y, vec.z);}

	SymmetricMatrix3x3()
	{
		SetZero();
	}
	SymmetricMatrix3x3(const SymmetricMatrix3x3& other)
	{
		*this = other;
	}
	SymmetricMatrix3x3(float x00, float x01, float x02, float x11, float x12, float x22)
	{
		Set(x00, x01, x02, x11, x12, x22);
	}

	void Set(float x00, float x01, float x02, float x11, float x12, float x22)
	{
		m_x00 = x00; m_x01=x01; m_x02=x02; m_x11=x11; m_x12=x12; m_x22=x22;
	}
	void SetZero()
	{
		*this = SymmetricMatrix3x3::Zero();
	}
	SymmetricMatrix3x3& operator=(const SymmetricMatrix3x3& other)
	{
		Set(other.m_x00, other.m_x01, other.m_x02, other.m_x11, other.m_x12, other.m_x22);

		return *this;
	}

	SymmetricMatrix3x3 operator+(const SymmetricMatrix3x3& other) const
	{
		SymmetricMatrix3x3 result(*this);
		result += other;
		return result;
	}
	SymmetricMatrix3x3 operator-(const SymmetricMatrix3x3& other) const
	{
		SymmetricMatrix3x3 result(*this);
		result -= other;
		return result;
	}

	SymmetricMatrix3x3 operator*(float a) const
	{
		SymmetricMatrix3x3 result(*this);
		result *= a;
		return result;
	}
	friend SymmetricMatrix3x3 operator*(float a, SymmetricMatrix3x3& mat)
	{
		return mat*a;
	}
	Vector3D operator*(const Vector3D& vec) const
	{
		return Vector3D(m_x00*vec.x+m_x01*vec.y+m_x02*vec.z,
						m_x01*vec.x+m_x11*vec.y+m_x12*vec.z,
						m_x02*vec.x+m_x12*vec.y+m_x22*vec.z);
	}

	SymmetricMatrix3x3 operator-() const
	{
		SymmetricMatrix3x3 mat = *this;
		mat*= -1;
		return mat;
	}

	SymmetricMatrix3x3 operator-(float a) const
	{
		return SymmetricMatrix3x3(m_x00-a,m_x01,m_x02,m_x11-a,m_x12,m_x22-a);
	}

	SymmetricMatrix3x3& operator*=(float a)
	{
		m_x00 *= a; m_x01 *= a; m_x02 *= a;
		m_x11 *= a; m_x12 *= a;
		m_x22 *= a;

		return *this;
	}
	SymmetricMatrix3x3& operator+=(const SymmetricMatrix3x3& other)
	{
		m_x00 += other.m_x00; m_x01 += other.m_x01; m_x02 += other.m_x02;
		m_x11 += other.m_x11; m_x12 += other.m_x12;
		m_x22 += other.m_x22;

		return *this;
	}
	SymmetricMatrix3x3& operator-=(const SymmetricMatrix3x3& other)
	{
		m_x00 -= other.m_x00; m_x01 -= other.m_x01; m_x02 -= other.m_x02;
		m_x11 -= other.m_x11; m_x12 -= other.m_x12;
		m_x22 -= other.m_x22;

		return *this;
	}

	void AddOuterProduct(const Vector3D& vec)
	{
		m_x00 += vec.x*vec.x;
		m_x01 += vec.x*vec.y;
		m_x02 += vec.x*vec.z;
		m_x11 += vec.y*vec.y;
		m_x12 += vec.y*vec.z;
		m_x22 += vec.z*vec.z;
	}
	void AddWeightedOuterProduct(float weight, const Vector3D& vec)
	{
		m_x00 += weight*vec.x*vec.x;
		m_x01 += weight*vec.x*vec.y;
		m_x02 += weight*vec.x*vec.z;
		m_x11 += weight*vec.y*vec.y;
		m_x12 += weight*vec.y*vec.z;
		m_x22 += weight*vec.z*vec.z;

	}
	void SubtractOuterProduct(const Vector3D& vec)
	{
		m_x00 -= vec.x*vec.x;
		m_x01 -= vec.x*vec.y;
		m_x02 -= vec.x*vec.z;
		m_x11 -= vec.y*vec.y;
		m_x12 -= vec.y*vec.z;
		m_x22 -= vec.z*vec.z;
	}

	float Trace() const
	{
		return m_x00+m_x11+m_x22;
	}
	float MaxAbs() const;

	Vector3D LargestColumnNormalized() const
	{
		float scales[3] = {Math::Sqr(m_x00) + Math::Sqr(m_x01) + Math::Sqr(m_x02), Math::Sqr(m_x01) + Math::Sqr(m_x11) + Math::Sqr(m_x12), Math::Sqr(m_x02) + Math::Sqr(m_x12) + Math::Sqr(m_x22)};
		int i = Math::ArgMax(scales[0], scales[1], scales[2]);
		if(scales[i] == 0) return Vector3D(1,0,0);
		else {
			switch(i) {
			case 0: return Vector3D(m_x00,m_x01,m_x02)/Math::Sqrt(scales[i]); break;
			case 1: return Vector3D(m_x01,m_x11,m_x12)/Math::Sqrt(scales[i]); break;
			case 2: return Vector3D(m_x02,m_x12,m_x22)/Math::Sqrt(scales[i]); break;
			default: XN_ASSERT(false); return Vector3D(1,0,0); break;
			}
		}
	}

	SymmetricMatrix3x3 CofactorMatrix() const
	{
		return SymmetricMatrix3x3(m_x11*m_x22 - m_x12*m_x12, m_x12*m_x02 - m_x01*m_x22, m_x01*m_x12 - m_x11*m_x02, m_x00*m_x22 - m_x02*m_x02, m_x01*m_x02 - m_x00*m_x12, m_x00*m_x11 - m_x01*m_x01);
	}

	float Determinant() const
	{
		float cofactor00 = m_x11*m_x22 - m_x12*m_x12;
		float cofactor01 = m_x12*m_x02 - m_x01*m_x22;
		float cofactor02 = m_x01*m_x12 - m_x11*m_x02;
		return m_x00 * cofactor00 + m_x01*cofactor01 + m_x02*cofactor02;
	}

	bool GetInverse(SymmetricMatrix3x3& inv, float tolerance=1e-8) const
	{
		float cofactor00 = m_x11*m_x22 - m_x12*m_x12;
		float cofactor01 = m_x12*m_x02 - m_x01*m_x22;
		float cofactor02 = m_x01*m_x12 - m_x11*m_x02;
		float det = m_x00 * cofactor00 + m_x01*cofactor01 + m_x02*cofactor02;
		if(Math::Abs(det)<=tolerance) return false;
		float oneOverDet = 1/det;
		inv =  SymmetricMatrix3x3(cofactor00*oneOverDet, cofactor01*oneOverDet, cofactor02*oneOverDet,
									(m_x00*m_x22 - m_x02*m_x02)*oneOverDet, (m_x01*m_x02 - m_x00*m_x12)*oneOverDet,
									(m_x00*m_x11 - m_x01*m_x01)*oneOverDet);
		return true;
	}
	SymmetricMatrix3x3 Inverse(float tolerance=1e-8) const
	{
		SymmetricMatrix3x3 inv;
		if(GetInverse(inv,tolerance)) return inv;
		else return SymmetricMatrix3x3();
	}
	void Invert(float tolerance=1e-8)
	{
		*this = Inverse(tolerance);
	}

	// vec^T * mat * vec
	float MatrixInnerProduct(const Vector3D& vec) const
	{
		return Math::Sqr(vec.x)*m_x00 + Math::Sqr(vec.y)*m_x11 + Math::Sqr(vec.z)*m_x22 + 2*vec.x*vec.y*m_x01 + 2*vec.x*vec.z*m_x02 + 2*vec.y*vec.z*m_x12;
	}

	// Eigenvalues
	Vector3D GetEigenValues() const;
	void GetEigenVector(float lambda, Vector3D& vec) const;
	void GetEigenVectors(const Vector3D& lambdas, Matrix3x3& eigenVectors, float tolerance=1e-8) const;
	void SolveEigenProblem(Vector3D& eigenValues, Matrix3x3& eigenVectors, float tolerance=1e-8) const;
	void SolveSingleEigenProblem(int i, Vector3D& eigenValues, Vector3D& eigenVector, float tolerance=1e-8) const;

	friend SymmetricMatrix3x3 OuterProduct(const Vector3D& vec)
	{
		return SymmetricMatrix3x3(vec.x*vec.x, vec.x*vec.y, vec.x*vec.z, vec.y*vec.y, vec.y*vec.z, vec.z*vec.z);
	}

	float operator()(int x, int y) const
	{
		XN_ASSERT(0 <= x && x < 3 && 0 <= y && y < 3);

		if (x == 0 && y == 0) return m_x00;
		if ((x == 0 && y == 1) || (x == 1 && y == 0)) return m_x01;
		if ((x == 0 && y == 2) || (x == 2 && y == 0)) return m_x02;
		if (x == 1 && y == 1) return m_x11;
		if ((x == 1 && y == 2) || (x == 2 && y == 1)) return m_x12;
		if (x == 2 && y == 2) return m_x22;

		return m_x00;
	}
	float& operator()(int x, int y)
	{
		XN_ASSERT(0 <= x && x < 3 && 0 <= y && y < 3);

		if (x == 0 && y == 0) return m_x00;
		if ((x == 0 && y == 1) || (x == 1 && y == 0)) return m_x01;
		if ((x == 0 && y == 2) || (x == 2 && y == 0)) return m_x02;
		if (x == 1 && y == 1) return m_x11;
		if ((x == 1 && y == 2) || (x == 2 && y == 1)) return m_x12;
		if (x == 2 && y == 2) return m_x22;

		return m_x00;
	}
private:
	Vector3D GetEigenValuesNoScaling() const;
	void SolveEigenProblemNoScaling(Vector3D& eigenValues, Matrix3x3& eigenVectors, float tolerance=1e-8) const;
	Vector3D GetEigenValuesWithScaling() const;
	void SolveEigenProblemWithScaling(Vector3D& eigenValues, Matrix3x3& eigenVectors, float tolerance=1e-8) const;

	float m_x00, m_x01, m_x02, m_x11, m_x12, m_x22;
};

} // xnl

#endif // _XN_SYMMETRIC_MATRIX_3X3_H_
