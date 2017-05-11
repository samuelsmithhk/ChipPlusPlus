//
// Created by Samuel Smith on 5/6/17.
//

#ifndef CHIPPLUSPLUS_ROM_H
#define CHIPPLUSPLUS_ROM_H

#include <string>

class ROM {
private:
    std::string romFilePath;
    char* buffer;
    int bytePosition = 0;
public:
    ROM(std::string _romFilePath);
    void loadFile();
    unsigned char nextByte();
};


#endif //CHIPPLUSPLUS_ROM_H
