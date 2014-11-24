#ifndef _CHIP8_
#define _CHIP8_

#include <string.h>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <SDL.h>



/*
    0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
    0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
    0x200-0xFFF - Program ROM and work RAM
*/

enum chip8Flags {
    COLLISION = 0x1
};

enum chip8ColorFlags {
    RED = 0xff0000,
    GREEN = 0xff00,
    BLUE = 0xff,
    BLACK = 0x0,
    WHITE = 0xffffff,
    BACKGROUND = BLACK,
    FOREGROUND = WHITE
};

class Chip8 {
private:
    unsigned short opcode;
    std::vector<unsigned char> memory;
    std::vector<unsigned char> chip8_fontset;
    std::vector<unsigned char> V;

    unsigned short I, pc;
    unsigned short drawFlag;

    std::vector<unsigned char> gfx; //64*32


    std::vector<unsigned short> stack;
    unsigned short sp;

    unsigned char key[16];
    unsigned char delayTimer;
    unsigned char soundTimer;

    SDL_Window* gWindow;
    SDL_Surface* gScreenSurface;
    SDL_Renderer* renderer;

    SDL_Surface* gScreen;

    const int SCREEN_WIDTH;// = 640;
    const int SCREEN_HEIGHT;// = 480;

    int backgroundColor;
    int backgroundWindowColor;
    int color;

    void drawGFX();
    void loadFont();
    void keyListener(SDL_Event);


public:
    Chip8(void);
    ~Chip8(void);

    void initialize();
    void loadGame(std::string);
    void emulateCircle();
    void initWindow();
    void render();
};

#endif

