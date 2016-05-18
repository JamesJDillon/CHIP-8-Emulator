#include <iostream>
#include <algorithm>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <SFML/Graphics.hpp>
#include <math.h>
#include <ncurses.h>
#include <unistd.h>

sf::RenderWindow window(sf::VideoMode(640, 320), "CHIP-8");

unsigned char chip8_fontset[80] =
{ 
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};






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
    unsigned char graphics[64][32];
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned short stack[16];
    unsigned short stack_pointer;
    unsigned char keys[16];
public:
    void setKey(char, int);
    void render();
    bool getDrawFlag();
    void clearScreen();
    void cycle();
    void print_mem();
    void load_ROM(std::string);
    void initialize();
};


void Chip8::setKey(char code, int set) {
    keys[code] = set;
    std::cout << "Key: " << std::hex << code << "set to: " << set << std::endl;
}

void Chip8::render() {
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            sf::RectangleShape rect;

            if (graphics[x][y] == 0) {
                rect.setFillColor(sf::Color::Black);
            } else {
                rect.setFillColor(sf::Color::White);
            }

            rect.setPosition(x * 10, y * 10);
            rect.setSize(sf::Vector2f(10, 10));
            window.draw(rect);
        }
    }

    drawFlag = false;
}



bool Chip8::getDrawFlag() {
    return drawFlag;
}


void Chip8::clearScreen() {

    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 32; y++) {
            graphics[x][y] = 0;
        }
    }

    std::cout << "Screen cleared." << std::endl;
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

    drawFlag = true;

    for (size_t i = 0; i < (sizeof(memory) / sizeof(memory[0])); i++) {
        memory[i] = 0;
        //std::cout << +memory[i] << std::endl;
    }

    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 32; y++) {
            graphics[x][y] = 0;
        }
    }

    for (size_t i = 0; i < (sizeof(registers) / sizeof(registers[0])); i++) {
        keys[i] = registers[i] = 0;
    } 

    for (size_t i = 0; i < (sizeof(stack) / sizeof(stack[0])); i++) {
        stack[i] = 0;
    } 

    for(int i = 0; i < 80; ++i) {
        memory[i] = chip8_fontset[i];       
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
                    --stack_pointer;
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
            ++stack_pointer;
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
            registers[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            program_counter += 2;
            //registers[(opcode & 0x0F00) >> 8]
        break;

        case 0x7000:
            registers[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
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

                if (((int)y + (int)yline) >= 32) {
                    break;
                }

                for(int xline = 0; xline < 8; xline++)
                {


                    if (((int)x + (int)xline) >= 64) {
                        break;
                    }

                    if((pixel & (0x80 >> xline)) != 0)
                    {
                        if ((graphics[(x + xline)][(y + yline)]) == 1) {
                            registers[0xF] = 1;
                        }

                        graphics[(x + xline)][(y + yline)] ^= 1;
                    }
                }
            }
                        
            drawFlag = true;            
            program_counter += 2;        
            break;
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
                    if (index + registers[(opcode & 0x0F00) >> 8] > 0xFFF) {
                        registers[0xF] = 1;
                    } else {
                        registers[0xF] = 0;
                    }

                    index += registers[(opcode & 0x0F00) >> 8];
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
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i) {
                        memory[index + i] = registers[i];   
                    }

                    index += ((opcode & 0x0F00) >> 8) + 1;
                    program_counter += 2;
                break;

                case 0x0065:
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i) {
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
    window.setFramerateLimit(10);



    Chip8 chip;
    chip.initialize();
    chip.load_ROM("maze.ch8");

    while (window.isOpen())
    {
        chip.cycle();

        if (chip.getDrawFlag()) {
            chip.render();
            window.display();
            //chip.clearScreen();
        }

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed) {
                std::cout << "Pressed " << std::endl;
                if (event.key.code == sf::Keyboard::Num1) {
                    chip.setKey(1, 1);
                } else if (event.key.code == sf::Keyboard::Num2) {
                    chip.setKey(0x2, 1);
                } else if (event.key.code == sf::Keyboard::Num3) {
                    chip.setKey(0x3, 1);
                } else if (event.key.code == sf::Keyboard::Num4) {
                    chip.setKey(0xC, 1);
                } else if (event.key.code == sf::Keyboard::Q) {
                    chip.setKey(0x4, 1);
                } else if (event.key.code == sf::Keyboard::W) {
                    chip.setKey(0x5, 1);
                } else if (event.key.code == sf::Keyboard::E) {
                    chip.setKey(0x6, 1);
                } else if (event.key.code == sf::Keyboard::R) {
                    chip.setKey(0xD, 1);
                } else if (event.key.code == sf::Keyboard::A) {
                    chip.setKey(0x7, 1);
                } else if (event.key.code == sf::Keyboard::S) {
                    chip.setKey(0x8, 1);
                } else if (event.key.code == sf::Keyboard::D) {
                    chip.setKey(0x9, 1);
                } else if (event.key.code == sf::Keyboard::F) {
                    chip.setKey(0xE, 1);
                } else if (event.key.code == sf::Keyboard::Z) {
                    chip.setKey(0xA, 1);
                } else if (event.key.code == sf::Keyboard::X) {
                    chip.setKey(0x0, 1);
                } else if (event.key.code == sf::Keyboard::C) {
                    chip.setKey(0xB, 1);
                } else if (event.key.code == sf::Keyboard::V) {
                    chip.setKey(0xF, 1);
                }
            } else if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::Num1) {
                    chip.setKey(0x1, 0);
                } else if (event.key.code == sf::Keyboard::Num2) {
                    chip.setKey(0x2, 0);
                } else if (event.key.code == sf::Keyboard::Num3) {
                    chip.setKey(0x3, 0);
                } else if (event.key.code == sf::Keyboard::Num4) {
                    chip.setKey(0xC, 0);
                } else if (event.key.code == sf::Keyboard::Q) {
                    chip.setKey(0x4, 0);
                } else if (event.key.code == sf::Keyboard::W) {
                    chip.setKey(0x5, 0);
                } else if (event.key.code == sf::Keyboard::E) {
                    chip.setKey(0x6, 0);
                } else if (event.key.code == sf::Keyboard::R) {
                    chip.setKey(0xD, 0);
                } else if (event.key.code == sf::Keyboard::A) {
                    chip.setKey(0x7, 0);
                } else if (event.key.code == sf::Keyboard::S) {
                    chip.setKey(0x8, 0);
                } else if (event.key.code == sf::Keyboard::D) {
                    chip.setKey(0x9, 0);
                } else if (event.key.code == sf::Keyboard::F) {
                    chip.setKey(0xE, 0);
                } else if (event.key.code == sf::Keyboard::Z) {
                    chip.setKey(0xA, 0);
                } else if (event.key.code == sf::Keyboard::X) {
                    chip.setKey(0x0, 0);
                } else if (event.key.code == sf::Keyboard::C) {
                    chip.setKey(0xB, 0);
                } else if (event.key.code == sf::Keyboard::V) {
                    chip.setKey(0xF, 0);
                }
            }
        }


        usleep(30000);
    }

    return 0;
}


