#include "byte.h"
#include "sample.h"

using namespace std;

Sample::Sample(byte s, unsigned short addr1, unsigned short addr2)
: slot(s), startAddress(addr1), loopAddress(addr2)
{
}

byte Sample::get_slot()
{
    return slot;
}

unsigned short Sample::get_start_address()
{
    return startAddress;
}

unsigned short Sample::get_loop_address()
{
    return loopAddress;
}
