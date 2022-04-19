#ifndef BRR_H
#define BRR_H

#include "global.h"

#include <cstdio>
#include <string>
#include <vector>

const unsigned BUFFER_SIZE = 1024;
const std::string SAMPLES_FOLDER = "samples/";

class Brr
{
    private:
        unsigned sz;
        unsigned short loopPoint;
        std::vector<byte> data;
        bool error;
        
    public:
        Brr(std::string filename);
        unsigned size();
        unsigned short get_loop_point();
        byte read_data(unsigned index);
        bool has_error();
};

#endif // BRR_H
