#include <iostream>

#include "ROM.h"
#include "RAM.h"

using namespace std;

ROM* gameROM = nullptr;
RAM* systemRAM = nullptr;

int main(int argc, char* argv[]) {

    if (argc != 2) {
        cerr << "Incorrect program usage. Correct usage: chippp <rom path>" << endl;
    }

    string romPath = argv[1];
    gameROM = new ROM(romPath);
    gameROM->loadFile();
    systemRAM = new RAM;
    systemRAM->loadROMIntoMemory(gameROM);

    for (int i = 512; i < 4096; i += 2) {
        short opcode = systemRAM->getOpcode(i);
        cout << hex << opcode << endl;
    }

    return 0;
}