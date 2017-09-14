//
//  wave.c
//  wave
//
//  Created by Maxim Sivash on 8/8/17.
//  Copyright © 2017 Maxim Sivash. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "wave.h"

int NextRiff(FILE* file, RIFFHEADER* RiffHeader) {
    int result = 0;
    
    fread(&RiffHeader->ChunkName, 4 * sizeof(int8_t), 1, file);
    fread(&RiffHeader->ChunkSize, sizeof(int32_t), 1, file);
    
    return result;
}


int WaveInitFromFile(char *filepath, WAVE *w) {
    RIFFHEADER riffh;
    
    int32_t total_size;
    char riff[4];
    char wave[4];

    int offset    = 0;
    int result    = 0;
    int fmt_found = 0;
    
    w->writable = 0;
    
    w->file = fopen(filepath, "rb");
    if (w->file == NULL){
        printf("ERROR opening %s\n", filepath);
    }
    
    // Walk through wav file, find fmt shunk
    fread(&riff, 4, 1, w->file);
    fread(&total_size, sizeof(total_size), 1, w->file);
    
    fread(&wave, 4, 1, w->file);
    offset += 4;
    
    while(1) {
        NextRiff(w->file, &riffh);
        printf("chunk name %.4s, chunk size %d\n", riffh.ChunkName, riffh.ChunkSize);
        
        if (!strncmp(riffh.ChunkName, "fmt ", 4)) {
            fmt_found = 1;
            ReadWaveFormatex(w->file, &w->FmtHeader, riffh.ChunkSize);
        }
        else if (offset >= total_size || !strncmp(riffh.ChunkName, "data", 4)) {
            w->data_size = riffh.ChunkSize;
            break;
        }
        else {
            fseek(w->file, riffh.ChunkSize, SEEK_CUR);
        }
        
        offset += riffh.ChunkSize;
        printf("offset = %d\n", offset);
        printf("position = %ld\n", ftell(w->file));
        
    }

    if (fmt_found != 1) {
        result = 1;
    }
    
    return result;
}


int WaveInit(char *filepath, WAVE *w, int format_tag, int n_channels, int framerate, int bits_per_sample) {

    int32_t fmt_chunk_size = 16;
    int32_t data_chunk_size = 0;
    
    w->writable = 1;
    
    // filling waveformatex
    w->FmtHeader.wFormatTag     = format_tag;
    w->FmtHeader.nChannels      = n_channels;
    w->FmtHeader.nSamplesPerSec = framerate;
    w->FmtHeader.wBitsPerSample = bits_per_sample;
    w->FmtHeader.nBlockAlign    = n_channels * bits_per_sample / 8;
    w->FmtHeader.nAvgBytesPerSec = w->FmtHeader.nBlockAlign * w->FmtHeader.nSamplesPerSec;
    
    w->file = fopen(filepath, "wb");
    
    fwrite("RIFF", sizeof(int8_t), 4, w->file);
    
    w->riff_size_pos = ftell(w->file);
    fwrite(&data_chunk_size, sizeof(int32_t), 1, w->file);
    
    fwrite("WAVE",          sizeof(int8_t) , 4, w->file);
    fwrite("fmt ",          sizeof(int8_t) , 4, w->file);
    fwrite(&fmt_chunk_size, sizeof(int32_t), 1, w->file);
    WriteWaveformatex(w->file, &w->FmtHeader);
    fwrite("data", sizeof(int8_t), 4, w->file);
    w->data_size_pos = ftell(w->file);
    fwrite(&data_chunk_size, sizeof(int32_t), 1, w->file);
    
    w->riff_size = ftell(w->file) - 8;
    
    return 0;
}

int WaveClose(WAVE *w){

    long current_pos;
    int32_t data_len, riff_len;
    
    if (w->writable) {
        
        printf("writable = %d\n", w->writable);
        
        current_pos = ftell(w->file);
        data_len = current_pos - w->data_size_pos - 4;
        riff_len = current_pos - w->riff_size_pos - 4;
        
        printf("data len = %d\n", data_len);
    
        fseek(w->file, w->riff_size_pos, SEEK_SET);
        fwrite(&riff_len, sizeof(data_len), 1, w->file);

        fseek(w->file, w->data_size_pos, SEEK_SET);
        fwrite(&data_len, sizeof(data_len), 1, w->file);
    }
    
    fclose(w->file);
    return 0;
}

int WriteWaveformatex(FILE *f, WAVEFORMATEX *waveformatex) {
    
    fwrite(&waveformatex->wFormatTag,      sizeof(int16_t), 1, f);
    fwrite(&waveformatex->nChannels,       sizeof(int16_t), 1, f);
    fwrite(&waveformatex->nSamplesPerSec,  sizeof(int32_t), 1, f);
    fwrite(&waveformatex->nAvgBytesPerSec, sizeof(int32_t), 1, f);
    fwrite(&waveformatex->nBlockAlign,     sizeof(int16_t), 1, f);
    fwrite(&waveformatex->wBitsPerSample,  sizeof(int16_t), 1, f);
    
    return 0;
    
}

int ReadWaveFormatex(FILE *f, WAVEFORMATEX *waveformatex, uint header_size)
{
    fread(&waveformatex->wFormatTag, 		      sizeof(int16_t), 1, f);
    fread(&waveformatex->nChannels,               sizeof(int16_t), 1, f);
    fread(&waveformatex->nSamplesPerSec,          sizeof(int32_t), 1, f);
    fread(&waveformatex->nAvgBytesPerSec,         sizeof(int32_t), 1, f);
    fread(&waveformatex->nBlockAlign,             sizeof(int16_t), 1, f);
    fread(&waveformatex->wBitsPerSample,          sizeof(int16_t), 1, f);
    
    if (header_size > 16) {
        fread(&waveformatex->cbSize, sizeof(int16_t), 1, f);
    }
    
    return 0;
}


int WaveWriteSamples (WAVE *w, int32_t *samples, uint8_t *temp_buffer, int sampleslen)
{
    // temp buffer len should be at least sizeof(samples) * 4
    
    int i, shift, j, samplewidth, writebuffer_len;
    samplewidth = w->FmtHeader.wBitsPerSample / 8;
    
    writebuffer_len = sampleslen * samplewidth;
    for(i = 0; i < sampleslen; ++i) {
        shift = 24;
        for(j = samplewidth -1; j >= 0; --j) {
            temp_buffer[i * samplewidth + j] = (samples[i] >> shift);
            shift -= 8;
        }
    }
    
    fwrite(temp_buffer, writebuffer_len, 1, w->file);
    return 0;
}

int WaveReadSamples(WAVE *w, uint32_t *samples, uint8_t *temp_buffer, int samples_len)
{
    int i, j, shift, fread_return, readbuffer_len, samplewidth;
    
    samplewidth = w->FmtHeader.wBitsPerSample / 8;
    
    readbuffer_len = samples_len * samplewidth;
    
    
    
    fread_return = fread(temp_buffer, readbuffer_len, 1, w->file);
    if(1 != fread_return) {
        printf("!!!!!!fread error!!!!!!!!!!!!! read %d elements\n", fread_return);
        return -1;
    }
    
    for(i = 0; i < samples_len; ++i) {
        samples[i] = 0;
        shift = 24;
        for(j = samplewidth - 1; j >= 0; --j) {
            samples[i] = samples[i] | (temp_buffer[i * samplewidth + j] << shift);
            shift -= 8;
        }
    }
    return 0;
}

int WaveFramesN(WAVE *w, long *n)
{
    *n = w->data_size / (w->FmtHeader.wBitsPerSample / 8) / w->FmtHeader.nChannels;
    return 0;
}
