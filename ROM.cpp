//
// Created by Samuel Smith on 5/6/17.
//
#include <fstream>

#include "ROM.h"

using namespace std;

ROM::ROM(std::string _romFilePath) : romFilePath(_romFilePath) {}

void ROM::loadFile() {
    ifstream romFile;
    romFile.open(romFilePath, ios::binary);
    buffer = new char[3232];
    romFile.read(buffer, 3232);
    romFile.close();
}

char ROM::nextByte() {
    char toReturn = buffer[bytePosition];
    bytePosition++;
    return toReturn;
}