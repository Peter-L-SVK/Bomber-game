/*
 * Bomber Game
 * Version: 1.0
 * Copyright (c) 2025 Peter Leukanič 
 * Under MIT License
 *
 */

#ifndef BOMBER_H
#define BOMBER_H

#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
 
#define DAMAGE_RADIUS 3
#define BOMB_KEY KEY_DOWN 
#define PAUSE_KEY 'p'
#define HELP_KEY 'h'
#define MACHINE_GUN_KEY ' '  // Spacebar for machine gun
#define MACHINE_GUN_RANGE 5
#define MAX_AMMO 17
#define FORTUNE_LENGTH 1024
#define SCROLL_DELAY 100000000L
#define SAFE_BOMB_HEIGHT 5
#define END_GAME_PAUSE 4
#define BOMBER_COLOR 1
#define BUILDING_COLOR 2
#define BOMB_COLOR 3
#define TEXT_COLOR 4
#define BACKGROUND_COLOR COLOR_WHITE
#define PINK_TEXT_COLOR 5
#define MAX_NAME_LENGTH 20
#define MAX_SCORES 10
#define HELP_COLOR 6 
#define HELP_DELAY 300000000L  // 300ms for visual effects

typedef struct {
    int x, y;
    int active;
} Bomb;

typedef struct {
    char name[MAX_NAME_LENGTH];
    int score;
} HighScore;

// Function declarations
void draw_city_with_delay(int world[], int cols, int lines);
void get_fortune_message(char* buffer);
void show_scrolling_message(const char* message, int scroll_pos, int row);
void show_help_screen(const char* fortune_msg, int* scroll_pos);
void show_menu();
void pause_game(const char* fortune_msg, int* scroll_pos);
void end_game_pause();
int compare_scores(const void* a, const void* b);
void show_all_scores();
void show_top_scores();
void save_score(const char* name, int score);
void load_scores(HighScore scores[]);
void display_scores(HighScore scores[]);
void ensure_score_file();
void get_player_name(char* name);
void show_info_screen(const char* fortune_msg, int* scroll_pos);

#endif
