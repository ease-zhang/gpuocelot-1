/*
 * Copyright 1993-2006 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO USER:   
 *
 * This source code is subject to NVIDIA ownership rights under U.S. and 
 * international Copyright laws.  
 *
 * NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE 
 * CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR 
 * IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH 
 * REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.   
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL, 
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS 
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE 
 * OR PERFORMANCE OF THIS SOURCE CODE.  
 *
 * U.S. Government End Users.  This source code is a "commercial item" as 
 * that term is defined at 48 C.F.R. 2.101 (OCT 1995), consisting  of 
 * "commercial computer software" and "commercial computer software 
 * documentation" as such terms are used in 48 C.F.R. 12.212 (SEPT 1995) 
 * and is provided to the U.S. Government only as a commercial end item.  
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through 
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the 
 * source code with only those rights set forth herein.
 */

#ifndef __BODYSYSTEMCUDA_H__
#define __BODYSYSTEMCUDA_H__

#include "bodysystem.h"

// CUDA BodySystem: runs on the GPU
class BodySystemCUDA : public BodySystem
{
public:
    BodySystemCUDA(int numBodies, bool usePBO);
    BodySystemCUDA(int numBodies, unsigned int p, unsigned int q, bool usePBO);
    virtual ~BodySystemCUDA();

    virtual void update(float deltaTime);

    virtual void setSoftening(float softening);
    virtual void setDamping(float damping);

    virtual float* getArray(BodyArray array);
    virtual void   setArray(BodyArray array, const float* data);

    virtual unsigned int getCurrentReadBuffer() const 
    { 
        return m_pbo[m_currentRead]; 
    }

    virtual void synchronizeThreads() const;

protected: // methods
    BodySystemCUDA() {}

    virtual void _initialize(int numBodies);
    virtual void _finalize();
    
protected: // data
    // CPU data
    float* m_hPos;
    float* m_hVel;

    // GPU data
    float* m_dPos[2];
    float* m_dVel[2];

    bool m_bUsePBO;

    float m_damping;

    unsigned int m_pbo[2];
    unsigned int m_currentRead;
    unsigned int m_currentWrite;

    unsigned int m_p;
    unsigned int m_q;
};

#endif // __BODYSYSTEMCUDA_H__
