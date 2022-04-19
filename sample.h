#ifndef SAMPLE_H
#define SAMPLE_H

#include "global.h"

class Sample
{
    private:
        byte slot;
        unsigned short startAddress;
        unsigned short loopAddress;
        
    public:
        Sample() {}
        Sample(byte s, unsigned short addr1, unsigned short addr2);
        byte get_slot();
        unsigned short get_start_address();
        unsigned short get_loop_address();
};

#endif // SAMPLE_H
