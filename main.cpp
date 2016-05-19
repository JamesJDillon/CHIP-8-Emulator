#include <iostream>
#include <string>
#include <SFML/Graphics.hpp>

//For coloring the error outputs.
const std::string error("\033[0;31m");
const std::string reset("\033[0m");

sf::RenderWindow window(sf::VideoMode(640, 320), "CHIP-8");

//The chip8 fontset array I took from some other chip8 emulator.
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
    bool            drawFlag;
    unsigned short  opcode;
    //Chip8 has 4k memory.
    unsigned char   memory[4096];
    unsigned char   registers[16];
    unsigned short  index;
    unsigned short  program_counter;
    //The screen is 2048 (64 * 32) pixels.
    unsigned char   graphics[64][32];
    unsigned char   delay_timer;
    unsigned char   sound_timer;
    unsigned short  stack[16];
    unsigned short  stack_pointer;
    unsigned char   keys[16];
public:
    void keyRelease(sf::Event);
    void keyPress(sf::Event);
    //Used to set keys[char] = int
    //draws the screen.
    void render();
    bool getDrawFlag();
    void clearScreen();
    void cycle();
    bool load_ROM(std::string);
    void initialize();
};


void Chip8::keyPress(sf::Event key) {
    if (key.key.code == sf::Keyboard::Num1) { keys[0x1] = 1; }
    if (key.key.code == sf::Keyboard::Num2) { keys[0x2] = 1; }
    if (key.key.code == sf::Keyboard::Num3) { keys[0x3] = 1; }
    if (key.key.code == sf::Keyboard::Num4) { keys[0xC] = 1; }

    if (key.key.code == sf::Keyboard::Q)    { keys[0x4] = 1; }
    if (key.key.code == sf::Keyboard::W)    { keys[0x5] = 1; }
    if (key.key.code == sf::Keyboard::E)    { keys[0x6] = 1; }
    if (key.key.code == sf::Keyboard::R)    { keys[0xD] = 1; }

    if (key.key.code == sf::Keyboard::A)    { keys[0x7] = 1; }
    if (key.key.code == sf::Keyboard::S)    { keys[0x8] = 1; }
    if (key.key.code == sf::Keyboard::D)    { keys[0x9] = 1; }
    if (key.key.code == sf::Keyboard::F)    { keys[0xE] = 1; }

    if (key.key.code == sf::Keyboard::Z)    { keys[0xA] = 1; }
    if (key.key.code == sf::Keyboard::X)    { keys[0x0] = 1; }
    if (key.key.code == sf::Keyboard::C)    { keys[0xB] = 1; }
    if (key.key.code == sf::Keyboard::V)    { keys[0xF] = 1; }
}   

void Chip8::keyRelease(sf::Event key) {
    if (key.key.code == sf::Keyboard::Num1) { keys[0x1] = 0; }
    if (key.key.code == sf::Keyboard::Num2) { keys[0x2] = 0; }
    if (key.key.code == sf::Keyboard::Num3) { keys[0x3] = 0; }
    if (key.key.code == sf::Keyboard::Num4) { keys[0xC] = 0; }

    if (key.key.code == sf::Keyboard::Q)    { keys[0x4] = 0; }
    if (key.key.code == sf::Keyboard::W)    { keys[0x5] = 0; }
    if (key.key.code == sf::Keyboard::E)    { keys[0x6] = 0; }
    if (key.key.code == sf::Keyboard::R)    { keys[0xD] = 0; }

    if (key.key.code == sf::Keyboard::A)    { keys[0x7] = 0; }
    if (key.key.code == sf::Keyboard::S)    { keys[0x8] = 0; }
    if (key.key.code == sf::Keyboard::D)    { keys[0x9] = 0; }
    if (key.key.code == sf::Keyboard::F)    { keys[0xE] = 0; }

    if (key.key.code == sf::Keyboard::Z)    { keys[0xA] = 0; }
    if (key.key.code == sf::Keyboard::X)    { keys[0x0] = 0; }
    if (key.key.code == sf::Keyboard::C)    { keys[0xB] = 0; }
    if (key.key.code == sf::Keyboard::V)    { keys[0xF] = 0; }
}


void Chip8::render() {
    //For the entire height/width of the display:
    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 32; y++) {
            //Create a rectangle.
            sf::RectangleShape rect;

            //If the value at this location is 0, color the rect black. Else color white.
            if (graphics[x][y] == 0) {
                rect.setFillColor(sf::Color::Black);
            } else {
                rect.setFillColor(sf::Color::White);
            }

            //Because the screen is 640x320, times position by 10 (as rect will be 10 pixels long.)
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
}

bool Chip8::load_ROM(std::string filename) {
    //Open the file in "read and binary" mode.
    FILE *rom = std::fopen(filename.c_str(), "rb");

    if (rom == NULL) {
        return false;
    }

    //Get the size of the file.
    fseek(rom, 0, SEEK_END);
    long filesize = ftell(rom);
    rewind(rom);

    std::cout << "Filesize: " << filesize << std::endl;

    //Mate a char buffer the size of the file.
    char *buff = (char*)malloc(sizeof(char) * filesize);

    //If the buffer is null, malloc messed up somehow.
    if (buff == NULL) {
        std::cout << "ERROR ALLOCATING MEMORY!" << std::endl;
        return false;
    }

    //Read data from rom stream in to buff.
    int result = fread(buff, 1, filesize, rom);
    if (result != filesize) {
        std::cout << "ERROR READING ROM!" << std::endl;
        return false;
    }


    //If filesize is less than 4096 (minus the 512 bytes that the rom can't be stored in)
    if (filesize < (4096 - 512)) {
        for (int i = 0; i < filesize; i++) {
            memory[i + 512] = buff[i];
        }
    } else {
        std::cout << "ROM too large!" << std::endl;
        return false;
    }

    fclose(rom);
    free(buff);

    return true;
}


void Chip8::initialize() {
    //0x000 through 0x1FF is reserved for the interpreter.
    program_counter = 0x200;
    opcode          = 0;
    index           = 0;
    stack_pointer   = 0;

    sound_timer     = 0;
    delay_timer     = 0;

    drawFlag        = true;

    //Initialize every array to zero.
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
    //Fetch the opcode.
    opcode = memory[program_counter] << 8 | memory[program_counter + 1];

    //std::cout << std::hex << opcode << std::endl;

    switch(opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x000F) {
                //0x00E0. Clear the displays.
                case 0x0000:
                    clearScreen();
                    drawFlag = true;
                    program_counter += 2;
                break;

                //0x00EE. Return from a subroutine.
                case 0x000E:
                    --stack_pointer;
                    program_counter = stack[stack_pointer];
                    program_counter += 2;
                break;

                default:
                    std::cout << "Unknown opcode: " << opcode << std::endl;
            }
        break;

        //0x1nnn. Jump to location nnn.
        case 0x1000:
            program_counter = opcode & 0x0FFF;
        break;

        //0x2nnn. Calls subroutine at nnn.
        case 0x2000:
            stack[stack_pointer] = program_counter;
            stack_pointer++;
            program_counter = opcode & 0x0FFF;
        break;

        //0x3xkk. If registers[x] == kk then skip next instruction.
        case 0x3000:
            if (registers[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
                program_counter += 4;
            } else {
                program_counter += 2;
            }
        break;

        //0x4xkk. If registers[x] != kk, then skip next instruction.
        case 0x4000:
            if (registers[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
                program_counter += 4;
            } else {
                program_counter += 2;
            }           
        break;

        //0x5xy0. If registers[x] == registers[y], skip next instruction.
        case 0x5000:
            if (registers[(opcode & 0x0F00) >> 8] == registers[(opcode & 0x00F0) >> 4]) {
                program_counter += 4;
            } else {
                program_counter += 2;
            }
        break;

        //0x6xkk. Puts kk in to register x. registers[x] == kk;
        case 0x6000:
            registers[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            program_counter += 2;
        break;

        //0x7xkk. "Set Vx = Vx + kk". Adds kk to register[x], then stores result in registers[x].
        case 0x7000:
            registers[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            program_counter += 2;
        break;

        //0x8000. There are 9 instructions that begin with 0x8000.
        case 0x8000:
            //Get last byte. 
            switch (opcode & 0x000F) {
                //0x8xy0. Set registers[x] = registers[y].
                case 0x0000:
                    registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x00F0) >> 4];
                    program_counter += 2;
                break;

                //0x8xy1. Set register[x] = register[x] |(OR) register[y].
                case 0x0001:
                    registers[(opcode & 0x0F00) >> 8] |= registers[(opcode & 0x00F0) >> 4];
                    program_counter += 2;                    
                break;

                //0x8xy2. Same as before, except this time AND.
                case 0x0002:
                    registers[(opcode & 0x0F00) >> 8] &= registers[(opcode & 0x00F0) >> 4];
                    program_counter += 2;                    
                break;

                //0x8xy3. register[x] = register[x] XOR register[y]
                case 0x0003:
                    registers[(opcode & 0x0F00) >> 8] ^= registers[(opcode & 0x00F0) >> 4];
                    program_counter += 2;                    
                break;

                //0x8xy4. register[x] += register[y], set register[0xF] to carry.
                case 0x0004:
                    if(registers[(opcode & 0x00F0) >> 4] > (0xFF - registers[(opcode & 0x0F00) >> 8])) {
                        registers[0xF] = 1;
                    } else {
                        registers[0xF] = 0;  
                    }

                    registers[(opcode & 0x0F00) >> 8] += registers[(opcode & 0x00F0) >> 4];
                    program_counter += 2;                       
                break;

                //0x8xy5. register[x] -= register[y], set register[0xF] to NOT carry.
                case 0x0005:
                    if(registers[(opcode & 0x00F0) >> 4] > (0xFF - registers[(opcode & 0x0F00) >> 8])) {
                        registers[0xF] = 0;
                    } else {
                        registers[0xF] = 1;  
                    }

                    registers[(opcode & 0x0F00) >> 8] -= registers[(opcode & 0x00F0) >> 4];
                    program_counter += 2;                  
                break;

                //0x8xy6. "Set register[x] = register[x] SHR 1."
                case 0x0006:
                    registers[0xF] = registers[(opcode & 0x0F00) >> 8] & 0x1;
                    registers[(opcode & 0x0F00) >> 8] >>= 1;
                    program_counter += 2;                   
                break;

                //0x8xy7. "Set register[x] = register[y] - register[x], set register[F] = NOT borrow."
                case 0x0007:
                    if(registers[(opcode & 0x0F00) >> 8] > registers[(opcode & 0x00F0) >> 4]) {
                        registers[0xF] = 0;
                    } else {
                        registers[0xF] = 1;  
                    }

                    registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x00F0) >> 4] - registers[(opcode & 0x0F00) >> 8];
                    program_counter += 2;    
                break;

                //0x8xy8. "Set register[x] = register[x] SHL 1."
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

        //0x9xy0. If register[x] != register[y], skip the instruction.
        case 0x9000:
            if (registers[(opcode & 0x0F00) >> 8] != registers[(opcode & 0x00F0) >> 4]) {
                program_counter += 4;
            } else {
                program_counter += 2;
            }
        break;

        //0xAnnn. Set index = nnn.
        case 0xA000:
            index = opcode & 0x0FFF;
            program_counter += 2;
        break;

        //0xBnnn. Jump to location nnn + register[0].
        case 0xB000:
            program_counter = (opcode & 0x0FFF) + registers[0];
        break;

        //0xCxkk. Set register[x] = random byte AND kk.
        case 0xC000:
            registers[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
            program_counter += 2;
        break;

        /*
        0xDxyn.
        "Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.

        The interpreter reads n bytes from memory, starting at the address stored in I. 
        These bytes are then displayed as sprites on screen at coordinates (Vx, Vy). 
        Sprites are XORed onto the existing screen. 
        If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0. 
        If the sprite is positioned so part of it is outside the coordinates of the display, 
        it wraps around to the opposite side of the screen."
        */
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

        //Two operations start with 0xE000.
        case 0xE000:
            switch(opcode & 0x00FF) {
                //0xEx9E. Skip next instruction if key with value of register[x] is pressed.
                case 0x009E:
                    if (keys[registers[(opcode & 0x0F00) >> 8]] != 0) {
                        program_counter += 4;
                    } else {
                        program_counter += 2;
                    }
                break;

                //0xExA1. Skip next instruction of key with value of register[x] is not pressed.
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

        //9 operations begin with 0xF000.
        case 0xF000:
            switch(opcode & 0x00FF) {
                //0xFx07. Set register[x] = delay timer value.
                case 0x0007:
                    registers[(opcode & 0x0F00) >> 8] = delay_timer;
                    program_counter += 2;
                break;

                //0xFx0A. Waits for keypress, stores that value in register[x].
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

                //0xFx15. Sets delay timer = register[x].
                case 0x0015:
                    delay_timer = registers[(opcode & 0x0F00) >> 8];
                    program_counter += 2;
                break;

                //0xFx18. Sets sound time = register[x].
                case 0x0018:
                    sound_timer = registers[(opcode & 0x0F00) >> 8];
                    program_counter += 2;
                break;

                //0xFx1E. Set index += register[x].
                case 0x001E:
                    if (index + registers[(opcode & 0x0F00) >> 8] > 0xFFF) {
                        registers[0xF] = 1;
                    } else {
                        registers[0xF] = 0;
                    }

                    index += registers[(opcode & 0x0F00) >> 8];
                    program_counter += 2;
                break;

                //0xFx29. "Set I = location of sprite for digit Vx."
                case 0x0029:
                    index = registers[(opcode & 0x0F00) >> 8] * 0x5;
                    program_counter += 2;
                break;

                //0xFx33. "Store BCD representation of Vx in memory locations I, I+1, and I+2."
                case 0x0033:
                    memory[index] = registers[(opcode & 0x0F00) >> 8] / 100;
                    memory[index + 1] = (registers[(opcode & 0x0F00) >> 8] / 10) % 10;

                    memory[index + 2] = (registers[(opcode & 0x0F00) >> 8] % 100) % 10;                 
                    program_counter += 2;
                break;

                //0xFx55. Stores registers[0] through register[x] in memory (starting at location index.)
                case 0x0055:
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i) {
                        memory[index + i] = registers[i];   
                    }

                    index += ((opcode & 0x0F00) >> 8) + 1;
                    program_counter += 2;
                break;

                //0xFx65. Read registers[0] through register[x] from memory starting at location index.
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

    //Decremet the delay_timer after every cycle.
    if (delay_timer > 0) {
        delay_timer--;
    }

    if (sound_timer > 0) {
        if (sound_timer == 1) {
            std::cout << "\aBEEP!" << std::endl;
        }
        sound_timer--;
    }
}



int main(int argc, char* argv[])
{

    if (argc < 2) {
        std::cerr << error << "Error!" << reset << std::endl;
        std::cerr << error << "Usage:    ./main filename" << reset << std::endl;
        std::cerr << error << "Example:  ./main PONG" << reset << std::endl;
        return 1;
    }

    Chip8 chip;
    chip.initialize();

    if (!chip.load_ROM(argv[1])) {
        std::cerr << error << "Invalid ROM filename. Please try again." << reset << std::endl;
        return 1;
    }


    while (window.isOpen())
    {
        chip.cycle();

        //The screen needs drawn.
        if (chip.getDrawFlag()) {
            chip.render();
            window.display();
        }

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed) {
                chip.keyPress(event);
            } else if (event.type == sf::Event::KeyReleased) {
                chip.keyRelease(event);
            }
        }
    }

    return 0;
}


