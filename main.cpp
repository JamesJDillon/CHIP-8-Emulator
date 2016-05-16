#include <iostream>
#include <algorithm>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <math.h>
#include <SDL/SDL.h>

class Chip8 {
private:
    bool drawFlag;
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
    unsigned char *get_graphics();
    bool getDrawFlag();
    void clearScreen();
    void cycle();
    void print_mem();
    void load_ROM(std::string);
    void initialize();
};

unsigned char* Chip8::get_graphics() {
    return graphics;
}

bool Chip8::getDrawFlag() {
    return drawFlag;
}

void Chip8::clearScreen() {
    for (size_t i = 0; i < (sizeof(graphics) / sizeof(graphics[0])); i++) {
        graphics[i] = 0;
    }

    std::cout << "Screen cleared." << std::endl;
}

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

    sound_timer = 0;
    delay_timer = 0;

    for (size_t i = 0; i < (sizeof(memory) / sizeof(memory[0])); i++) {
        memory[i] = 0;
        //std::cout << +memory[i] << std::endl;
    }

    for (size_t i = 0; i < (sizeof(graphics) / sizeof(graphics[0])); i++) {
        graphics[i] = 0;
    }


    for (size_t i = 0; i < (sizeof(registers) / sizeof(registers[0])); i++) {
        registers[i] = 0;
    } 

    for (size_t i = 0; i < (sizeof(stack) / sizeof(stack[0])); i++) {
        stack[i] = 0;
    } 
}

void Chip8::cycle() {
    opcode = memory[program_counter] << 8 | memory[program_counter + 1];

    std::cout << std::hex << opcode << std::endl;
    switch(opcode & 0xF000) {

        case 0x0000:
            switch (opcode & 0x000F) {
                case 0x0000:
                    clearScreen();
                    drawFlag = true;
                    program_counter += 2;
                break;

                case 0x000E:
                    stack_pointer--;
                    program_counter = stack[stack_pointer];
                    program_counter += 2;
                break;

                default:
                    std::cout << "Unknown opcode: " << opcode << std::endl;
            }
        break;

        case 0x1000:
            program_counter = opcode & 0x0FFF;
        break;

        case 0x2000:
            stack[stack_pointer] = program_counter;
            stack_pointer++;
            program_counter = opcode & 0x0FFF;
        break;

        case 0x3000:
            if (registers[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
                program_counter += 4;
            } else {
                program_counter += 2;
            }
        break;

        case 0x4000:
            if (registers[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
                program_counter += 4;
            } else {
                program_counter += 2;
            }           
        break;

        case 0x5000:
            if (registers[(opcode & 0x0F00) >> 8] == registers[(opcode & 0x00F0) >> 4]) {
                program_counter += 4;
            } else {
                program_counter += 2;
            }
        break;

        case 0x6000:
            //std::cout << ((opcode & 0x0F00) >> 8) << std::endl;
            memory[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
            program_counter += 2;
            //registers[(opcode & 0x0F00) >> 8]
        break;

        case 0x7000:
            memory[(opcode & 0x0F00) >> 8] = memory[(opcode & 0x0F00) >> 8] + memory[(opcode & 0x00FF)];
            program_counter += 2;
        break;

        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0000:
                    registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x00F0) >> 4];
                    program_counter += 2;
                break;

                case 0x0001:
                    registers[(opcode & 0x0F00) >> 8] |= registers[(opcode & 0x00F0) >> 4];
                    program_counter += 2;                    
                break;

                case 0x0002:
                    registers[(opcode & 0x0F00) >> 8] &= registers[(opcode & 0x00F0) >> 4];
                    program_counter += 2;                    
                break;

                case 0x0003:
                    registers[(opcode & 0x0F00) >> 8] ^= registers[(opcode & 0x00F0) >> 4];
                    program_counter += 2;                    
                break;

                case 0x0004:
                    if(registers[(opcode & 0x00F0) >> 4] > (0xFF - registers[(opcode & 0x0F00) >> 8])) {
                        registers[0xF] = 1;
                    } else {
                        registers[0xF] = 0;  
                    }

                    registers[(opcode & 0x0F00) >> 8] += registers[(opcode & 0x00F0) >> 4];
                    program_counter += 2;                       
                break;

                case 0x0005:
                    if(registers[(opcode & 0x00F0) >> 4] > (0xFF - registers[(opcode & 0x0F00) >> 8])) {
                        registers[0xF] = 0;
                    } else {
                        registers[0xF] = 1;  
                    }

                    registers[(opcode & 0x0F00) >> 8] -= registers[(opcode & 0x00F0) >> 4];
                    program_counter += 2;                  
                break;

                case 0x0006:
                    registers[0xF] = registers[(opcode & 0x0F00) >> 8] & 0x1;
                    registers[(opcode & 0x0F00) >> 8] >>= 1;
                    program_counter += 2;                   
                break;

                case 0x0007:
                    if(registers[(opcode & 0x0F00) >> 8] > registers[(opcode & 0x00F0) >> 4]) {
                        registers[0xF] = 0;
                    } else {
                        registers[0xF] = 1;  
                    }

                    registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x00F0) >> 4] - registers[(opcode & 0x0F00) >> 8];
                    program_counter += 2;    
                break;

                case 0x000E:
                    registers[0xF] = registers[(opcode & 0x0F00) >> 8] >> 7;
                    registers[(opcode & 0x0F00) >> 8] <<= 1;
                    program_counter += 2;
                break;

                default:
                    std::cout << "Unknown opcode: " << std::hex << opcode << std::endl;
                    break;
            }
        break;

        case 0x9000:
            if (registers[(opcode & 0x0F00) >> 8] != registers[(opcode & 0x00F0) >> 4]) {
                program_counter += 4;
            } else {
                program_counter += 2;
            }
        break;

        case 0xA000:
            index = opcode & 0x0FFF;
            program_counter += 2;
        break;

        case 0xB000:
            program_counter = (opcode & 0x0FFF) + registers[0];
        break;

        case 0xC000:
            registers[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
            program_counter += 2;
        break;

        case 0xD000:
        {
            unsigned short x = registers[(opcode & 0x0F00) >> 8];
            unsigned short y = registers[(opcode & 0x00F0) >> 4];
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;

            registers[0xF] = 0;
            for (int yline = 0; yline < height; yline++)
            {
                pixel = memory[index + yline];
                for(int xline = 0; xline < 8; xline++)
                {
                    if((pixel & (0x80 >> xline)) != 0)
                    {
                        if(graphics[(x + xline + ((y + yline) * 64))] == 1)
                        {
                            registers[0xF] = 1;                                    
                        }
                        graphics[x + xline + ((y + yline) * 64)] ^= 1;
                    }
                }
            }
                        
            drawFlag = true;            
            program_counter += 2;
        }
        break;

        case 0xE000:
            switch(opcode & 0x00FF) {
                case 0x009E:
                    if (keys[registers[(opcode & 0x0F00) >> 8]] != 0) {
                        program_counter += 4;
                    } else {
                        program_counter += 2;
                    }
                break;

                case 0x00A1:
                    if (keys[registers[(opcode & 0x0F00) >> 8]] == 0) {
                        program_counter += 4;
                    } else {
                        program_counter += 2;
                    }                
                break;

                default:
                    std::cout << "Unknown opcode: " << std::hex << opcode << std::endl;
                    break;
            }
        break;

        case 0xF000:
            switch(opcode & 0x00FF) {
                case 0x0007:
                    registers[(opcode & 0x0F00) >> 8] = delay_timer;
                    program_counter += 2;
                break;

                case 0x000A:
                {
                    bool keyPressed = false;

                    for (int i = 0; i < 16; i++) {
                        if (keys[i] != 0) {
                            registers[(opcode & 0x0F00) >> 8] = i;
                            keyPressed = true;
                        }
                    }

                    if (keyPressed == false) {
                        return;
                    }

                    program_counter += 2;
                }
                break;

                case 0x0015:
                    delay_timer = registers[(opcode & 0x0F00) >> 8];
                    program_counter += 2;
                break;

                case 0x0018:
                    sound_timer = registers[(opcode & 0x0F00) >> 8];
                    program_counter += 2;
                break;

                case 0x001E:
                    index = index + registers[(opcode & 0x0F00) >> 8];
                    program_counter += 2;
                break;

                case 0x0029:
                    index = registers[(opcode & 0x0F00) >> 8] * 0x5;
                    program_counter += 2;
                break;

                case 0x0033:
                    memory[index] = registers[(opcode & 0x0F00) >> 8] / 100;
                    memory[index + 1] = (registers[(opcode & 0x0F00) >> 8] / 10) % 10;
                    memory[index + 2] = (registers[(opcode & 0x0F00) >> 8] % 100) % 10;                 
                    program_counter += 2;
                break;

                case 0x0055:
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
                        memory[index + i] = registers[i];   
                    }

                    index += ((opcode & 0x0F00) >> 8) + 1;
                    program_counter += 2;
                break;

                case 0x0065:
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
                        registers[i] = memory[index + i];   
                    }

                    index += ((opcode & 0x0F00) >> 8) + 1;
                    program_counter += 2;
                break;

                default:
                    std::cout << "Unknown opcode: " << std::hex << opcode << std::endl;
            }

        break;

        default:
            std::cout << "Unknown opcode: " << std::hex << opcode << std::endl;
            break;
    }

    if (delay_timer > 0) {
        delay_timer--;
    }

    if (sound_timer > 0) {
        if (sound_timer == 1) {
            std::cout << "BEEP!" << std::endl;
        }
        sound_timer--;
    }
}


int main()
{

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Quit();

    return 0;
    // sf::RenderWindow window(sf::VideoMode(500, 500), "CHIP-8");


    // Chip8 chip;
    // chip.initialize();
    // chip.load_ROM("PONG");

    // while (window.isOpen()) {
    //     sf::Event event;

    //     while (window.pollEvent(event)) {
    //         if (event.type == sf::Event::Closed) {
    //             window.close();
    //         }
    //     }

    //     if (chip.getDrawFlag()) {
    //         std::cout << "Draw called" << std::endl;
    //         unsigned char* gfx = chip.get_graphics();

    //         for (int i = 0; i < 64 * 32; i++) {
    //             int x = (i % 64);
    //             int y = (int)floor(i / 64);

    //             sf::RectangleShape rect(sf::Vector2f(x, y));
    //             rect.setSize(sf::Vector2f(20, 20));

    //             if (gfx[i] == 1) {
    //                 rect.setFillColor(sf::Color::White);
    //             } else {
    //                 rect.setFillColor(sf::Color::White);
    //             }
    //             // sf::RectangleShape rectangle(sf::Vector2f(120, 50));

    //             // // change the size to 100x100
    //             // rectangle.setSize(sf::Vector2f(100, 100));
    //             window.draw(rect);
    //         }
    //         //drawGraphics();
    //     }

    //     chip.cycle();

    // }


    // return 0;

}