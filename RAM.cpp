//
// Created by Samuel Smith on 5/6/17.
//

#include "RAM.h"

short RAM::getOpcode(int memoryPosition) {
    char op1 = memory[memoryPosition];
    char op2 = memory[memoryPosition + 1];
    short opcode = op1 << 8; //take 1 byte char, and add 8 bits to the right
    opcode |= op2; //use bitwise or to merge op2 into the right hand side of the opcode
    return opcode;
}

void RAM::loadROMIntoMemory(ROM* rom) {
    for (int i = 512; i < 3744; i++) {
        memory[i] = rom->nextByte();
    }
}