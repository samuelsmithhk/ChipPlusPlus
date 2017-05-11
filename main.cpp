#include <iostream>

#include "ROM.h"

using namespace std;

unsigned char memory[4096];
unsigned short programCounter = 512;
unsigned short stack[16];
unsigned short stackPointer = 0;
unsigned char vRegisters[15];
bool vFlag = false;
unsigned short iRegister = 0;
bool display[64][32];
bool draw = false;

void loadROMIntoMemory(ROM* rom) {
    for (int i = 512; i < 3744; i++) {
        memory[i] = rom->nextByte();
    }
}

unsigned short getInstruction(int memoryPosition) {
    unsigned char op1 = memory[memoryPosition];
    unsigned char op2 = memory[memoryPosition + 1];
    unsigned short instruction = op1 << 8; //take 1 byte char, and add 8 bits to the right
    instruction |= op2; //use bitwise or to merge op2 into the right hand side of the opcode
    return instruction;
}

char decodeInstruction(unsigned short instruction, char*& decoded) {
    char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    char* d = new char[4];

    d[3] = digits[instruction % 16];
    instruction /= 16;
    d[2] = digits[instruction % 16];
    instruction /= 16;
    d[1] = digits[instruction % 16];
    instruction /= 16;
    d[0] = digits[instruction];

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

int hex2dec(char* hex, int size) {
    if (size == 1) {
        return hex2decDigit(hex[0]);
    } else if (size == 2) {
        int digit1 = hex2decDigit(hex[0]);
        int digit2 = hex2decDigit(hex[1]);

        return (digit1 * 16) + digit2;
    } else if (size == 3) {
        int digit1 = hex2decDigit(hex[0]);
        int digit2 = hex2decDigit(hex[1]);
        int digit3 = hex2decDigit(hex[2]);

        return (digit1 * 256) + (digit2 * 16) + digit3;
    }
    return -1;
}

bool loadSpriteIntoDisplay(int x, int y, int n) {
    bool unset = false;
    int pixelCount = 8 * n;
    bool sprite[pixelCount];
    int pixelIndex = 0;

    for (unsigned short i = 0; i < n; i ++) {
        unsigned short payloadAddress = iRegister + i;
        unsigned char payload = memory[payloadAddress];

        for (int j = 7; j >= 0; j--) {
            sprite[pixelIndex + j] = (payload % 2 == 1);
            payload /= 2;
        }
        pixelIndex += 8;
    }

    pixelIndex = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < n; j++) {
            bool displayBit = display[x + i][y + j];
            bool spriteBit = sprite[pixelIndex];

            if (displayBit == spriteBit) {
                unset = true;
                displayBit = false;
            } else if (spriteBit) {
                unset = true;
                displayBit = true;
            } else {
                displayBit = true;
            }
            display[x + i][y + j] = displayBit;
            pixelIndex += 1;
        }
    }

    return unset;
}

void drawDisplay() {
    //temporary, will move to opengl later
    for (int i = 0; i < 100; i++) {
        cout << endl;
    }

    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            cout << (display[j][i] ? "\u25A0" : " ");
            cout << (display[j][i] ? "\u25A0" : " ");
        }
        cout << endl;
    }
}

int nextInstruction() {
    unsigned short currentInstruction = getInstruction(programCounter);
    char *decoded;

    /**
        NNN: address
        NN: 8-bit constant
        N: 4-bit constant
        X and Y: 4-bit register identifier
        I : 16bit register (For memory address) (Similar to void pointer)
    */

    char x, y, n;
    char *nn = new char[2];
    char *nnn = new char[3];

    switch (decodeInstruction(currentInstruction, decoded)) {
        case 4:
            //2NNN
            //Calls subroutine at NNN.
            nnn[0] = decoded[1];
            nnn[1] = decoded[2];
            nnn[2] = decoded[3];
            stack[stackPointer] = programCounter;
            stackPointer++;
            programCounter = (unsigned short) hex2dec(nnn, 3);
            break;
        case 8:
            //6XNN
            //Sets VX to NN.
            x = (char) hex2decDigit(decoded[1]);
            nn[0] = decoded[2];
            nn[1] = decoded[3];
            vRegisters[x] = (unsigned char) hex2dec(nn, 2);
            programCounter += 2;
            break;
        case 20:
            //ANNN
            //Sets I to NNN
            nnn[0] = decoded[1];
            nnn[1] = decoded[2];
            nnn[2] = decoded[3];
            iRegister = (unsigned short) hex2dec(nnn, 3);
            programCounter += 2;
            break;
        case 23:
            //DXYN
            //Draws a sprite
            //Co-ordinates X and Y
            //Width 8 pixels
            //Height N pixels
            //reading of pixel values starts at iRegister, bit encoded
            //if a pixel that was set before this instruction is unset, set vf to 1
            //if no pixels get unset, set vf to 0
            x = (char) hex2decDigit(decoded[1]);
            y = (char) hex2decDigit(decoded[2]);
            n = (char) hex2decDigit(decoded[3]);

            vFlag = loadSpriteIntoDisplay(x, y, n);
            draw = true;
            programCounter += 2;
            break;
        default:
            cerr << "Unimplemented opcode: " << decoded << endl;
            return 1;
    }

    return 0;
}

int main(int argc, char* argv[]) {

    if (argc != 2) {
        cerr << "Incorrect program usage. Correct usage: chippp <rom path>" << endl;
    }

    string romPath = argv[1];
    ROM* gameROM = new ROM(romPath);
    gameROM->loadFile();
    loadROMIntoMemory(gameROM);

    for (;;) {
        if (nextInstruction() == 1) {
            cerr << "Execution failed for some reason";
            return 1;
        }

        if (draw) {
            drawDisplay();
            draw = false;
        }
    }
}