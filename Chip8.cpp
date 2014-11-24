#include "Chip8.h"


Chip8::Chip8(void) :
    memory(4096), V(16), stack(16),
    gfx(64*32), SCREEN_WIDTH(640), SCREEN_HEIGHT(480), chip8_fontset(0x50) {
    backgroundColor = 0xffffff;
    backgroundWindowColor = 0xffffff;
    color = 0x0;

    loadFont();
    initWindow();

}


Chip8::~Chip8(void) {}

void Chip8::loadFont() {
    unsigned char font[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    for(int i=0; i< 0x50; i++) {
        chip8_fontset[i] = font[i];
    }
}

void Chip8::initialize() {
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;


    // Clear display
    // Clear stack
    // Clear registers V0-VF
    // Clear memory

    for(int i=0; i<0x50; i++) {
        memory[i] = chip8_fontset[i];
    }

    for(int i=0; i<=0xf; i++) {
        key[i] = V[i] = 0;
    }

    // Reset timers
    delayTimer = soundTimer = 0;
}

void Chip8::loadGame(std::string filename) {

    filename += ".bin";

    std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
    std::streampos size;
    char *memblock;


    if(file.is_open()) {

        size = file.tellg();
        memblock = new char[size];

        file.seekg(0, std::ios::beg);
        file.read(memblock, size);
        file.close();

        for(int i = 0; i<size; i++) {
            memory[i + 0x200] = memblock[i];
        }

        delete []memblock;
    }
}

void Chip8::emulateCircle() {
    drawFlag = 0;
    // Fetch Opcode
    opcode = memory[pc] << 8 | memory[pc + 1];

    std::cout << "0x" << std::hex << opcode << std::endl;

    // Decode Opcode
    switch(opcode & 0xf000) {
        case 0x0000: {
            switch(opcode & 0xfff) {
                case 0x0e0:
                    for(int i=0; i<64*32; i++) {
                        gfx[i] = 0;
                    }
                    pc += 2;
                    break;
                case 0xee:
                    sp--;
                    pc = stack[sp];
                    pc += 2;
                    break;
                default:
                    std::cout << "Unknown opcode: 0x" << std::hex << opcode << std::endl;
                    break;
            }
            break;
        }
        case 0x1000: //0x1NNN
            pc = opcode & 0xfff;
            break;
        case 0x2000: //2NNN
            stack[sp] = pc;
            sp++;
            pc = opcode & 0x0fff;
            break;
        case 0x3000: //0x3XNN
            if(V[(opcode & 0x0f00) >> 8] == (opcode & 0xff)) {
                pc += 4;
            }
            else {
                pc += 2;
            }
            break;
        case 0x4000:
            if(V[(opcode & 0xf00)>>8] != (opcode & 0xff)) {
                pc += 4;
            }
            else
                pc += 2;
            break;
        case 0x5000:
            if(V[(opcode & 0xf00) >> 8] == V[(opcode & 0xf0) >> 4])
                pc += 4;
            else
                pc += 2;
            break;
        case 0x6000:
            V[(opcode & 0x0f00) >> 8] = opcode & 0x00ff;
            pc += 2;
            break;
        case 0x7000: // 0x7XNN
            V[(opcode & 0x0f00) >> 8] += opcode & 0x00ff;
            pc += 2;
            break;
        case 0x8000: {
            switch(opcode & 0xf) {
                // x = (opcode & 0xf00)>>8
                // y = (opcode & 0xf0)>>4
                case 0x0:
                    V[(opcode & 0xf00)>>8] = V[(opcode & 0xf0) >> 4];
                    pc += 2;
                    break;
                case 0x1:
                    V[(opcode & 0xf00)>>8] |= V[(opcode & 0xf0)>>4];
                    pc += 2;
                    break;
                case 0x2:
                    V[(opcode & 0xf00)>>8] &= V[(opcode & 0xf0)>>4];
                    pc += 2;
                    break;
                case 0x3:
                    V[(opcode & 0xf00)>>8] ^= V[(opcode & 0xf0)>>4];
                    pc += 2;
                    break;
                case 0x4:
                    V[(opcode & 0xf00)>>8] = V[(opcode & 0xf00)>>8] + V[(opcode & 0xf0)>>4];
                    if(V[(opcode & 0xf00)>>8] > 0xf) {
                        V[(opcode & 0xf00)>>8] &= 0xff;
                        V[0xf] = 1;
                    }
                    else
                        V[0xf] = 0;
                    pc += 2;
                    break;
                case 0x5:
                    if(V[(opcode & 0xf00)>>8] > V[(opcode & 0xf0)>>4]) {
                        V[0xf] = 1;
                    }
                    else
                        V[0xf] = 0;
                    V[(opcode & 0xf00)>>8] -= V[(opcode & 0xf0)>>4];
                    pc += 2;
                    break;
                case 0x6:
                    if((V[(opcode & 0xf00)>>8] & 1) == 1) {
                        V[0xf] = 1;
                    }
                    else
                        V[0xf] = 0;
                    V[(opcode & 0xf00)>>8] >> 1;
                    pc += 2;
                    break;
                case 0x7:
                    if(V[(opcode & 0xf0)>>4] > V[(opcode & 0xf00)>>8]) {
                        V[0xf] = 1;
                    }
                    else
                        V[0xf] = 0;
                    V[(opcode & 0xf00)>>8] = V[(opcode & 0xf0)>>4] - V[(opcode & 0xf00)>>8];
                    pc += 2;
                    break;
                case 0xE:
                    V[(opcode & 0xf00)>>8] = V[(opcode & 0xf00)>>8] << 1;
                    if(V[(opcode & 0xf00)>>8] >> 8 == 1) {
                        V[0xf] = 1;
                    }
                    else
                        V[0xf] = 0;
                    pc += 2;
                    break;
                default:
                    std::cout << "Unknown opcode: 0x" << std::hex << opcode << std::endl;
                    break;

            }
            break;
        }
        case 0x9000:  // 0x9xy0
            if(V[(opcode & 0x0f00) >> 8] != V[(opcode & 0x00f0) >> 4]) {
                pc += 4;
            }
            else pc += 2;
            break;
        case 0xA000: //ANNN
            I = opcode & 0xfff;
            pc += 2;
            break;
        case 0xB000:
            pc = V[0] + (opcode & 0xfff);
            break;
        case 0xC000:
            V[(opcode & 0x0f00) >> 8] = (rand() % 0xff) & (opcode & 0xff);
            pc += 2;
            break;
        case 0xD000: { //DXYN
            //width = 8
            // gfx[width * row + col]
            unsigned short height = opcode & 0xf; //height
            unsigned short x = V[(opcode & 0xf00) >> 8]; //x
            unsigned short y = V[(opcode & 0xf0) >> 4]; //y
            unsigned short row = 0, pixel = 0;
            V[0xf] = 0;

            for(int i = 0; i < height; i++) {
                pixel = memory[I + i];
                for(int k = 0; k < 8; k++) {
                    int z = ((y + i)  * 64 + (x + k));
                    if((pixel & (0x80 >> k)) == 0 || z > 2048) continue;
                    if(gfx[z] == 1)
                        V[0xf] = 1;
                    gfx[z] ^= 1;
                }
            }

            drawFlag = 1;
            pc += 2;
            break;
        }
        case 0xe000:
            switch(opcode & 0xff) {
                case 0x9E:
                    if(key[V[(opcode & 0xf00) >> 8]] == 1) {
                        pc += 2;
                    }
                    else {
                        pc += 4;
                    }
                    break;
                case 0xa1:
                    if(key[V[(opcode & 0xf00) >> 8]] == 0) {
                        pc += 4;
                    }
                    else {
                        pc += 2;
                    }
                    break;
                default:
                    std::cout << "Unknown opcode: 0x" << std::hex << opcode << std::endl;
                    break;
            }
            break;
        case 0xf000: {
            switch(opcode & 0x00ff) {
                case 0x07:
                    V[(opcode & 0x0f00) >> 8] = delayTimer;
                    pc += 2;
                    break;
                case 0x0A:
                    V[(opcode & 0xf00) >> 8];
                    for(int i=0; i<16; i++) {
                        if(key[i] == 1) {
                            V[(opcode & 0xf00) >> 8] = i;
                        }
                    }
                    pc += 2;
                    break;
                case 0x15:
                    delayTimer = V[(opcode & 0x0f00) >> 8];
                    pc += 2;
                    break;
                case 0x18:
                    //soundtimer
                    pc += 2;
                    break;
                case 0x1E:
                    I += V[(opcode & 0xf00) >> 8];
                    pc += 2;
                    break;
                case 0x29: { //0xFX29
                    unsigned short x = (opcode & 0x0f00) >> 8;
                    I = V[x];
                    pc += 2;
                    break;
                }
                case 0x33: { //0xFX33
                    memory[I] = (V[(opcode & 0x0f00) >> 8])/100;
                    memory[I + 1] = ((V[(opcode & 0x0f00) >> 8])/10) % 10;
                    memory[I + 2] = (V[(opcode & 0x0f00) >> 8]) % 10;
                    pc += 2;
                    break;
                }
                case 0x55: { //0xFX55
                    unsigned short x = (opcode & 0x0f00) >> 8;
                    for(int i=0; i<=x; i++) {
                        memory[I + i] = V[i];
                    }
                    pc += 2;
                    break;
                }
                case 0x65: { //0xFX65
                    unsigned short x = (opcode & 0x0f00) >> 8;
                    for(int i=0; i<=x; i++) {
                        V[i] = memory[I + i];
                    }
                    pc += 2;
                    break;
                }
                default:
                    std::cout << "Unknown opcode: 0x" << std::hex << opcode << std::endl;
                    break;
            }
            break;
        }

        default:
            std::cout << "Unknown opcode: 0x" << std::hex << opcode << std::endl;
            break;
    }

// Update timers
    if(delayTimer > 0) {
        delayTimer--;
    }
}

void Chip8::initWindow() {
    if( SDL_Init( SDL_INIT_VIDEO )) {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }
    else {
        gWindow = SDL_CreateWindow( "Chip8 - Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( gWindow == NULL ) {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        }
        else {
            gScreenSurface = SDL_GetWindowSurface( gWindow );
        }
    }

    renderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
}

void Chip8::render() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            return;
        }
        keyListener(e);
    }
    if(!drawFlag) return;
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);

    //SDL_RenderClear(renderer);

    drawGFX();


    SDL_RenderPresent(renderer);
}

void Chip8::keyListener(SDL_Event event) {
    if(event.type == SDL_KEYDOWN) {
        switch(event.key.keysym.sym) {
            case SDLK_1:
                key[0x1] = 1;
                break;
            case SDLK_2:
                key[0x2] = 1;
                break;
            case SDLK_3:
                key[0x3] = 1;
                break;
            case SDLK_4:
                key[0xc] = 1;
                break;
            case SDLK_q:
                key[0x4] = 1;
                break;
            case SDLK_w:
                key[0x5] = 1;
                break;
            case SDLK_e:
                key[0x6] = 1;
                break;
            case SDLK_r:
                key[0xd] = 1;
                break;
            case SDLK_a:
                key[0x7] = 1;
                break;
            case SDLK_s:
                key[0x8] = 1;
                break;
            case SDLK_d:
                key[0x9] = 1;
                break;
            case SDLK_f:
                key[0xe] = 1;
                break;
            case SDLK_y:
                key[0xa] = 1;
                break;
            case SDLK_x:
                key[0x0] = 1;
                break;
            case SDLK_c:
                key[0xb] = 1;
                break;
            case SDLK_v:
                key[0xf] = 1;
                break;
        }
    }
    if(event.type == SDL_KEYUP)
        switch(event.key.keysym.sym) {
            case SDLK_1:
                key[0x1] = 0;
                break;
            case SDLK_2:
                key[0x2] = 0;
                break;
            case SDLK_3:
                key[0x3] = 0;
                break;
            case SDLK_4:
                key[0xc] = 0;
                break;
            case SDLK_q:
                key[0x4] = 0;
                break;
            case SDLK_w:
                key[0x5] = 0;
                break;
            case SDLK_e:
                key[0x6] = 0;
                break;
            case SDLK_r:
                key[0xd] = 0;
                break;
            case SDLK_a:
                key[0x7] = 0;
                break;
            case SDLK_s:
                key[0x8] = 0;
                break;
            case SDLK_d:
                key[0x9] = 0;
                break;
            case SDLK_f:
                key[0xe] = 0;
                break;
            case SDLK_y:
                key[0xa] = 0;
                break;
            case SDLK_x:
                key[0x0] = 0;
                break;
            case SDLK_c:
                key[0xb] = 0;
                break;
            case SDLK_v:
                key[0xf] = 0;
                break;
        }
}


void Chip8::drawGFX() {
    //SDL_SetRenderDrawColor(renderer, 0xff, 0, 0, 0xff);

    int x = 0, y = 0;
    for(int i=0; i < gfx.size(); i++) {
        SDL_SetRenderDrawColor(renderer, FOREGROUND >> 16, (FOREGROUND >> 8) & 0xff, FOREGROUND & 0xff, 0xff);
        SDL_Rect rect;
        x = i%64;
        if(i % 64 == 0 && i != 0) {
            y++;
        }
        rect.x = x*10;
        rect.y = y*10;
        rect.w = rect.h = 10;
        if(gfx[i] == 0) {
            SDL_SetRenderDrawColor(renderer, BACKGROUND >> 16, (BACKGROUND >> 8) & 0xff, BACKGROUND & 0xff, 0xff);
        }
        SDL_RenderFillRect(renderer, &rect);
    }
}

