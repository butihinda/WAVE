//
//  wave.h
//  wave
//
//  Created by Maxim Sivash on 8/8/17.
//  Copyright © 2017 Maxim Sivash. All rights reserved.
//

#include <stdint.h>

#ifndef wave_h
#define wave_h


        
typedef struct {
    char ChunkName[4];
    int32_t ChunkSize;
} RIFFHEADER;


typedef struct {
    int16_t wFormatTag;
    int16_t nChannels;
    int32_t nSamplesPerSec;
    int32_t nAvgBytesPerSec;
    int16_t nBlockAlign;
    int16_t wBitsPerSample;
    int16_t cbSize;
} WAVEFORMATEX;


typedef struct {
    FILE *file;
    char **file_path;
    
    long riff_size_pos;
    long data_size_pos;
    
    long riff_size;
    long data_size;
    
    int writable;
  
    WAVEFORMATEX FmtHeader;
    
} WAVE;


int WalkWave(FILE* file);
int WaveInitFromFile(char *filepath, WAVE *w);
int WaveInit(char *filepath, WAVE *w, int format_tag, int n_channels, int framerate, int bits_per_sample);
int WriteWaveformatex(FILE *f, WAVEFORMATEX *waveformatex);
int ReadWaveFormatex(FILE *f, WAVEFORMATEX *waveformatex, uint header_size);
int WaveClose(WAVE *w);

int WaveFramesN(WAVE *w, long *n);

int WaveWriteSamples(WAVE *w, int32_t *samples, uint8_t *temp_buffer, int sampleslen);
int WaveReadSamples(WAVE *w, uint32_t *samples, uint8_t *temp_buffer, int samples_len);

#endif /* wave_h */
