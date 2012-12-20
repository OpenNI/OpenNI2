/*****************************************************************************
*                                                                            *
*  OpenNI 2.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
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
#include "LinAlg.h"
#include <math.h>

/***************************************************
* Utility functions
***************************************************/
#if (XN_REAL == XnFloat)
	#define R_EPS 1.E-4   // Single Precision is  6 decimal digits
#elif (XN_REAL == XnDouble)
	#define R_EPS 1.E-12  // Double Precision is 15 decimal digits
#else
	#error "XN_REAL type is not supported!"
#endif

// checks if a real number is smaller than epsilon (and so, efficiently zero)
XnBool IsZero(XN_REAL r)
{
	return (r < R_EPS && -r < R_EPS);
}

// swaps two unsigned integers
void SwapUI(XnUInt32* pA, XnUInt32* pB)
{
	XnUInt32 temp = *pA;
	*pA = *pB;
	*pB = temp;
}

// swaps two real numbers
void SwapR(XN_REAL* pA, XN_REAL* pB)
{
	XN_REAL temp = *pA;
	*pA = *pB;
	*pB = temp;
}

/***************************************************
* Vectors
***************************************************/
#define MATRIX_SIZE	3

typedef XN_REAL XnRealVector[MATRIX_SIZE], PXnRealVector[];
typedef XnUInt32 XnUIntVector[MATRIX_SIZE], PXnUIntVector[];

// sets all values in the vector to dVal
void XnRealVectorSet(PXnRealVector pVector, const XN_REAL dVal)
{
	XnUInt32 nIndex;
	for (nIndex = 0; nIndex < MATRIX_SIZE; ++nIndex)
		pVector[nIndex] = dVal;
}

// sets all values in the vector to nVal
void XnUIntVectorSet(PXnUIntVector pVector, const XnUInt32 nVal)
{
	XnUInt32 nIndex;
	for (nIndex = 0; nIndex < MATRIX_SIZE; ++nIndex)
		pVector[nIndex] = nVal;
}

// copies values from another vector
void XnRealVectorCopy(PXnRealVector pVector, const PXnRealVector pOtherVector)
{
	XnUInt32 nIndex;
	for (nIndex = 0; nIndex < MATRIX_SIZE; ++nIndex)
		pVector[nIndex] = pOtherVector[nIndex];
}

/***************************************************
* Matrices
***************************************************/

typedef XnRealVector XnRealMatrix[MATRIX_SIZE], PXnRealMatrix[];

// sets all values in a matrix to dVal
void XnRealMatrixSet(PXnRealMatrix pMatrix, const XN_REAL dVal)
{
	XnUInt32 nIndex;
	for (nIndex = 0; nIndex < MATRIX_SIZE; ++nIndex)
		XnRealVectorSet(pMatrix[nIndex], dVal);
}

// copies values from anohter matrix
void XnRealMatrixCopy(PXnRealMatrix pMatrix, const PXnRealMatrix pOtherMatrix)
{
	XnUInt32 nIndex;
	for (nIndex = 0; nIndex < MATRIX_SIZE; ++nIndex)
		XnRealVectorCopy(pMatrix[nIndex], pOtherMatrix[nIndex]);
}

/***************************************************
* LU decomposition - breaks a matrix into lower- and upper-triangular forms
***************************************************/
typedef struct LUDecomposition
{
	XnRealMatrix m_Matrix;
	XnRealMatrix m_LU;
	XnRealVector m_Solution;
	XnInt32 m_nD;
	XnUIntVector m_Indices;
} LUDecomposition;

// initializes the struct with the original matrix
void XnLUReset(LUDecomposition* pLUDecomp, PXnRealMatrix pMatrix)
{
	XnRealMatrixCopy(pLUDecomp->m_Matrix, pMatrix);
	XnRealMatrixCopy(pLUDecomp->m_LU, pMatrix);
	XnUIntVectorSet(pLUDecomp->m_Indices, 0);
	XnRealVectorSet(pLUDecomp->m_Solution, 0.0);
	pLUDecomp->m_nD = 1;
}

// Decomposes the matrix into LUP form where:  P X = LU 
// P is the pivot matrix (stored here as a vector) and L and U are lower and upper
// triangular matrices. The main diagonal of L is ones ==> The determinant of the matrix 
// is determined by U.
void XnDecomposeToLU(LUDecomposition* pLUDecomp)
{
	XnInt32 i, k, ii, inner, iii, j;
	XN_REAL p;
	XnInt32 kprime;

	for(i = 0 ; i < MATRIX_SIZE ; ++i)
		pLUDecomp->m_Indices[i] = i;

	for(k = 0 ; k < MATRIX_SIZE-1 ; ++k)
	{
		p = 0.0;
		kprime = k;

		for(ii = k; ii < MATRIX_SIZE; ++ii)
		{
			if(fabs(pLUDecomp->m_LU[ii][k]) > p)
			{
				p = fabs(pLUDecomp->m_LU[ii][k]);
				kprime = ii;
			}
		}

		assert(!IsZero(p));

		if(k != kprime)
		{
			//then we must flip!!
			pLUDecomp->m_nD = -pLUDecomp->m_nD;

			//this is the index vector that LUSolv Understands--
			//WE DO NOT FLIP THE INDICIES:: PLEASE DO THINK THIS IS
			//NOT A MISTAKE (it is a feature and works!!)
			pLUDecomp->m_Indices[k]= kprime;
			SwapUI(&pLUDecomp->m_Indices[k], &pLUDecomp->m_Indices[kprime]);

			for(inner = 0; inner < MATRIX_SIZE; ++inner)
				SwapR(&pLUDecomp->m_LU[k][inner], &pLUDecomp->m_LU[kprime][inner]);

		}

		for(iii = k + 1 ; iii < MATRIX_SIZE ; ++iii)
		{
			//check for division by zero:
			assert(!IsZero(pLUDecomp->m_LU[k][k]));

			pLUDecomp->m_LU[iii][k] /= pLUDecomp->m_LU[k][k];

			for(j = k + 1 ; j < MATRIX_SIZE ; ++j)
				pLUDecomp->m_LU[iii][j] -= pLUDecomp->m_LU[iii][k] * pLUDecomp->m_LU[k][j];

		}
	}
}

// performs forward and backwards substitution to obtain the solution vector.
// b is the right hand side vector and the solution is stored in m_Soln.
// LUx = b 
// ==> Ly = b (forward solution) and then 
// ==> Ux = y (backward solution) provides the solution: x.
void XnLUSolution(LUDecomposition* pLUDecomp, PXnRealMatrix pMatrix, PXnRealVector pB)
{
	XnInt32 i, ii = -1, ip, j;
	XN_REAL sum;

	XnLUReset(pLUDecomp, pMatrix);

	XnDecomposeToLU(pLUDecomp);

	XnRealVectorCopy(pLUDecomp->m_Solution, pB);

	//forward substitution of L via P
	for (i = 0; i < MATRIX_SIZE; ++i) 
	{
		ip = pLUDecomp->m_Indices[i];
		sum = pLUDecomp->m_Solution[ip];
		pLUDecomp->m_Solution[ip] = pLUDecomp->m_Solution[i];

		if (ii >= 0)
		{
			for (j = ii; j < i; ++j) 
				sum -= pLUDecomp->m_LU[i][j] * pLUDecomp->m_Solution[j];
		}
		else
		{
			if(!IsZero(sum)) 
				ii = i;
		}

		pLUDecomp->m_Solution[i] = sum;
	}

	//backward substitution of U
	for (i = MATRIX_SIZE-1; i >= 0; --i) 
	{
		sum = pLUDecomp->m_Solution[i];

		for (j = i + 1; j < MATRIX_SIZE ; ++j) 
			sum -= pLUDecomp->m_LU[i][j] * pLUDecomp->m_Solution[j];

		pLUDecomp->m_Solution[i] = sum / pLUDecomp->m_LU[i][i];
	}
}

/***************************************************
* Linear Fitting
***************************************************/
void DoLinearFitting(XN_REAL x[], XN_REAL y[], XN_REAL z[], XnUInt32 nVals, XN_REAL* pA, XN_REAL* pB, XN_REAL* pC)
{
	XnUInt32 nIndex;
	LUDecomposition LU;
	XnRealMatrix matrix;
	XnRealVector vector;
	XN_REAL X;
	XN_REAL Y;
	XN_REAL Z;

	//matrix
	XN_REAL 
		x1 = 0, y1 = 0, x2 = 0, x1y1 = 0, y2 = 0,

		//vector:
		z1 = 0, z1x1 = 0, z1y1 = 0;


	for(nIndex = 0; nIndex < nVals; ++nIndex)
	{
		X = *x++;
		Y = *y++;
		Z = *z++;
		x1 += X;
		y1 += Y;
		x2 += X*X;
		x1y1 += X*Y;
		y2 += Y*Y;
		z1 += Z;
		z1x1 += Z*X;
		z1y1 += Z*Y;
	}

	//left hand side
	XnRealMatrixSet(matrix, 0.0);

	matrix[0][0] = nVals;
	matrix[0][1] = x1;
	matrix[0][2] = y1;

	matrix[1][0] = x1;
	matrix[1][1] = x2;
	matrix[1][2] = x1y1;

	matrix[2][0] = y1;
	matrix[2][1] = x1y1;
	matrix[2][2] = y2;

	//right hand side
	XnRealVectorSet(vector, 0.0);

	vector[0] = z1;
	vector[1] = z1x1;
	vector[2] = z1y1;

	XnLUSolution(&LU, matrix, vector);

	*pC = LU.m_Solution[0];
	*pA = LU.m_Solution[1];
	*pB = LU.m_Solution[2];
}
