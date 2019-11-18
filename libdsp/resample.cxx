/*
Copyright (c) 2019, Ning Wang <nwang.cooper@gmail.com> All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:
    
     Redistributions of source code must retain the above copyright notice, 
     this list of conditions and the following disclaimer.

     Redistributions in binary form must reproduce the above copyright notice, 
     this list of conditions and the following disclaimer in the 
     documentation and/or other materials provided with the distribution.
 
     Neither the name of its contributors can be used to endorse or promote 
     products derived from this software witthout specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS 
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <stdlib.h>
#include <string.h>
#include "resample.h"
#include <math.h>
#include <assert.h>

resample::resample(float *taps, int n_taps, int upsample, int blksize)
    : m_n_phase(upsample), m_blksize(blksize)
    , m_pos(0), m_mu(0.0f)
{

    
    m_phase_len = (n_taps + m_n_phase - 1) / m_n_phase;
    m_phase_taps = new float*[m_n_phase];
    m_out = new float*[m_n_phase];
        
    for (int i = 0; i<m_n_phase; i++){
        m_phase_taps[i] = new float[m_phase_len];
        m_out[i] = new float[m_blksize];
    }
    
    m_history = new float[m_blksize];


    for (int i=0; i<m_phase_len; i++){
        for (int j=0; j<m_n_phase; j++){
            int n = i*m_n_phase + j;
            if (n < n_taps){
                m_phase_taps[j][i] = taps[n];
            }else{
                m_phase_taps[j][i] = 0.0f;
            }
        }
    }

    for (int i=0; i<m_blksize; i++){
            m_history[i]= 0.0f;
    }
}


resample::~resample()
{
    for (int i=0; i<m_n_phase; i++){
        delete[] m_phase_taps[i];
        delete[] m_out[i];
    }
    delete[] m_history;
    delete[] m_phase_taps;
    delete[] m_out;

}


int resample::process(float* inout, int n_in, float rate)
{
    //fitler for each polyphase and store them;
    const float *input = inout;
    float *output = inout;
    int n_out = 0;

    assert(n_in <= m_blksize);
    
    for (int i=0; i<n_in; i++){

        for(int j=0; j<m_n_phase; j++){
            float accu = m_phase_taps[j][0] * input[i];
            for(int n=1; n<m_phase_len; n++){
                accu += m_phase_taps[j][n] * m_history[n-1];
            }
            m_out[j][i] = accu;
        }

        for (int n=m_phase_len-2 ; n>0; n--){
            m_history[n] = m_history[n-1];
        }
        m_history[0] = input[i];
    }

    //get the output
    for (int i=0; i<n_in; i++)
    {
        float t = m_pos + m_mu + rate * m_n_phase;  /* t is based on the up sampled rate*/
        int phase0, phase1, n;
        
        m_pos = (int)floorf(t);
        m_mu = t - m_pos;

        phase0 = m_pos % m_n_phase;
        phase1 = (phase0 + 1) % m_n_phase;
        n = m_pos / m_n_phase;

        if (n > m_blksize){
            break;
        }

        output[n_out++] = m_out[phase0][n] * m_mu + (1.0f - m_mu)*m_out[phase1][n];
    }

    return n_out;
}
        
        

        
