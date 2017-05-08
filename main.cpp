#include <iostream>

#include "ROM.h"
#include "RAM.h"

using namespace std;

RAM* memory = nullptr;

void emulationLoop();
char decodeOpcode(short opcode, char*& decoded);

int hex2dec(char* hex, int size);
int hex2decDigit(char hex);

int main(int argc, char* argv[]) {

    if (argc != 2) {
        cerr << "Incorrect program usage. Correct usage: chippp <rom path>" << endl;
    }

    string romPath = argv[1];
    ROM* gameROM = new ROM(romPath);
    gameROM->loadFile();
    memory = new RAM;
    memory->loadROMIntoMemory(gameROM);

    emulationLoop();

    return 0;
}

void emulationLoop() {
    short programCounter = 512;
    short opcode = 0;
    char vRegisters[16];

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    for (;;) {
        opcode = memory->getOpcode(programCounter);
        char* decoded;

        /**
         *  NNN: address
            NN: 8-bit constant
            N: 4-bit constant
            X and Y: 4-bit register identifier
            I : 16bit register (For memory address) (Similar to void pointer)
         */

        char x, y;
        char*nn = new char[2];

        switch (decodeOpcode(opcode, decoded)) {
            case 8:
                //6XNN
                //Sets VX to NN.
                x = (char) hex2decDigit(decoded[1]);
                nn[0] = decoded[2];
                nn[1] = decoded[3];
                vRegisters[x] = (char) hex2dec(nn, 2);
                break;
            default:
                cerr << "Unimplemented opcode: " << decoded << endl;
                return;
        }

        programCounter += 2;
    }
#pragma clang diagnostic pop
}

char decodeOpcode(short opcode, char*& decoded) {
    char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    char* d = new char[4];

    d[3] = digits[opcode % 16];
    opcode /= 16;
    d[2] = digits[opcode % 16];
    opcode /= 16;
    d[1] = digits[opcode % 16];
    opcode /= 16;
    d[0] = digits[opcode];

    decoded = d;

    //https://en.wikipedia.org/wiki/CHIP-8#Opcode_table
    if (strcmp(decoded, "00E0") == 0) return 1;
    if (strcmp(decoded, "00EE") == 0) return 2;
    if (decoded[0] == '0') return 0;
    if (decoded[0] == '1') return 3;
    if (decoded[0] == '2') return 4;
    if (decoded[0] == '3') return 5;
    if (decoded[0] == '4') return 6;
    if (decoded[0] == '5') return 7;
    if (decoded[0] == '6') return 8;
    if (decoded[0] == '7') return 9;
    if (decoded[0] == '8') {
        if (decoded[3] == '0') return 10;
        if (decoded[3] == '1') return 11;
        if (decoded[3] == '2') return 12;
        if (decoded[3] == '3') return 13;
        if (decoded[3] == '4') return 14;
        if (decoded[3] == '5') return 15;
        if (decoded[3] == '6') return 16;
        if (decoded[3] == '7') return 17;
        if (decoded[3] == 'E') return 18;
    }
    if (decoded[0] == '9') return 19;
    if (decoded[0] == 'A') return 20;
    if (decoded[0] == 'B') return 21;
    if (decoded[0] == 'C') return 22;
    if (decoded[0] == 'D') return 23;
    if (decoded[0] == 'E') {
        if (decoded[3] == 'E') return 24;
        if (decoded[3] == '1') return 25;
    }
    if (decoded[0] == 'F') {
        if (decoded[3] == '7') return 26;
        if (decoded[3] == 'A') return 27;
        if (decoded[3] == '5') {
            if (decoded[2] == '1') return 28;
            if (decoded[2] == '5') return 33;
            if (decoded[2] == '6') return 34;
        }
        if (decoded[3] == '8') return 29;
        if (decoded[3] == 'E') return 30;
        if (decoded[3] == '9') return 31;
        if (decoded[3] == '3') return 32;
    }

    return -1; //error state
}


int hex2dec(char* hex, int size) {
    if (size == 1) {
        return hex2decDigit(hex[0]);
    } else if (size == 2) {
        int digit1 = hex2decDigit(hex[0]);
        int digit2 = hex2decDigit(hex[1]);

        return (digit1 * 16) + digit2;
    }
}

int hex2decDigit(char hex) {
    switch (hex) {
        case '0' : return 0;
        case '1' : return 1;
        case '2' : return 2;
        case '3' : return 3;
        case '4' : return 4;
        case '5' : return 5;
        case '6' : return 6;
        case '7' : return 7;
        case '8' : return 8;
        case '9' : return 9;
        case 'A' : return 10;
        case 'B' : return 11;
        case 'C' : return 12;
        case 'D' : return 13;
        case 'E' : return 14;
        case 'F' : return 15;
        default:
            cerr << "Invalid hex digit" << endl;
            exit(1);
    }
}
