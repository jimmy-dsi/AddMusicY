#include "byte.h"
#include "music.h"

#include <iostream>
#include <iomanip>

using namespace std;

Music::Music(unsigned pc, unsigned short aram)
{
    samplesAreDefined = false;

    samplePtrSize = 0;
    sampleDataSize = 0;
    instrumentSize = 0;
    patternSize = 0;
    trackSize = 0;
    size = 0;

    offsetPC = pc;
    offsetARAM = aram;

    songStart[0] = 0x00;
    songStart[1] = 0x00;
    songStart[2] = (byte)(ARAM_START & 255);
    songStart[3] = (byte)((ARAM_START >> 8) & 255);

    songEnd[0] = 0x02;
    songEnd[1] = 0x00;
    songEnd[2] = 0x90;
    songEnd[3] = 0xFF;
    songEnd[4] = (byte)(ARAM_START & 255);
    songEnd[5] = (byte)((ARAM_START >> 8) & 255);
    songEnd[6] = 0x00;
    songEnd[7] = 0x00;
    songEnd[8] = 0x00;
    songEnd[9] = 0x04;

    entireData = NULL;
}

Music::~Music()
{
    delete[] entireData;
}

void Music::write(bool isLoop, byte b)
{
    if (!isLoop)
    {
        data[size] = b;
        size++;

        offsetPC++;
        offsetARAM++;
    }
    else
    {
        loopData[loopSize] = b;
        loopSize++;
    }

    //offsetPC++;
    //offsetARAM++;
}

void Music::overwrite(unsigned short offset, unsigned short b)
{
    data[offset] = (byte)(b & 255);
    data[offset + 1] = (byte)((b >> 8) & 255);
}

void Music::overwrite_byte(unsigned short offset, byte b)
{
    data[offset] = b;
}

void Music::sample_ptr_write(unsigned short p)
{
    samplePointers[samplePtrSize] = (byte)(p & 255);
    samplePointers[samplePtrSize + 1] = (byte)((p >> 8) & 255);
    
    samplePtrSize += 2;
}

void Music::sample_data_write(byte b)
{
    sampleData[sampleDataSize] = b;
    sampleDataSize++;
}

void Music::instrument_write(byte b)
{
    instrumentData[instrumentSize] = b;
    instrumentSize++;
}

void Music::pattern_write(unsigned short b)
{
    patternData[patternSize] = (byte)(b & 255);
    patternData[patternSize + 1] = (byte)(b >> 8);

    patternSize += 2;
    offsetPC += 2;
    offsetARAM += 2;
}

void Music::pattern_overwrite(unsigned short offset, unsigned short b)
{
    patternData[offset] = (byte)(b & 255);
    patternData[offset + 1] = (byte)((b >> 8) & 255);
}

void Music::track_write(unsigned short b)
{
    trackData[trackSize] = (byte)(b & 255);
    trackData[trackSize + 1] = (byte)((b >> 8) & 255);

    trackSize += 2;
    offsetPC += 2;
    offsetARAM += 2;
}

void Music::track_overwrite(unsigned short offset, unsigned short b)
{
    trackData[offset] = (byte)(b & 255);
    trackData[offset + 1] = (byte)(b >> 8);
}

byte* Music::get_entire_data()
{
    unsigned sz = total_size();
    delete[] entireData;
    entireData = new byte[sz];
    
    unsigned currentSize = 0;
    
    if (samplesAreDefined)
    {
        byte_cat(entireData, currentSize, samplePtrStart, 4);
        byte_cat(entireData, currentSize, samplePointers, samplePtrSize);
        byte_cat(entireData, currentSize, sampleDataStart, 4);
        byte_cat(entireData, currentSize, sampleData, sampleDataSize);
    }
    
    byte_cat(entireData, currentSize, insStart, 4);
    byte_cat(entireData, currentSize, instrumentData, instrumentSize);
    byte_cat(entireData, currentSize, songStart, 4);
    byte_cat(entireData, currentSize, patternData, patternSize);
    byte_cat(entireData, currentSize, trackData, trackSize);
    byte_cat(entireData, currentSize, data, size);
    byte_cat(entireData, currentSize, loopData, loopSize);
    byte_cat(entireData, currentSize, songEnd, 10);
    
    return entireData;
}

unsigned Music::total_size()
{
    unsigned sz = 0;
    
    if (samplesAreDefined)
    {
        sz += 4 + samplePtrSize + 4 + sampleDataSize;
    }
    
    sz += 4 + instrumentSize + 4 + patternSize + trackSize + size + loopSize + 10;
    return sz;
}

void Music::byte_cat(byte*& dest, unsigned& destSize, byte* src, unsigned srcSize)
{
    for (unsigned i = 0; i < srcSize; i++)
    {
        dest[destSize + i] = src[i];
    }
    
    destSize += srcSize;
}
