#include "byte.h"
#include "brr.h"

#include <iostream>

using namespace std;

Brr::Brr(string filename)
: sz(0), loopPoint(0), data(), error(false)
{
    FILE* pFile;
    pFile = fopen((SAMPLES_FOLDER + filename).c_str(), "rb");
    
    if (pFile == NULL)
    {
        error = true;
    }
    else
    {
        byte loop[2];
        unsigned result = fread(&loop, 1, 2, pFile);
        
        if (result == 2)
        {
            loopPoint = (unsigned short)loop[0];
            loopPoint |= ((unsigned short)loop[1] << 8);
            
            do
            {
                byte buf[BUFFER_SIZE];
                result = fread(&buf, 1, BUFFER_SIZE, pFile);
                
                for (unsigned i = 0; i < result; i++)
                {
                    data.push_back(buf[i]);
                }
            }
            while (result >= BUFFER_SIZE);
            
            sz = data.size();
        }
        
        fclose(pFile);
    }
}

unsigned Brr::size()
{
    return sz;
}

unsigned short Brr::get_loop_point()
{
    return loopPoint;
}

bool Brr::has_error()
{
    return error;
}

byte Brr::read_data(unsigned index)
{
    return data[index];
}
