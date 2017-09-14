//
//  main.c
//  wave
//
//  Created by Maxim Sivash on 8/8/17.
//  Copyright © 2017 Maxim Sivash. All rights reserved.
//

#define BUFLEN 1024

#include <stdio.h>
#include <string.h>
#include "wave.h"

int main(int argc, const char * argv[]) {

    char wav_path[] = "/Users/maximsivash/Documents/c_wave/wave/wav_files/sweep_24.wav";
    
    int8_t temp_buffer[BUFLEN * 4];
    int32_t samples_buffer[BUFLEN];

    WAVE w;
    WAVE w_;
    
    long samples_left = 0;
    long buflen = BUFLEN;
    
    WaveInitFromFile(wav_path, &w);
    WaveInit("/Users/maximsivash/Documents/c_wave/wave/wav_files/result.wav", &w_, w.FmtHeader.wFormatTag,
                w.FmtHeader.nChannels, w.FmtHeader.nSamplesPerSec, 32);
    
    WaveFramesN(&w, &samples_left);
    samples_left *= w.FmtHeader.nChannels;
    
    while(samples_left) {
       
        WaveReadSamples(&w, samples_buffer, temp_buffer, buflen);
        WaveWriteSamples(&w_, samples_buffer, temp_buffer, buflen);
        
        samples_left -= buflen;
        if (buflen > samples_left)
            buflen = samples_left;
        printf("buflen %d, samples left %d\n", buflen, samples_left);
        
        if (samples_left == 0)
            break;
    }
//    printf("ready to exit\n");
    
    WaveClose(&w);
    WaveClose(&w_);
    
}
