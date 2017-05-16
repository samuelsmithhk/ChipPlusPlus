#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <iostream>

#include "ROM.h"

using namespace std;

unsigned char memory[4096];
unsigned short programCounter = 512;
unsigned short stack[16];
unsigned short stackPointer = 0;
unsigned char vRegisters[16];
unsigned short iRegister = 0;
bool display[64][32];
bool keyboard[16];
unsigned char delayTimer = 0;

chrono::high_resolution_clock::time_point timestamp1, timestamp2; //for timers
double timeSinceDelayTick = 0.0;

void loadROMIntoMemory(ROM* rom) {
    for (int i = 512; i < 3744; i++) {
        memory[i] = rom->nextByte();
    }
}

void loadFontSetIntoMemory() {
    memory[80] = 240;   //1111  ****
    memory[81] = 144;   //1001  *  *
    memory[82] = 144;   //1001  *  *
    memory[83] = 144;   //1001  *  *
    memory[84] = 240;   //1111  ****

    memory[85] = 32;    //0010    *
    memory[86] = 96;    //0110   **
    memory[87] = 32;    //0010    *
    memory[88] = 32;    //0010    *
    memory[89] = 112;   //0111   ***

    memory[90] = 240;   //1111  ****
    memory[91] = 16;    //0001     *
    memory[92] = 240;   //1111  ****
    memory[93] = 128;   //1000  *
    memory[94] = 240;   //1111  ****

    memory[95] = 240;   //1111  ****
    memory[96] = 16;    //0001     *
    memory[97] = 240;   //1111  ****
    memory[98] = 16;    //0001     *
    memory[99] = 240;   //1111  ****

    memory[100] = 144;  //1001  *  *
    memory[101] = 144;  //1001  *  *
    memory[102] = 240;  //1111  ****
    memory[103] = 16;   //0001     *
    memory[104] = 16;   //0001     *

    memory[105] = 240;  //1111  ****
    memory[106] = 128;  //1000  *
    memory[107] = 240;  //1111  ****
    memory[108] = 16;   //0001     *
    memory[109] = 240;  //1111  ****

    memory[110] = 240;  //1111  ****
    memory[111] = 128;  //1000  *
    memory[112] = 240;  //1111  ****
    memory[113] = 144;  //1001  *  *
    memory[114] = 240;  //1111  ****

    memory[115] = 240;  //1111  ****
    memory[116] = 16;   //0001     *
    memory[117] = 32;   //0010    *
    memory[118] = 65;   //0100   *
    memory[119] = 65;   //0100   *

    memory[120] = 240;  //1111  ****
    memory[121] = 144;  //1001  *  *
    memory[122] = 240;  //1111  ****
    memory[123] = 144;  //1001  *  *
    memory[124] = 240;  //1111  ****

    memory[125] = 240;  //1111  ****
    memory[126] = 144;  //1001  *  *
    memory[127] = 240;  //1111  ****
    memory[128] = 16;   //0001     *
    memory[129] = 16;   //0001     *

    memory[130] = 240;  //1111  ****
    memory[131] = 144;  //1001  *  *
    memory[132] = 240;  //1111  ****
    memory[133] = 144;  //1001  *  *
    memory[134] = 144;  //1001  *  *

    memory[135] = 224;  //1110  ***
    memory[136] = 144;  //1001  *  *
    memory[137] = 224;  //1110  ***
    memory[138] = 144;  //1001  *  *
    memory[139] = 224;  //1110  ***

    memory[140] = 240;  //1111  ****
    memory[141] = 128;  //1000  *
    memory[142] = 128;  //1000  *
    memory[143] = 128;  //1000  *
    memory[144] = 240;  //1111  ****

    memory[145] = 224;  //1110  ***
    memory[146] = 144;  //1001  *  *
    memory[147] = 144;  //1001  *  *
    memory[148] = 144;  //1001  *  *
    memory[149] = 224;  //1119  ***

    memory[150] = 240;  //1111  ****
    memory[151] = 128;  //1000  *
    memory[152] = 240;  //1111  ****
    memory[153] = 128;  //1000  *
    memory[154] = 240;  //1111  ****

    memory[155] = 240;  //1111  ****
    memory[156] = 128;  //1000  *
    memory[157] = 240;  //1111  ****
    memory[158] = 128;  //1000  *
    memory[159] = 128;  //1000  *
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
    bool sprite[8][n];

    for (unsigned short i = 0; i < n; i++) {
        unsigned short payloadAddress = iRegister + i;
        unsigned char payload = memory[payloadAddress];

        for (int j = 7; j >= 0; j--) {
            sprite[j][i] = (payload % 2 == 1);
            payload /= 2;
        }
    }

    for (int line = 0; line < n; line++) {
        int addressY = y + line;
        for (int bit = 0; bit < 8; bit++) {
            int addressX = x + bit;
            bool displayBit = display[addressX][addressY];
            bool spriteBit = sprite[bit][line];

            if (displayBit && spriteBit) {
                unset = true;
            }

            displayBit = displayBit != spriteBit;
            display[addressX][addressY] = displayBit;
        }
    }

    return unset;
}

void processKey(unsigned char key, int x, int y, bool down) {
    if (key < 58) {
        keyboard[key - 48] = down;
    } else {
        switch (key) {
            case 97:
            case 65:
                keyboard[10] = down;
                break;
            case 98:
            case 66:
                keyboard[11] = down;
                break;
            case 99:
            case 67:
                keyboard[12] = down;
                break;
            case 100:
            case 68:
                keyboard[13] = down;
                break;
            case 101:
            case 69:
                keyboard[14] = down;
                break;
            case 102:
            case 70:
                keyboard[15] = down;
                break;
            default:
                break;
        }
    }
}

void processKeyDown(unsigned char key, int x, int y) {
    processKey(key, x, y, true);
}

void processKeyUp(unsigned char key, int x, int y) {
    processKey(key, x, y, false);
}

void drawDisplay() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    double xPos = -0.984375;
    double yPos = 0.96875;

    glBegin(GL_TRIANGLES);

    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (display[x][y]) {
                glVertex3f(xPos + 0.0, yPos + 0.03125,0.0);
                glVertex3f(xPos + 0.03125, yPos + -0.03125,0.0);
                glVertex3f(xPos + 0.0, yPos+ -0.03125,0.0);

                glVertex3f(xPos + 0.03125, yPos + 0.03125,0.0);
                glVertex3f(xPos + 0.03125, yPos + -0.03125,0.0);
                glVertex3f(xPos + 0.0, yPos + 0.03125,0.0);
            }
            xPos += 0.03125;
        }
        xPos = -1.0;
        yPos -= 0.0625;
    }

    glEnd();
    glutSwapBuffers();
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

    unsigned char x, y, n;
    char *nn = new char[2];
    char *nnn = new char[3];
    unsigned short temp;

    switch (decodeInstruction(currentInstruction, decoded)) {
        case 2:
            //OOEE
            //Returns from subroutine
            stackPointer--;
            programCounter = stack[stackPointer];
            programCounter += 2;
            break;
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
        case 5:
            //3XNN
            //Skips next instruction if V(X) is equal to NN
            x = (unsigned char) hex2decDigit(decoded[1]);
            nn[0] = decoded[2];
            nn[1] = decoded[3];

            if (x == (unsigned char) hex2dec(nn, 2)) {
                programCounter += 2;
            }

            programCounter += 2;
            break;
        case 8:
            //6XNN
            //Sets VX to NN.
            x = (unsigned char) hex2decDigit(decoded[1]);
            nn[0] = decoded[2];
            nn[1] = decoded[3];
            vRegisters[x] = (unsigned char) hex2dec(nn, 2);
            programCounter += 2;
            break;
        case 9:
            //7XNN
            //Adds NN to V(X)
            x = (unsigned char) hex2decDigit(decoded[1]);
            nn[0] = decoded[2];
            nn[1] = decoded[3];
            vRegisters[x] += (unsigned char) hex2dec(nn, 2);
            programCounter += 2;
            break;
        case 12:
            //8XY2
            //Set X to v[X] & v[Y]
            x = (unsigned char) hex2decDigit(decoded[1]);
            y = (unsigned char) hex2decDigit(decoded[2]);
            vRegisters[x] = vRegisters[x] & vRegisters[y];
            programCounter += 2;
            break;
        case 14:
            //8XY4
            //Add V(Y) to V(X), if result greater than 255, subtract 255 and set v[15] to 1 (0 if not)
            x = (unsigned char) hex2decDigit(decoded[1]);
            y = (unsigned char) hex2decDigit(decoded[2]);
            temp = vRegisters[x] + vRegisters[y];

            if (temp > 255) {
                temp -= 255;
                vRegisters[15] = 1;
            } else {
                vRegisters[15] = 0;
            }

            vRegisters[x] = (unsigned char) temp;
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
        case 22:
            //CXNN
            //Sets V(X) to the result of a bitwise AND operation between NN and a random number between 0 and 255
            x = (unsigned char) hex2decDigit(decoded[1]);
            nn[0] = decoded[2];
            nn[1] = decoded[3];

            temp = (unsigned short) (chrono::system_clock::now().time_since_epoch().count() % 250);
            vRegisters[x] = (unsigned char) temp & (unsigned char) hex2dec(nn, 2);
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
            x = (unsigned char) hex2decDigit(decoded[1]);
            y = (unsigned char) hex2decDigit(decoded[2]);
            n = (unsigned char) hex2decDigit(decoded[3]);

            vRegisters[15] = (unsigned char) (loadSpriteIntoDisplay(x, y, n) ? 1 : 0);
            programCounter += 2;
            break;
        case 25:
            //EXA1
            //Skip next instruction if key stored in V(X) is not pressed
            x = (unsigned char) hex2decDigit(decoded[1]);
            temp = vRegisters[x];
            if (!keyboard[temp]) {
                programCounter += 2;
            }
            programCounter += 2;
            break;
        case 26:
            //FX07
            //Sets V(X) to the value of the delay timer
            x = (char) hex2decDigit(decoded[1]);
            vRegisters[x] = delayTimer;
            programCounter += 2;
            break;
        case 28:
            //FX15
            //Sets the delay timer to V(X)
            x = (char) hex2decDigit(decoded[1]);
            delayTimer = vRegisters[x];
            programCounter += 2;
            break;
        case 31:
            //FX29
            //Sets I to the location of the font-character of the character stored in V(X)
            x = (char) hex2decDigit(decoded[1]);
            temp = 80;

            for (int c = 0; c <= vRegisters[x]; c++) {
                temp += 5;
            }

            iRegister = temp;
            programCounter += 2;
            break;
        case 32:
            //FX33
            //Get the number in V(X).
            //Store the hundreds digit at address I
            //Store the tens digit at address I + 1
            //Store the unit digit at address I + 2
            x = (char) hex2decDigit(decoded[1]);
            temp = vRegisters[x];
            memory[iRegister + 2] = (unsigned char) (temp % 10);
            temp /= 10;
            memory[iRegister + 1] = (unsigned char) (temp % 10);
            temp /= 10;
            memory[iRegister] = (unsigned char) (temp % 10);
            programCounter += 2;
            break;
        case 34:
            //FX65
            //Stores from V(0) to V(X) in memory starting at address I
            x = (char) hex2decDigit(decoded[1]);
            for (char i = 0; i <= x; i++) {
                memory[iRegister + i] = vRegisters[i];
            }

            programCounter += 2;
            break;
        default:
            cerr << "Unimplemented opcode: " << decoded << endl;
            return 1;
    }

    return 0;
}

void nextStep() {
    if (nextInstruction() == 1) {
        return;
        //exit(1);
    }

    timestamp2 = timestamp1;
    timestamp1 = chrono::high_resolution_clock::now();

    if (delayTimer > 0) {
        chrono::duration<double, milli> time = timestamp1 - timestamp2;
        timeSinceDelayTick += time.count();
        if (timeSinceDelayTick >= 16.667) {
            delayTimer -= 1;
            timeSinceDelayTick = 0.0;
        }
    }

    drawDisplay();
    glutPostRedisplay();
}

int main(int argc, char* argv[]) {

    if (argc != 2) {
        cerr << "Incorrect program usage. Correct usage: chippp <rom path>" << endl;
    }

    string romPath = argv[1];
    ROM* gameROM = new ROM(romPath);
    gameROM->loadFile();
    loadROMIntoMemory(gameROM);
    loadFontSetIntoMemory();

    glutInit(&argc, argv);
    glutInitWindowPosition(-1, -1);
    glutInitWindowSize(640, 320);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutCreateWindow("ChipPlusPlus");
    glutKeyboardFunc(processKeyDown);
    glutKeyboardUpFunc(processKeyUp);
    glutDisplayFunc(nextStep);
    glutMainLoop();
}