
#include "Chip8.h"



int main(int argc, char* argv[]) {
    Chip8 cpu;

    cpu.initialize();

    cpu.loadGame("data/TETRIS");

    while(true) {
        cpu.emulateCircle();
        cpu.render();
    }

    return 0;
}