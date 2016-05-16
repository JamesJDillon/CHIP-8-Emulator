#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
// #include <cstdio>
// #include <cstdlib>

class Chip8 {
private:
    unsigned short opcode;
    //Chip8 has 4k memory.
    unsigned char memory[4096];
    unsigned char registers[16];
    unsigned short index;
    unsigned short program_counter;
    //The screen is 2048 pixels.
    unsigned char graphics[64 * 32];
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned short stack[16];
    unsigned short stack_pointer;
    unsigned char keys[16];
public:
    void load_ROM(std::string);
    void initialize();
};

void Chip8::load_ROM(std::string filename) {

    std::ifstream rom;
    rom.open(filename.c_str(), std::ifstream::binary);


    while (rom >> std::hex >> a) {
        std::cout << c << std::endl;
    }

    rom.close();
    // std::cout << filename << std::endl;

    // FILE *rom;
    // rom = std::fopen(filename.c_str(), "r+b");

    // fseek(rom, 0, SEEK_END);
    // int size = ftell(rom);
    // fseek(rom, 0, SEEK_SET);


    // int c;
    // while ((c = std::fgetc(rom)) != EOF) {
    //     memory[]
    //     std::cout << std::hex << c << std::endl;
    // }
}


void Chip8::initialize() {
    program_counter = 0x200;
    opcode = 0;
    index = 0;
    stack_pointer = 0;

    for (size_t i = 0; i < (sizeof(memory) / sizeof(memory[0])); i++) {
        memory[i] = 0x0;
        //std::cout << +memory[i] << std::endl;
    }

    for (size_t i = 0; i < (sizeof(graphics) / sizeof(graphics[0])); i++) {
        graphics[i] = 0x0;
    }

    for (size_t i = 0; i < (sizeof(stack) / sizeof(stack[0])); i++) {
        stack[i] = 0x0;
    } 
}


int main()
{
    Chip8 chip;
    chip.initialize();
    chip.load_ROM("PONG");
}