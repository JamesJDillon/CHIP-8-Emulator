#include <iostream>

class Chip8 {
private:
    unsigned short opcode;
    unsigned char memory[4096];
public:
    void set_opcode(short);
    short get_opcode();
};

void Chip8::set_opcode(short i) {
    opcode = i;
}

short Chip8::get_opcode() {
    return opcode;
}

int main()
{
    Chip8 chip;
    chip.set_opcode(0xFFF);
    std::cout << chip.get_opcode() << std::endl;
}