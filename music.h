#ifndef MUSIC_H
#define MUSIC_H

#include "global.h"

#include <iostream>
#include <iomanip>

const unsigned SAMPLE_POINTER_CAP = 0x100;
const unsigned SAMPLE_DATA_CAP = 0x10000;
const unsigned INSTRUMENT_DATA_CAP = 0x600;

const unsigned short SAMPLE_PTR_ARAM = 0x3C60;
const unsigned short SAMPLE_DATA_ARAM = 0xB960;
const unsigned short INSTRUMENT_ARAM = 0x3D00;

struct Music
{
    bool samplesAreDefined;

    unsigned samplePtrSize;
    unsigned sampleDataSize;
    unsigned instrumentSize;
    unsigned patternSize;
    unsigned trackSize;
    unsigned size;
    unsigned loopSize;

    unsigned offsetPC;
    unsigned short offsetARAM;
    
    byte samplePtrStart[4];
    byte samplePointers[SAMPLE_POINTER_CAP];
    byte sampleDataStart[4];
    byte sampleData[SAMPLE_DATA_CAP];
    byte insStart[4];
    byte instrumentData[INSTRUMENT_DATA_CAP];
    byte songStart[4];
    byte patternData[DATA_CAP];
    byte trackData[DATA_CAP];
    byte data[DATA_CAP];
    byte loopData[DATA_CAP];
    byte songEnd[10];
    
    byte* entireData;

    Music(unsigned off, unsigned short aram);
    ~Music();
    void write(bool isLoop, byte b);
    void overwrite(unsigned short offset, unsigned short b);
    void overwrite_byte(unsigned short offset, byte b);
    void sample_ptr_write(unsigned short p);
    void sample_data_write(byte b);
    void instrument_write(byte b);
    void pattern_write(unsigned short b);
    void pattern_overwrite(unsigned short offset, unsigned short b);
    void track_write(unsigned short b);
    void track_overwrite(unsigned short offset, unsigned short b);
    
    byte* get_entire_data();
    unsigned total_size();
    
    void byte_cat(byte*& dest, unsigned& destSize, byte* src, unsigned srcSize);
};

#endif // MUSIC_H
