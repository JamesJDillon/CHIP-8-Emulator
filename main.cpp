#include <iostream>
#include <algorithm>
#include <string>
#include <cstdio>
#include <cstdlib>

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
    void print_mem();
    void load_ROM(std::string);
    void initialize();
};

void Chip8::print_mem() {
    for (size_t i = 0; i < (sizeof(memory) / sizeof(memory[0])); i++) {
        std::cout << std::hex << memory[i] << std::endl;
    }
}

void Chip8::load_ROM(std::string filename) {
    std::cout << filename << std::endl;

    FILE *rom = std::fopen(filename.c_str(), "rb");

    fseek(rom, 0, SEEK_END);
    long filesize = ftell(rom);
    rewind(rom);

    std::cout << "Filesize: " << filesize << std::endl;

    char *buff = (char*)malloc(sizeof(char) * filesize);

    if (buff == NULL) {
        std::cout << "ERROR ALLOCATING MEMORY!" << std::endl;
    }

    int result = fread(buff, 1, filesize, rom);
    if (result != filesize) {
        std::cout << "ERROR READING ROM!" << std::endl;
        return;
    }


    if (filesize < (4096 - 512)) {
        for (int i = 0; i < filesize; i++) {
            memory[i + 512] = buff[i];
        }
    }

    fclose(rom);
    free(buff);
}


void Chip8::initialize() {
    program_counter = 0x200;
    opcode = 0;
    index = 0;
    stack_pointer = 0;

    for (size_t i = 0; i < (sizeof(memory) / sizeof(memory[0])); i++) {
        memory[i] = 0;
        //std::cout << +memory[i] << std::endl;
    }

    for (size_t i = 0; i < (sizeof(graphics) / sizeof(graphics[0])); i++) {
        graphics[i] = 0;
    }

    for (size_t i = 0; i < (sizeof(stack) / sizeof(stack[0])); i++) {
        stack[i] = 0;
    } 
}


int main()
{
    Chip8 chip;
    chip.initialize();
    chip.load_ROM("PONG");
    //chip.print_mem();
}