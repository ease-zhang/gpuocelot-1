/*
 * Copyright 1993-2007 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO USER:
 *
 * This source code is subject to NVIDIA ownership rights under U.S. and
 * international Copyright laws.  Users and possessors of this source code
 * are hereby granted a nonexclusive, royalty-free license to use this code
 * in individual and commercial software.
 *
 * NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE
 * CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR
 * IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS,  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION,  ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOURCE CODE.
 *
 * U.S. Government End Users.   This source code is a "commercial item" as
 * that term is defined at  48 C.F.R. 2.101 (OCT 1995), consisting  of
 * "commercial computer  software"  and "commercial computer software
 * documentation" as such terms are  used in 48 C.F.R. 12.212 (SEPT 1995)
 * and is provided to the U.S. Government only as a commercial end item.
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the
 * source code with only those rights set forth herein.
 *
 * Any use of this source code in individual and commercial software must
 * include, in the user documentation and internal comments to the code,
 * the above Disclaimer and U.S. Government End Users Notice.
 */

#ifndef PERMUTATIONS_H
#define PERMUTATIONS_H

#include <cutil_inline.h> // cutilCondition


static void computePermutations(uint permutations[1024])
{
	int indices[16];
	int num = 0;

	// 3 element permutations:

	// first cluster [0,i) is at the start
	for( int m = 0; m < 16; ++m )
	{
		indices[m] = 0;
	}
	const int imax = 15;
	for( int i = imax; i >= 0; --i )
	{
		// second cluster [i,j) is half along
		for( int m = i; m < 16; ++m )
		{
			indices[m] = 2;
		}
		const int jmax = ( i == 0 ) ? 15 : 16;
		for( int j = jmax; j >= i; --j )
		{
			// last cluster [j,k) is at the end
			if( j < 16 )
			{
				indices[j] = 1;
			}

			uint permutation = 0;
			
			for(int p = 0; p < 16; p++) {
				permutation |= indices[p] << (p * 2);
				//permutation |= indices[15-p] << (p * 2);
			}
				
			permutations[num] = permutation;
				
			num++;
		}
	}
	cutilCondition(num == 151);

	for(int i = 0; i < 9; i++)
	{
		permutations[num] = 0x000AA555;
		num++;
	}
	cutilCondition(num == 160);

	// Append 4 element permutations:

	// first cluster [0,i) is at the start
	for( int m = 0; m < 16; ++m )
	{
		indices[m] = 0;
	}
	
	for( int i = imax; i >= 0; --i )
	{
		// second cluster [i,j) is one third along
		for( int m = i; m < 16; ++m )
		{
			indices[m] = 2;
		}
		const int jmax = ( i == 0 ) ? 15 : 16;
		for( int j = jmax; j >= i; --j )
		{
			// third cluster [j,k) is two thirds along
			for( int m = j; m < 16; ++m )
			{
				indices[m] = 3;
			}

			int kmax = ( j == 0 ) ? 15 : 16;
			for( int k = kmax; k >= j; --k )
			{
				// last cluster [k,n) is at the end
				if( k < 16 )
				{
					indices[k] = 1;
				}
				
				uint permutation = 0;

				bool hasThree = false;
				for(int p = 0; p < 16; p++) {
					permutation |= indices[p] << (p * 2);
					//permutation |= indices[15-p] << (p * 2);

					if (indices[p] == 3) hasThree = true;
				}
				
				if (hasThree) {
					permutations[num] = permutation;
					num++;
				}
			}
		}
	}
	cutilCondition(num == 975);
	
	// 1024 - 969 - 7 = 48 extra elements
	
	// It would be nice to set these extra elements with better values...
	for(int i = 0; i < 49; i++)
	{
		permutations[num] = 0x00AAFF55;
		num++;
	}

	cutilCondition(num == 1024);
}


#endif // PERMUTATIONS_H
