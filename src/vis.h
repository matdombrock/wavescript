#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "globals.h"
#include "util.h"
#define VIS_WIDTH 24
#define VIS_HEIGHT 16

void vis_clear() {
    printf("\033[2J");
    // Clear the console
    system("clear");
}

void vis_render(int vtick) {
    for (int y = 0; y < VIS_HEIGHT; y++) {
        for (int x = 0; x < VIS_WIDTH; x++) {
            int pos = vtick % (VIS_WIDTH * VIS_HEIGHT);
            if (pos == (y * VIS_WIDTH + x)) {
                printc(COLOR_RED, "▓▓");
            } else {
                printc(COLOR_YELLOW, "░░");
            }
        }         
        printf("\n");
    } 
}

void vis_loop() {
    int run = 1;
    int vtick = 0;
    // Configure terminal for non-blocking input
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Set non-blocking mode for stdin
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    while (run) {
        vis_clear();
        printf("🌘🌊 Tidalua 🌊🌒\n");
        vis_render(vtick);
        // Exit if the user presses 'Enter'
        int ch = getchar();
        if (ch != -1) _sys.keypress = ch;
        if (ch == '\n') {
            run = 0;
        }
        printf("🌊 %.2f | %d | %d | %c\n", _sys.speed, _sys.tick_num, vtick, _sys.keypress);
        printf("🌘 %s\n", _sys.filepath);
        vtick++;
        usleep(22 * 1000); // Sleep for 100 milliseconds to reduce CPU usage
    }
    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    // Restore blocking mode for stdin
    fcntl(STDIN_FILENO, F_SETFL, flags);
}
