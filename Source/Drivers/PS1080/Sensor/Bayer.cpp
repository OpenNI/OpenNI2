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
//---------------------------------------------------------------------------
// Many thanks to ROS guys for the improved algorithm!
//---------------------------------------------------------------------------

// Includes
#include "Bayer.h"

#define AVG(a,b) (((int)(a) + (int)(b)) >> 1)
#define AVG3(a,b,c) (((int)(a) + (int)(b) + (int)(c)) / 3)
#define AVG4(a,b,c,d) (((int)(a) + (int)(b) + (int)(c) + (int)(d)) >> 2)
#define WAVG4(a,b,c,d,x,y)  (unsigned char)( ( ((int)(a) + (int)(b)) * (int)(x) + ((int)(c) + (int)(d)) * (int)(y) ) / ( 2 * ((int)(x) + (int(y))) ) )

typedef enum
{
	Bilinear = 0,
	EdgeAware,
	EdgeAwareWeighted
} DebayeringMethod;

void fillRGB(unsigned width, unsigned height, const XnUInt8* bayer_pixel, unsigned char* rgb_buffer, DebayeringMethod debayering_method, XnUInt32 nDownSampleStep)
{
	unsigned rgb_line_step = width * 3;
//---------------------------------------------------------------------------
// Code
	unsigned rgb_line_skip = rgb_line_step - width * 3;
//---------------------------------------------------------------------------
	if (nDownSampleStep == 1)
	{
		//register const XnUInt8 *bayer_pixel = image_md_->Data ();
		register unsigned yIdx, xIdx;
		
		int bayer_line_step = width;
		int bayer_line_step2 = width << 1;
		
		if (debayering_method == Bilinear)
		{
			// first two pixel values for first two lines
			// Bayer         0 1 2
			//         0     G r g
			// line_step     b g b
			// line_step2    g r g
			
			rgb_buffer[3] = rgb_buffer[0] = bayer_pixel[1]; // red pixel
			rgb_buffer[1] = bayer_pixel[0]; // green pixel
			rgb_buffer[rgb_line_step + 2] = rgb_buffer[2] = bayer_pixel[bayer_line_step]; // blue;
			
			// Bayer         0 1 2
			//         0     g R g
			// line_step     b g b
			// line_step2    g r g
			//rgb_pixel[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG3 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1]);
			rgb_buffer[rgb_line_step + 5] = rgb_buffer[5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
			
			// BGBG line
			// Bayer         0 1 2
			//         0     g r g
			// line_step     B g b
			// line_step2    g r g
			rgb_buffer[rgb_line_step + 3] = rgb_buffer[rgb_line_step ] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
			rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step + 1], bayer_pixel[bayer_line_step2]);
			//rgb_pixel[rgb_line_step + 2] = bayer_pixel[line_step];
			
			// pixel (1, 1)  0 1 2
			//         0     g r g
			// line_step     b G b
			// line_step2    g r g
			//rgb_pixel[rgb_line_step + 3] = AVG( bayer_pixel[1] , bayer_pixel[line_step2+1] );
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			//rgb_pixel[rgb_line_step + 5] = AVG( bayer_pixel[line_step] , bayer_pixel[line_step+2] );
			
			rgb_buffer += 6;
			bayer_pixel += 2;
			// rest of the first two lines
			
			for (xIdx = 2; xIdx < width - 2; xIdx += 2, rgb_buffer += 6, bayer_pixel += 2)
			{
				// GRGR line
				// Bayer        -1 0 1 2
				//           0   r G r g
				//   line_step   g b g b
				// line_step2    r g r g
				rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
				rgb_buffer[1] = bayer_pixel[0];
				rgb_buffer[2] = bayer_pixel[bayer_line_step + 1];
				
				// Bayer        -1 0 1 2
				//          0    r g R g
				//  line_step    g b g b
				// line_step2    r g r g
				rgb_buffer[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG3 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1]);
				rgb_buffer[rgb_line_step + 5] = rgb_buffer[5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
				
				// BGBG line
				// Bayer         -1 0 1 2
				//         0      r g r g
				// line_step      g B g b
				// line_step2     r g r g
				rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
				rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
				rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
				
				// Bayer         -1 0 1 2
				//         0      r g r g
				// line_step      g b G b
				// line_step2     r g r g
				rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				//rgb_pixel[rgb_line_step + 5] = AVG( bayer_pixel[line_step] , bayer_pixel[line_step+2] );
			}

			// last two pixel values for first two lines
			// GRGR line
			// Bayer        -1 0 1
			//           0   r G r
			//   line_step   g b g
			// line_step2    r g r
			rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
			rgb_buffer[1] = bayer_pixel[0];
			rgb_buffer[rgb_line_step + 5] = rgb_buffer[rgb_line_step + 2] = rgb_buffer[5] = rgb_buffer[2] = bayer_pixel[bayer_line_step];
			
			// Bayer        -1 0 1
			//          0    r g R
			//  line_step    g b g
			// line_step2    r g r
			rgb_buffer[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG (bayer_pixel[0], bayer_pixel[bayer_line_step + 1]);
			//rgb_pixel[5] = bayer_pixel[line_step];
			
			// BGBG line
			// Bayer        -1 0 1
			//          0    r g r
			//  line_step    g B g
			// line_step2    r g r
			rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
			rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
			//rgb_pixel[rgb_line_step + 2] = bayer_pixel[line_step];
			
			// Bayer         -1 0 1
			//         0      r g r
			// line_step      g b G
			// line_step2     r g r
			rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			//rgb_pixel[rgb_line_step + 5] = bayer_pixel[line_step];
			
			bayer_pixel += bayer_line_step + 2;
			rgb_buffer += rgb_line_step + 6 + rgb_line_skip;
			
			// main processing
			
			for (yIdx = 2; yIdx < height - 2; yIdx += 2)
			{
				// first two pixel values
				// Bayer         0 1 2
				//        -1     b g b
				//         0     G r g
				// line_step     b g b
				// line_step2    g r g
				
				rgb_buffer[3] = rgb_buffer[0] = bayer_pixel[1]; // red pixel
				rgb_buffer[1] = bayer_pixel[0]; // green pixel
				rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]); // blue;
				
				// Bayer         0 1 2
				//        -1     b g b
				//         0     g R g
				// line_step     b g b
				// line_step2    g r g
				//rgb_pixel[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG4 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1], bayer_pixel[1 - bayer_line_step]);
				rgb_buffer[5] = AVG4 (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2], bayer_pixel[-bayer_line_step], bayer_pixel[2 - bayer_line_step]);
				
				// BGBG line
				// Bayer         0 1 2
				//         0     g r g
				// line_step     B g b
				// line_step2    g r g
				rgb_buffer[rgb_line_step + 3] = rgb_buffer[rgb_line_step ] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
				rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step + 1], bayer_pixel[bayer_line_step2]);
				rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
				
				// pixel (1, 1)  0 1 2
				//         0     g r g
				// line_step     b G b
				// line_step2    g r g
				//rgb_pixel[rgb_line_step + 3] = AVG( bayer_pixel[1] , bayer_pixel[line_step2+1] );
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
				
				rgb_buffer += 6;
				bayer_pixel += 2;
				// continue with rest of the line
				for (xIdx = 2; xIdx < width - 2; xIdx += 2, rgb_buffer += 6, bayer_pixel += 2)
				{
					// GRGR line
					// Bayer        -1 0 1 2
					//          -1   g b g b
					//           0   r G r g
					//   line_step   g b g b
					// line_step2    r g r g
					rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
					rgb_buffer[1] = bayer_pixel[0];
					rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]);
					
					// Bayer        -1 0 1 2
					//          -1   g b g b
					//          0    r g R g
					//  line_step    g b g b
					// line_step2    r g r g
					rgb_buffer[3] = bayer_pixel[1];
					rgb_buffer[4] = AVG4 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1], bayer_pixel[1 - bayer_line_step]);
					rgb_buffer[5] = AVG4 (bayer_pixel[-bayer_line_step], bayer_pixel[2 - bayer_line_step], bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
					
					// BGBG line
					// Bayer         -1 0 1 2
					//         -1     g b g b
					//          0     r g r g
					// line_step      g B g b
					// line_step2     r g r g
					rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
					rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
					rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
					
					// Bayer         -1 0 1 2
					//         -1     g b g b
					//          0     r g r g
					// line_step      g b G b
					// line_step2     r g r g
					rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
					rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
					rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
				}

				// last two pixels of the line
				// last two pixel values for first two lines
				// GRGR line
				// Bayer        -1 0 1
				//           0   r G r
				//   line_step   g b g
				// line_step2    r g r
				rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
				rgb_buffer[1] = bayer_pixel[0];
				rgb_buffer[rgb_line_step + 5] = rgb_buffer[rgb_line_step + 2] = rgb_buffer[5] = rgb_buffer[2] = bayer_pixel[bayer_line_step];
				
				// Bayer        -1 0 1
				//          0    r g R
				//  line_step    g b g
				// line_step2    r g r
				rgb_buffer[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG (bayer_pixel[0], bayer_pixel[bayer_line_step + 1]);
				//rgb_pixel[5] = bayer_pixel[line_step];
				
				// BGBG line
				// Bayer        -1 0 1
				//          0    r g r
				//  line_step    g B g
				// line_step2    r g r
				rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
				rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
				//rgb_pixel[rgb_line_step + 2] = bayer_pixel[line_step];
				
				// Bayer         -1 0 1
				//         0      r g r
				// line_step      g b G
				// line_step2     r g r
				rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				//rgb_pixel[rgb_line_step + 5] = bayer_pixel[line_step];
				
				bayer_pixel += bayer_line_step + 2;
				rgb_buffer += rgb_line_step + 6 + rgb_line_skip;
			}
			
			//last two lines
			// Bayer         0 1 2
			//        -1     b g b
			//         0     G r g
			// line_step     b g b
			
			rgb_buffer[rgb_line_step + 3] = rgb_buffer[rgb_line_step ] = rgb_buffer[3] = rgb_buffer[0] = bayer_pixel[1]; // red pixel
			rgb_buffer[1] = bayer_pixel[0]; // green pixel
			rgb_buffer[rgb_line_step + 2] = rgb_buffer[2] = bayer_pixel[bayer_line_step]; // blue;
			
			// Bayer         0 1 2
			//        -1     b g b
			//         0     g R g
			// line_step     b g b
			//rgb_pixel[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG4 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1], bayer_pixel[1 - bayer_line_step]);
			rgb_buffer[5] = AVG4 (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2], bayer_pixel[-bayer_line_step], bayer_pixel[2 - bayer_line_step]);
			
			// BGBG line
			// Bayer         0 1 2
			//        -1     b g b
			//         0     g r g
			// line_step     B g b
			//rgb_pixel[rgb_line_step    ] = bayer_pixel[1];
			rgb_buffer[rgb_line_step + 1] = AVG (bayer_pixel[0], bayer_pixel[bayer_line_step + 1]);
			rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
			
			// Bayer         0 1 2
			//        -1     b g b
			//         0     g r g
			// line_step     b G b
			//rgb_pixel[rgb_line_step + 3] = AVG( bayer_pixel[1] , bayer_pixel[line_step2+1] );
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
			
			rgb_buffer += 6;
			bayer_pixel += 2;
			// rest of the last two lines
			for (xIdx = 2; xIdx < width - 2; xIdx += 2, rgb_buffer += 6, bayer_pixel += 2)
			{
				rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
				rgb_buffer[1] = bayer_pixel[0];
				rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]);

				rgb_buffer[rgb_line_step + 3] = rgb_buffer[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG4 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1], bayer_pixel[1 - bayer_line_step]);
				rgb_buffer[5] = AVG4 (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2], bayer_pixel[-bayer_line_step], bayer_pixel[-bayer_line_step + 2]);

				rgb_buffer[rgb_line_step ] = AVG (bayer_pixel[-1], bayer_pixel[1]);
				rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
				rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
				
				// Bayer       -1 0 1 2
				//        -1    g b g b
				//         0    r g r g
				// line_step    g b G b
				//rgb_pixel[rgb_line_step + 3] = bayer_pixel[1];
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
			}
			
			// last two pixel values for first two lines
			// GRGR line
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r G r
			// line_step    g b g
			rgb_buffer[rgb_line_step ] = rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
			rgb_buffer[1] = bayer_pixel[0];
			rgb_buffer[5] = rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]);
			
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r g R
			// line_step    g b g
			rgb_buffer[rgb_line_step + 3] = rgb_buffer[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step + 1], bayer_pixel[-bayer_line_step + 1]);
			//rgb_pixel[5] = AVG( bayer_pixel[line_step], bayer_pixel[-line_step] );
			
			// BGBG line
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r g r
			// line_step    g B g
			//rgb_pixel[rgb_line_step    ] = AVG2( bayer_pixel[-1], bayer_pixel[1] );
			rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
			rgb_buffer[rgb_line_step + 5] = rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
			
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r g r
			// line_step    g b G
			//rgb_pixel[rgb_line_step + 3] = bayer_pixel[1];
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			//rgb_pixel[rgb_line_step + 5] = bayer_pixel[line_step];
		}
		else if (debayering_method == EdgeAware)
		{
			int dh, dv;
			
			// first two pixel values for first two lines
			// Bayer         0 1 2
			//         0     G r g
			// line_step     b g b
			// line_step2    g r g
			
			rgb_buffer[3] = rgb_buffer[0] = bayer_pixel[1]; // red pixel
			rgb_buffer[1] = bayer_pixel[0]; // green pixel
			rgb_buffer[rgb_line_step + 2] = rgb_buffer[2] = bayer_pixel[bayer_line_step]; // blue;
			
			// Bayer         0 1 2
			//         0     g R g
			// line_step     b g b
			// line_step2    g r g
			//rgb_pixel[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG3 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1]);
			rgb_buffer[rgb_line_step + 5] = rgb_buffer[5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
			
			// BGBG line
			// Bayer         0 1 2
			//         0     g r g
			// line_step     B g b
			// line_step2    g r g
			rgb_buffer[rgb_line_step + 3] = rgb_buffer[rgb_line_step ] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
			rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step + 1], bayer_pixel[bayer_line_step2]);
			//rgb_pixel[rgb_line_step + 2] = bayer_pixel[line_step];
			
			// pixel (1, 1)  0 1 2
			//         0     g r g
			// line_step     b G b
			// line_step2    g r g
			//rgb_pixel[rgb_line_step + 3] = AVG( bayer_pixel[1] , bayer_pixel[line_step2+1] );
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			//rgb_pixel[rgb_line_step + 5] = AVG( bayer_pixel[line_step] , bayer_pixel[line_step+2] );
			
			rgb_buffer += 6;
			bayer_pixel += 2;
			// rest of the first two lines
			for (xIdx = 2; xIdx < width - 2; xIdx += 2, rgb_buffer += 6, bayer_pixel += 2)
			{
				// GRGR line
				// Bayer        -1 0 1 2
				//           0   r G r g
				//   line_step   g b g b
				// line_step2    r g r g
				rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
				rgb_buffer[1] = bayer_pixel[0];
				rgb_buffer[2] = bayer_pixel[bayer_line_step + 1];
				
				// Bayer        -1 0 1 2
				//          0    r g R g
				//  line_step    g b g b
				// line_step2    r g r g
				rgb_buffer[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG3 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1]);
				rgb_buffer[rgb_line_step + 5] = rgb_buffer[5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
				
				// BGBG line
				// Bayer         -1 0 1 2
				//         0      r g r g
				// line_step      g B g b
				// line_step2     r g r g
				rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
				rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
				rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
				
				// Bayer         -1 0 1 2
				//         0      r g r g
				// line_step      g b G b
				// line_step2     r g r g
				rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				//rgb_pixel[rgb_line_step + 5] = AVG( bayer_pixel[line_step] , bayer_pixel[line_step+2] );
			}
			
			// last two pixel values for first two lines
			// GRGR line
			// Bayer        -1 0 1
			//           0   r G r
			//   line_step   g b g
			// line_step2    r g r
			rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
			rgb_buffer[1] = bayer_pixel[0];
			rgb_buffer[rgb_line_step + 5] = rgb_buffer[rgb_line_step + 2] = rgb_buffer[5] = rgb_buffer[2] = bayer_pixel[bayer_line_step];
			
			// Bayer        -1 0 1
			//          0    r g R
			//  line_step    g b g
			// line_step2    r g r
			rgb_buffer[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG (bayer_pixel[0], bayer_pixel[bayer_line_step + 1]);
			//rgb_pixel[5] = bayer_pixel[line_step];
			
			// BGBG line
			// Bayer        -1 0 1
			//          0    r g r
			//  line_step    g B g
			// line_step2    r g r
			rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
			rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
			//rgb_pixel[rgb_line_step + 2] = bayer_pixel[line_step];
			
			// Bayer         -1 0 1
			//         0      r g r
			// line_step      g b G
			// line_step2     r g r
			rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			//rgb_pixel[rgb_line_step + 5] = bayer_pixel[line_step];
			
			bayer_pixel += bayer_line_step + 2;
			rgb_buffer += rgb_line_step + 6 + rgb_line_skip;
			// main processing
			for (yIdx = 2; yIdx < height - 2; yIdx += 2)
			{
				// first two pixel values
				// Bayer         0 1 2
				//        -1     b g b
				//         0     G r g
				// line_step     b g b
				// line_step2    g r g
				
				rgb_buffer[3] = rgb_buffer[0] = bayer_pixel[1]; // red pixel
				rgb_buffer[1] = bayer_pixel[0]; // green pixel
				rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]); // blue;
				
				// Bayer         0 1 2
				//        -1     b g b
				//         0     g R g
				// line_step     b g b
				// line_step2    g r g
				//rgb_pixel[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG4 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1], bayer_pixel[1 - bayer_line_step]);
				rgb_buffer[5] = AVG4 (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2], bayer_pixel[-bayer_line_step], bayer_pixel[2 - bayer_line_step]);
				
				// BGBG line
				// Bayer         0 1 2
				//         0     g r g
				// line_step     B g b
				// line_step2    g r g
				rgb_buffer[rgb_line_step + 3] = rgb_buffer[rgb_line_step ] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
				rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step + 1], bayer_pixel[bayer_line_step2]);
				rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
				
				// pixel (1, 1)  0 1 2
				//         0     g r g
				// line_step     b G b
				// line_step2    g r g
				//rgb_pixel[rgb_line_step + 3] = AVG( bayer_pixel[1] , bayer_pixel[line_step2+1] );
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
				
				rgb_buffer += 6;
				bayer_pixel += 2;
				// continue with rest of the line
				for (xIdx = 2; xIdx < width - 2; xIdx += 2, rgb_buffer += 6, bayer_pixel += 2)
				{
					// GRGR line
					// Bayer        -1 0 1 2
					//          -1   g b g b
					//           0   r G r g
					//   line_step   g b g b
					// line_step2    r g r g
					rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
					rgb_buffer[1] = bayer_pixel[0];
					rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]);
					
					// Bayer        -1 0 1 2
					//          -1   g b g b
					//          0    r g R g
					//  line_step    g b g b
					// line_step2    r g r g
					
					dh = abs (bayer_pixel[0] - bayer_pixel[2]);
					dv = abs (bayer_pixel[-bayer_line_step + 1] - bayer_pixel[bayer_line_step + 1]);
					
					if (dh > dv)
						rgb_buffer[4] = AVG (bayer_pixel[-bayer_line_step + 1], bayer_pixel[bayer_line_step + 1]);
					else if (dv > dh)
						rgb_buffer[4] = AVG (bayer_pixel[0], bayer_pixel[2]);
					else
						rgb_buffer[4] = AVG4 (bayer_pixel[-bayer_line_step + 1], bayer_pixel[bayer_line_step + 1], bayer_pixel[0], bayer_pixel[2]);
					
					rgb_buffer[3] = bayer_pixel[1];
					rgb_buffer[5] = AVG4 (bayer_pixel[-bayer_line_step], bayer_pixel[2 - bayer_line_step], bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
					
					// BGBG line
					// Bayer         -1 0 1 2
					//         -1     g b g b
					//          0     r g r g
					// line_step      g B g b
					// line_step2     r g r g
					rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
					rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
					
					dv = abs (bayer_pixel[0] - bayer_pixel[bayer_line_step2]);
					dh = abs (bayer_pixel[bayer_line_step - 1] - bayer_pixel[bayer_line_step + 1]);
					
					if (dv > dh)
						rgb_buffer[rgb_line_step + 1] = AVG (bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
					else if (dh > dv)
						rgb_buffer[rgb_line_step + 1] = AVG (bayer_pixel[0], bayer_pixel[bayer_line_step2]);
					else
						rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
					
					// Bayer         -1 0 1 2
					//         -1     g b g b
					//          0     r g r g
					// line_step      g b G b
					// line_step2     r g r g
					rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
					rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
					rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
				}
				
				// last two pixels of the line
				// last two pixel values for first two lines
				// GRGR line
				// Bayer        -1 0 1
				//           0   r G r
				//   line_step   g b g
				// line_step2    r g r
				rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
				rgb_buffer[1] = bayer_pixel[0];
				rgb_buffer[rgb_line_step + 5] = rgb_buffer[rgb_line_step + 2] = rgb_buffer[5] = rgb_buffer[2] = bayer_pixel[bayer_line_step];
				
				// Bayer        -1 0 1
				//          0    r g R
				//  line_step    g b g
				// line_step2    r g r
				rgb_buffer[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG (bayer_pixel[0], bayer_pixel[bayer_line_step + 1]);
				//rgb_pixel[5] = bayer_pixel[line_step];
				
				// BGBG line
				// Bayer        -1 0 1
				//          0    r g r
				//  line_step    g B g
				// line_step2    r g r
				rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
				rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
				//rgb_pixel[rgb_line_step + 2] = bayer_pixel[line_step];
				
				// Bayer         -1 0 1
				//         0      r g r
				// line_step      g b G
				// line_step2     r g r
				rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				//rgb_pixel[rgb_line_step + 5] = bayer_pixel[line_step];
				
				bayer_pixel += bayer_line_step + 2;
				rgb_buffer += rgb_line_step + 6 + rgb_line_skip;
			}
			
			//last two lines
			// Bayer         0 1 2
			//        -1     b g b
			//         0     G r g
			// line_step     b g b
			
			rgb_buffer[rgb_line_step + 3] = rgb_buffer[rgb_line_step ] = rgb_buffer[3] = rgb_buffer[0] = bayer_pixel[1]; // red pixel
			rgb_buffer[1] = bayer_pixel[0]; // green pixel
			rgb_buffer[rgb_line_step + 2] = rgb_buffer[2] = bayer_pixel[bayer_line_step]; // blue;
			
			// Bayer         0 1 2
			//        -1     b g b
			//         0     g R g
			// line_step     b g b
			//rgb_pixel[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG4 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1], bayer_pixel[1 - bayer_line_step]);
			rgb_buffer[5] = AVG4 (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2], bayer_pixel[-bayer_line_step], bayer_pixel[2 - bayer_line_step]);
			
			// BGBG line
			// Bayer         0 1 2
			//        -1     b g b
			//         0     g r g
			// line_step     B g b
			//rgb_pixel[rgb_line_step    ] = bayer_pixel[1];
			rgb_buffer[rgb_line_step + 1] = AVG (bayer_pixel[0], bayer_pixel[bayer_line_step + 1]);
			rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
			
			// Bayer         0 1 2
			//        -1     b g b
			//         0     g r g
			// line_step     b G b
			//rgb_pixel[rgb_line_step + 3] = AVG( bayer_pixel[1] , bayer_pixel[line_step2+1] );
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
			
			rgb_buffer += 6;
			bayer_pixel += 2;
			// rest of the last two lines
			for (xIdx = 2; xIdx < width - 2; xIdx += 2, rgb_buffer += 6, bayer_pixel += 2)
			{
				// GRGR line
				// Bayer       -1 0 1 2
				//        -1    g b g b
				//         0    r G r g
				// line_step    g b g b
				rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
				rgb_buffer[1] = bayer_pixel[0];
				rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]);
				
				// Bayer       -1 0 1 2
				//        -1    g b g b
				//         0    r g R g
				// line_step    g b g b
				rgb_buffer[rgb_line_step + 3] = rgb_buffer[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG4 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1], bayer_pixel[1 - bayer_line_step]);
				rgb_buffer[5] = AVG4 (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2], bayer_pixel[-bayer_line_step], bayer_pixel[-bayer_line_step + 2]);
				
				// BGBG line
				// Bayer       -1 0 1 2
				//        -1    g b g b
				//         0    r g r g
				// line_step    g B g b
				rgb_buffer[rgb_line_step ] = AVG (bayer_pixel[-1], bayer_pixel[1]);
				rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
				rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
				
				
				// Bayer       -1 0 1 2
				//        -1    g b g b
				//         0    r g r g
				// line_step    g b G b
				//rgb_pixel[rgb_line_step + 3] = bayer_pixel[1];
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
			}
			
			// last two pixel values for first two lines
			// GRGR line
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r G r
			// line_step    g b g
			rgb_buffer[rgb_line_step ] = rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
			rgb_buffer[1] = bayer_pixel[0];
			rgb_buffer[5] = rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]);
			
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r g R
			// line_step    g b g
			rgb_buffer[rgb_line_step + 3] = rgb_buffer[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step + 1], bayer_pixel[-bayer_line_step + 1]);
			//rgb_pixel[5] = AVG( bayer_pixel[line_step], bayer_pixel[-line_step] );
			
			// BGBG line
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r g r
			// line_step    g B g
			//rgb_pixel[rgb_line_step    ] = AVG2( bayer_pixel[-1], bayer_pixel[1] );
			rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
			rgb_buffer[rgb_line_step + 5] = rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
			
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r g r
			// line_step    g b G
			//rgb_pixel[rgb_line_step + 3] = bayer_pixel[1];
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			//rgb_pixel[rgb_line_step + 5] = bayer_pixel[line_step];
		}
		else if (debayering_method == EdgeAwareWeighted)
		{
			int dh, dv;
			
			// first two pixel values for first two lines
			// Bayer         0 1 2
			//         0     G r g
			// line_step     b g b
			// line_step2    g r g
			
			rgb_buffer[3] = rgb_buffer[0] = bayer_pixel[1]; // red pixel
			rgb_buffer[1] = bayer_pixel[0]; // green pixel
			rgb_buffer[rgb_line_step + 2] = rgb_buffer[2] = bayer_pixel[bayer_line_step]; // blue;
			
			// Bayer         0 1 2
			//         0     g R g
			// line_step     b g b
			// line_step2    g r g
			//rgb_pixel[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG3 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1]);
			rgb_buffer[rgb_line_step + 5] = rgb_buffer[5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
			
			// BGBG line
			// Bayer         0 1 2
			//         0     g r g
			// line_step     B g b
			// line_step2    g r g
			rgb_buffer[rgb_line_step + 3] = rgb_buffer[rgb_line_step ] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
			rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step + 1], bayer_pixel[bayer_line_step2]);
			//rgb_pixel[rgb_line_step + 2] = bayer_pixel[line_step];
			
			// pixel (1, 1)  0 1 2
			//         0     g r g
			// line_step     b G b
			// line_step2    g r g
			//rgb_pixel[rgb_line_step + 3] = AVG( bayer_pixel[1] , bayer_pixel[line_step2+1] );
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			//rgb_pixel[rgb_line_step + 5] = AVG( bayer_pixel[line_step] , bayer_pixel[line_step+2] );
			
			rgb_buffer += 6;
			bayer_pixel += 2;
			// rest of the first two lines
			for (xIdx = 2; xIdx < width - 2; xIdx += 2, rgb_buffer += 6, bayer_pixel += 2)
			{
				// GRGR line
				// Bayer        -1 0 1 2
				//           0   r G r g
				//   line_step   g b g b
				// line_step2    r g r g
				rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
				rgb_buffer[1] = bayer_pixel[0];
				rgb_buffer[2] = bayer_pixel[bayer_line_step + 1];
				
				// Bayer        -1 0 1 2
				//          0    r g R g
				//  line_step    g b g b
				// line_step2    r g r g
				rgb_buffer[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG3 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1]);
				rgb_buffer[rgb_line_step + 5] = rgb_buffer[5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
				
				// BGBG line
				// Bayer         -1 0 1 2
				//         0      r g r g
				// line_step      g B g b
				// line_step2     r g r g
				rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
				rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
				rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
				
				// Bayer         -1 0 1 2
				//         0      r g r g
				// line_step      g b G b
				// line_step2     r g r g
				rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				//rgb_pixel[rgb_line_step + 5] = AVG( bayer_pixel[line_step] , bayer_pixel[line_step+2] );
			}
			
			// last two pixel values for first two lines
			// GRGR line
			// Bayer        -1 0 1
			//           0   r G r
			//   line_step   g b g
			// line_step2    r g r
			rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
			rgb_buffer[1] = bayer_pixel[0];
			rgb_buffer[rgb_line_step + 5] = rgb_buffer[rgb_line_step + 2] = rgb_buffer[5] = rgb_buffer[2] = bayer_pixel[bayer_line_step];
			
			// Bayer        -1 0 1
			//          0    r g R
			//  line_step    g b g
			// line_step2    r g r
			rgb_buffer[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG (bayer_pixel[0], bayer_pixel[bayer_line_step + 1]);
			//rgb_pixel[5] = bayer_pixel[line_step];
			
			// BGBG line
			// Bayer        -1 0 1
			//          0    r g r
			//  line_step    g B g
			// line_step2    r g r
			rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
			rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
			//rgb_pixel[rgb_line_step + 2] = bayer_pixel[line_step];
			
			// Bayer         -1 0 1
			//         0      r g r
			// line_step      g b G
			// line_step2     r g r
			rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			//rgb_pixel[rgb_line_step + 5] = bayer_pixel[line_step];
			
			bayer_pixel += bayer_line_step + 2;
			rgb_buffer += rgb_line_step + 6 + rgb_line_skip;
			// main processing
			for (yIdx = 2; yIdx < height - 2; yIdx += 2)
			{
				// first two pixel values
				// Bayer         0 1 2
				//        -1     b g b
				//         0     G r g
				// line_step     b g b
				// line_step2    g r g
				
				rgb_buffer[3] = rgb_buffer[0] = bayer_pixel[1]; // red pixel
				rgb_buffer[1] = bayer_pixel[0]; // green pixel
				rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]); // blue;
				
				// Bayer         0 1 2
				//        -1     b g b
				//         0     g R g
				// line_step     b g b
				// line_step2    g r g
				//rgb_pixel[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG4 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1], bayer_pixel[1 - bayer_line_step]);
				rgb_buffer[5] = AVG4 (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2], bayer_pixel[-bayer_line_step], bayer_pixel[2 - bayer_line_step]);
				
				// BGBG line
				// Bayer         0 1 2
				//         0     g r g
				// line_step     B g b
				// line_step2    g r g
				rgb_buffer[rgb_line_step + 3] = rgb_buffer[rgb_line_step ] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
				rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step + 1], bayer_pixel[bayer_line_step2]);
				rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
				
				// pixel (1, 1)  0 1 2
				//         0     g r g
				// line_step     b G b
				// line_step2    g r g
				//rgb_pixel[rgb_line_step + 3] = AVG( bayer_pixel[1] , bayer_pixel[line_step2+1] );
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
				
				rgb_buffer += 6;
				bayer_pixel += 2;
				// continue with rest of the line
				for (xIdx = 2; xIdx < width - 2; xIdx += 2, rgb_buffer += 6, bayer_pixel += 2)
				{
					// GRGR line
					// Bayer        -1 0 1 2
					//          -1   g b g b
					//           0   r G r g
					//   line_step   g b g b
					// line_step2    r g r g
					rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
					rgb_buffer[1] = bayer_pixel[0];
					rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]);
					
					// Bayer        -1 0 1 2
					//          -1   g b g b
					//          0    r g R g
					//  line_step    g b g b
					// line_step2    r g r g
					
					dh = abs (bayer_pixel[0] - bayer_pixel[2]);
					dv = abs (bayer_pixel[-bayer_line_step + 1] - bayer_pixel[bayer_line_step + 1]);
					
					if (dv == 0 && dh == 0)
						rgb_buffer[4] = AVG4 (bayer_pixel[1 - bayer_line_step], bayer_pixel[1 + bayer_line_step], bayer_pixel[0], bayer_pixel[2]);
					else
						rgb_buffer[4] = WAVG4 (bayer_pixel[1 - bayer_line_step], bayer_pixel[1 + bayer_line_step], bayer_pixel[0], bayer_pixel[2], dh, dv);
					rgb_buffer[3] = bayer_pixel[1];
					rgb_buffer[5] = AVG4 (bayer_pixel[-bayer_line_step], bayer_pixel[2 - bayer_line_step], bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
					
					// BGBG line
					// Bayer         -1 0 1 2
					//         -1     g b g b
					//          0     r g r g
					// line_step      g B g b
					// line_step2     r g r g
					rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
					rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
					
					dv = abs (bayer_pixel[0] - bayer_pixel[bayer_line_step2]);
					dh = abs (bayer_pixel[bayer_line_step - 1] - bayer_pixel[bayer_line_step + 1]);
					
					if (dv == 0 && dh == 0)
						rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
					else
						rgb_buffer[rgb_line_step + 1] = WAVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1], dh, dv);
					
					// Bayer         -1 0 1 2
					//         -1     g b g b
					//          0     r g r g
					// line_step      g b G b
					// line_step2     r g r g
					rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
					rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
					rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
				}
				
				// last two pixels of the line
				// last two pixel values for first two lines
				// GRGR line
				// Bayer        -1 0 1
				//           0   r G r
				//   line_step   g b g
				// line_step2    r g r
				rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
				rgb_buffer[1] = bayer_pixel[0];
				rgb_buffer[rgb_line_step + 5] = rgb_buffer[rgb_line_step + 2] = rgb_buffer[5] = rgb_buffer[2] = bayer_pixel[bayer_line_step];
				
				// Bayer        -1 0 1
				//          0    r g R
				//  line_step    g b g
				// line_step2    r g r
				rgb_buffer[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG (bayer_pixel[0], bayer_pixel[bayer_line_step + 1]);
				//rgb_pixel[5] = bayer_pixel[line_step];
				
				// BGBG line
				// Bayer        -1 0 1
				//          0    r g r
				//  line_step    g B g
				// line_step2    r g r
				rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
				rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
				//rgb_pixel[rgb_line_step + 2] = bayer_pixel[line_step];
				
				// Bayer         -1 0 1
				//         0      r g r
				// line_step      g b G
				// line_step2     r g r
				rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				//rgb_pixel[rgb_line_step + 5] = bayer_pixel[line_step];
				
				bayer_pixel += bayer_line_step + 2;
				rgb_buffer += rgb_line_step + 6 + rgb_line_skip;
			}
			
			//last two lines
			// Bayer         0 1 2
			//        -1     b g b
			//         0     G r g
			// line_step     b g b
			
			rgb_buffer[rgb_line_step + 3] = rgb_buffer[rgb_line_step ] = rgb_buffer[3] = rgb_buffer[0] = bayer_pixel[1]; // red pixel
			rgb_buffer[1] = bayer_pixel[0]; // green pixel
			rgb_buffer[rgb_line_step + 2] = rgb_buffer[2] = bayer_pixel[bayer_line_step]; // blue;
			
			// Bayer         0 1 2
			//        -1     b g b
			//         0     g R g
			// line_step     b g b
			//rgb_pixel[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG4 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1], bayer_pixel[1 - bayer_line_step]);
			rgb_buffer[5] = AVG4 (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2], bayer_pixel[-bayer_line_step], bayer_pixel[2 - bayer_line_step]);
			
			// BGBG line
			// Bayer         0 1 2
			//        -1     b g b
			//         0     g r g
			// line_step     B g b
			//rgb_pixel[rgb_line_step    ] = bayer_pixel[1];
			rgb_buffer[rgb_line_step + 1] = AVG (bayer_pixel[0], bayer_pixel[bayer_line_step + 1]);
			rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
			
			// Bayer         0 1 2
			//        -1     b g b
			//         0     g r g
			// line_step     b G b
			//rgb_pixel[rgb_line_step + 3] = AVG( bayer_pixel[1] , bayer_pixel[line_step2+1] );
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
			
			rgb_buffer += 6;
			bayer_pixel += 2;
			// rest of the last two lines
			for (xIdx = 2; xIdx < width - 2; xIdx += 2, rgb_buffer += 6, bayer_pixel += 2)
			{
				// GRGR line
				// Bayer       -1 0 1 2
				//        -1    g b g b
				//         0    r G r g
				// line_step    g b g b
				rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
				rgb_buffer[1] = bayer_pixel[0];
				rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]);
				
				// Bayer       -1 0 1 2
				//        -1    g b g b
				//         0    r g R g
				// line_step    g b g b
				rgb_buffer[rgb_line_step + 3] = rgb_buffer[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG4 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1], bayer_pixel[1 - bayer_line_step]);
				rgb_buffer[5] = AVG4 (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2], bayer_pixel[-bayer_line_step], bayer_pixel[-bayer_line_step + 2]);
				
				// BGBG line
				// Bayer       -1 0 1 2
				//        -1    g b g b
				//         0    r g r g
				// line_step    g B g b
				rgb_buffer[rgb_line_step ] = AVG (bayer_pixel[-1], bayer_pixel[1]);
				rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
				rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
				
				
				// Bayer       -1 0 1 2
				//        -1    g b g b
				//         0    r g r g
				// line_step    g b G b
				//rgb_pixel[rgb_line_step + 3] = bayer_pixel[1];
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
			}
			
			// last two pixel values for first two lines
			// GRGR line
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r G r
			// line_step    g b g
			rgb_buffer[rgb_line_step ] = rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
			rgb_buffer[1] = bayer_pixel[0];
			rgb_buffer[5] = rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]);
			
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r g R
			// line_step    g b g
			rgb_buffer[rgb_line_step + 3] = rgb_buffer[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step + 1], bayer_pixel[-bayer_line_step + 1]);
			//rgb_pixel[5] = AVG( bayer_pixel[line_step], bayer_pixel[-line_step] );
			
			// BGBG line
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r g r
			// line_step    g B g
			//rgb_pixel[rgb_line_step    ] = AVG2( bayer_pixel[-1], bayer_pixel[1] );
			rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
			rgb_buffer[rgb_line_step + 5] = rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
			
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r g r
			// line_step    g b G
			//rgb_pixel[rgb_line_step + 3] = bayer_pixel[1];
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			//rgb_pixel[rgb_line_step + 5] = bayer_pixel[line_step];
		}
		//else
		//	THROW_OPENNI_EXCEPTION ("Unknwon debayering method: %d", (int)debayering_method);
	}
	//Warning: Downsampling mod is untested
	else if (nDownSampleStep > 1)
	{		
		// get each or each 2nd pixel group to find rgb values!
		register unsigned bayerXStep = nDownSampleStep;
		register unsigned bayerYSkip = (nDownSampleStep - 1) * width;
		
		// Downsampling and debayering at once
		register const XnUInt8* bayer_buffer = bayer_pixel;
		
		for (register unsigned yIdx = 0; yIdx < height; ++yIdx, bayer_buffer += bayerYSkip, rgb_buffer += rgb_line_skip) // skip a line
		{
			for (register unsigned xIdx = 0; xIdx < width; ++xIdx, rgb_buffer += 3, bayer_buffer += bayerXStep)
			{
				rgb_buffer[ 2 ] = bayer_buffer[ width ];
				rgb_buffer[ 1 ] = AVG (bayer_buffer[0], bayer_buffer[ width + 1]);
				rgb_buffer[ 0 ] = bayer_buffer[ 1 ];
			}
		}
	}
}

void Bayer2RGB888(const XnUInt8* pBayerImage, XnUInt8* pRGBImage, XnUInt32 nXRes, XnUInt32 nYRes, XnUInt32 nDownSampleStep)
{	
	fillRGB(nXRes, nYRes, pBayerImage, pRGBImage, DebayeringMethod(1), nDownSampleStep); // DebayeringMethod(0) == bilinear, (1) == edge aware, (2) == edge aware weighted
}



