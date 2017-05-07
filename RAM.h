//
// Created by Samuel Smith on 5/6/17.
//

#ifndef CHIPPLUSPLUS_RAM_H
#define CHIPPLUSPLUS_RAM_H


#include "ROM.h"

class RAM {
private:
    char memory[4096];
public:
    void loadROMIntoMemory(ROM* rom);
    short getOpcode(int memoryLocation);
};


#endif //CHIPPLUSPLUS_RAM_H
