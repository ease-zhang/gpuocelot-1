/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */
 
#ifndef _CUTIL_INLINE_BANKCHECKER_H_
#define _CUTIL_INLINE_BANKCHECKER_H_

#define cutilBankChecker(array, idx) array[idx]

    // Interface for bank conflict checker
inline void __cutilBankChecker(unsigned int tidx, unsigned int tidy, unsigned int tidz,
                            unsigned int bdimx, unsigned int bdimy, unsigned int bdimz,
                            char *aname, int index, char *file, int line) 
{
    cutCheckBankAccess( tidx, tidy, tidz, bdimx, bdimy, bdimz, file, line, aname, index);
}

#endif // _CUTIL_INLINE_BANKCHECKER_H_
