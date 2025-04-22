#define _POSIX_C_SOURCE 200201L

#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <time.h>
#include <unistd.h>  // For sleep()

typedef struct {
    int x, y;
    int active;
} Bomb;

#define DAMAGE_RADIUS 2  // 1 = 3-block wide explosion

void pause_game(int seconds) {
    timeout(-1);  // Make getch() blocking
    halfdelay(seconds * 10);  // 10 = 1 second units
    getch();
    nodelay(stdscr, TRUE);  // Restore non-blocking input
}

int main() {
    srand(time(NULL));
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);

    struct timespec ts_frame = { .tv_sec = 0, .tv_nsec = 50000000L };
    struct timespec ts_bomb  = { .tv_sec = 0, .tv_nsec = 20000000L };

    int world[COLS];
    for (int i = 0; i < COLS; i++) {
        world[i] = rand() % (LINES / 3) + 1;
    }

    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < world[x]; y++) {
            mvprintw(LINES - y - 1, x, "#");
        }
    }
    refresh();

    int bomber_x = 0;
    int bomber_y = 1;
    int bomber_dx = 1;
    int score = 0;
    Bomb bomb = {0};
    int game_over = 0;
    int win = 0;

    while (!game_over && !win) {
        erase();

        int city_destroyed = 1;
        for (int x = 0; x < COLS; x++) {
            if (world[x] > 0) {
                city_destroyed = 0;
                for (int y = 0; y < world[x]; y++) {
                    mvprintw(LINES - y - 1, x, "#");
                }
            }
        }
        if (city_destroyed) {
            win = 1;
            break;
        }

        bomber_x += bomber_dx;
        if (bomber_x >= COLS - 4 || bomber_x <= 0) {
            bomber_dx *= -1;
            bomber_y++;
        }

        mvprintw(bomber_y, bomber_x, bomber_dx > 0 ? "^==-" : "-==^");

        if (bomber_y >= LINES - world[bomber_x] - 1) {
            game_over = 1;
            break;
        }

        if (bomb.active) {
            mvprintw(bomb.y, bomb.x, "*");
            bomb.y++;
            
            if (bomb.y >= LINES - world[bomb.x] - 1) {
                for (int dx = -DAMAGE_RADIUS; dx <= DAMAGE_RADIUS; dx++) {
                    int target_x = bomb.x + dx;
                    if (target_x >= 0 && target_x < COLS && world[target_x] > 0) {
                        world[target_x]--;
                        score += 10;
                    }
                }
                bomb.active = 0;
            }
            nanosleep(&ts_bomb, NULL);
        }

        int ch = getch();
        if (ch == ' ' && !bomb.active) {
            bomb.x = bomber_x + (bomber_dx > 0 ? 2 : 1);
            bomb.y = bomber_y + 1;
            bomb.active = 1;
        }

        mvprintw(0, 0, "Score: %d", score);
        refresh();
        nanosleep(&ts_frame, NULL);

        if (ch == 'q') break;
    }

    // Game end screen
    erase();
    mvprintw(LINES/2, COLS/2-4, win ? "YOU WIN!" : "GAME OVER!");
    mvprintw(LINES/2+1, COLS/2-8, "Score: %d", score);
    refresh();
    
    pause_game(3);  // Pause for 3 seconds before exit

    endwin();
    return 0;
}
