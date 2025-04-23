/*
 * Bomber Game
 * Version: 1.0
 * Copyright (c) 2025 Peter Leukanič 
 * Under MIT License
 *
 */

#include "bomber.h"
#include <stdio.h>
#include <stdlib.h>

#define SCORE_FILE "bomber.scores"

void show_info_screen(const char* fortune_msg, int* scroll_pos) {
    flushinp();
    nodelay(stdscr, TRUE);
    
    // Slower animation (200ms per frame)
    struct timespec ts = {.tv_sec = 0, .tv_nsec = 200000000L}; 
    
    // Calculate subtitle properties
    char* subtitles[] = {
        "Bombs destroy 3x3 area",
        "Machine gun destroys 5 blocks",
        "Watch your altitude!",
        "Good luck pilot!",
        NULL
    };
    int total_subtitles = 0;
    while (subtitles[total_subtitles] != NULL) total_subtitles++;
    
    int animating = 1;
    
    while (animating) {
        clear();
        
        /* Draw cow art and static text */
        int start_row = 2;
        char* cow_lines[] = {
            "   \\_/",
            "  (o o)",
            " /  V  \\", 
            "/(  _  )\\",
            "  ^^ ^^"
        };
        for (int i = 0; i < 5; i++) {
            mvprintw(start_row + i, COLS/2 - 3, "%s", cow_lines[i]);
        }
        
        start_row += 7;
        mvprintw(start_row++, COLS/2 - 10, "=== BOMBER GAME ===");
        mvprintw(start_row++, COLS/2 - 12, "Fly your bomber plane");
        mvprintw(start_row++, COLS/2 - 12, "Destroy enemy buildings");
        mvprintw(start_row++, COLS/2 - 12, "Avoid crashing!");
        start_row++;
        
        /* Improved subtitle scrolling */
        for (int i = 0; subtitles[i] != NULL; i++) {
            // Calculate position with smooth upward scroll
            int row_pos = (LINES + 2) + i - (*scroll_pos % (LINES + total_subtitles + 10));
            
            // Only draw if visible on screen
            if (row_pos >= 0 && row_pos < LINES) {
                mvprintw(row_pos, COLS/2 - strlen(subtitles[i])/2, "%s", subtitles[i]);
            }
        }
        
        // Show scrolling fortune message at bottom
        show_scrolling_message(fortune_msg, *scroll_pos, LINES-1);
        
        mvprintw(LINES-2, COLS/2 - 15, "Press any key to return");
        refresh();
        
        if (getch() != ERR) {
            animating = 0;
        }
        
        (*scroll_pos)++;
        nanosleep(&ts, NULL);
    }
    
    nodelay(stdscr, FALSE);
}

void ensure_score_file() {
  // Initialize with default scores if file doesn't exist or is empty
  FILE* file = fopen(SCORE_FILE, "rb");
  if (!file) {
    // File doesn't exist - create with default scores
    HighScore default_scores[MAX_SCORES];
    for (int i = 0; i < MAX_SCORES; i++) {
      strcpy(default_scores[i].name, "Player");
      default_scores[i].score = 0;
    }
    
    file = fopen(SCORE_FILE, "wb");
    if (file) {
      fwrite(default_scores, sizeof(HighScore), MAX_SCORES, file);
      fclose(file);
    }
  } else {
    // Check if file is empty
    fseek(file, 0, SEEK_END);
    if (ftell(file) == 0) {
      fclose(file);
      // File exists but is empty - initialize it
      HighScore default_scores[MAX_SCORES];
      for (int i = 0; i < MAX_SCORES; i++) {
	strcpy(default_scores[i].name, "Player");
	default_scores[i].score = 0;
      }
      
      file = fopen(SCORE_FILE, "wb");
      if (file) {
	fwrite(default_scores, sizeof(HighScore), MAX_SCORES, file);
	fclose(file);
      }
    } else {
      fclose(file);
    }
  }
}

void get_player_name(char* name) {
  clear();  // Clear the menu screen first
  refresh();
  
  echo(); // Enable echo for name input
  curs_set(1); // Show cursor
  
  // Center the prompt better
  int prompt_y = LINES/2;
  int prompt_x = COLS/2 - 15;
  
  // Draw a clean box for input
  if (has_colors()) {
    attron(COLOR_PAIR(TEXT_COLOR));
  }
  mvprintw(prompt_y-2, prompt_x, "-----------------------------");
  mvprintw(prompt_y-1, prompt_x, "| ENTER YOUR NAME:          |");
  mvprintw(prompt_y,   prompt_x, "|                           |");
  mvprintw(prompt_y+1, prompt_x, "-----------------------------");
  
  // Move cursor inside the box
  move(prompt_y, prompt_x + 2);
  if (has_colors()) {
    attroff(COLOR_PAIR(TEXT_COLOR));
  }
  
  // Get input
  do {
    getnstr(name, MAX_NAME_LENGTH-1);
  } while (strlen(name) == 0);  // Ensure non-empty name
  
  noecho(); // Disable echo again
  curs_set(0); // Hide cursor
}

void load_scores(HighScore scores[]) {
  FILE* file = fopen(SCORE_FILE, "rb");
  if (file) {
    // Read exactly MAX_SCORES records
    size_t read = fread(scores, sizeof(HighScore), MAX_SCORES, file);
    fclose(file);
    
    // Initialize any empty slots (shouldn't happen if ensure_score_file worked)
    for (size_t i = read; i < MAX_SCORES; i++) {
      strcpy(scores[i].name, "Player");
      scores[i].score = 0;
    }
    
    // Sort the scores
    qsort(scores, MAX_SCORES, sizeof(HighScore), compare_scores);
  } else {
    // This shouldn't happen if ensure_score_file worked
    for (int i = 0; i < MAX_SCORES; i++) {
      strcpy(scores[i].name, "Player");
      scores[i].score = 0;
    }
  }
}

// Comparison function for qsort
int compare_scores(const void* a, const void* b) {
  HighScore* scoreA = (HighScore*)a;
  HighScore* scoreB = (HighScore*)b;
  return (scoreB->score - scoreA->score); // Descending order
}

void save_score(const char* name, int score) {
  HighScore scores[MAX_SCORES];
  load_scores(scores);  // Load current scores
  
  // Check if score qualifies for high score list
  if (score <= scores[MAX_SCORES-1].score) {
    return;  // Score too low, don't save
  }
  
  // Create new score entry
  HighScore newScore;
  strncpy(newScore.name, name, MAX_NAME_LENGTH-1);
  newScore.name[MAX_NAME_LENGTH-1] = '\0';
  newScore.score = score;
  
  // Find position to insert
  int pos = MAX_SCORES - 1;
  while (pos > 0 && score > scores[pos-1].score) {
    pos--;
  }
  
  // Shift scores down
  for (int i = MAX_SCORES-1; i > pos; i--) {
    scores[i] = scores[i-1];
  }
  
  // Insert new score
  scores[pos] = newScore;
  
  // Save all scores back to file
  FILE* file = fopen(SCORE_FILE, "wb");
  if (file) {
    fwrite(scores, sizeof(HighScore), MAX_SCORES, file);
    fclose(file);
  }
}

void display_scores(HighScore scores[]) {
  mvprintw(6, COLS/2-10, "=== TOP 10 SCORES ===");
  for (int i = 0; i < MAX_SCORES; i++) {
    mvprintw(8 + i, COLS/2-10, "%2d. %-20s %5d", i+1, scores[i].name, scores[i].score);
  }
}

void show_top_scores() {
  HighScore scores[MAX_SCORES];
  load_scores(scores);
  
  clear();
  mvprintw(0, COLS/2-10, "=== BOMBER GAME ===");
  display_scores(scores);
  mvprintw(LINES-2, COLS/2-15, "Press any key to return");
  refresh();
  
  timeout(-1); // Blocking wait
  getch();
  timeout(0); // Back to non-blocking
}

void show_all_scores() {
  FILE* file = fopen(SCORE_FILE, "rb");
  if (file) {
    clear();
    mvprintw(0, COLS/2-10, "=== ALL SCORES ===");
    
    HighScore score;
    int row = 2;
    while (fread(&score, sizeof(HighScore), 1, file) == 1) {
      mvprintw(row++, COLS/2-10, "%s: %d", score.name, score.score);
      if (row >= LINES-2) break;
    }
    fclose(file);
    
    mvprintw(LINES-2, COLS/2-15, "Press any key to return");
    refresh();
    getch();
  }
}

void draw_city_with_delay(int world[], int cols, int lines) {
  struct timespec ts = { .tv_sec = 0, .tv_nsec = 1000000L };
  for (int x = 0; x < cols; x++) {
    for (int y = 0; y < world[x]; y++) {
      if (has_colors()) {
	attron(COLOR_PAIR(BUILDING_COLOR));
      }
      mvprintw(lines - y - 1, x, "#");
      if (has_colors()) {
	attroff(COLOR_PAIR(BUILDING_COLOR));
      }
      refresh();
      nanosleep(&ts, NULL);
    }
  }
}

void get_fortune_message(char* buffer) {
  // Use -n 300 to get longer fortunes (adjust number as needed)
  FILE* fp = popen("fortune -s -n 300", "r");
  if (fp) {
    // Read multiple lines if needed
    size_t total = 0;
    char line[FORTUNE_LENGTH];
    buffer[0] = '\0';
    
    while (fgets(line, sizeof(line), fp) != NULL && total < FORTUNE_LENGTH - 1) {
      strncat(buffer, line, FORTUNE_LENGTH - total - 1);
      total += strlen(line);
    }
    pclose(fp);
    
    // Replace newlines with spaces for smooth scrolling
    for (char *p = buffer; *p; p++) {
      if (*p == '\n') *p = ' ';
    }
  } else {
    strcpy(buffer, "BOMBER GAME - DESTROY THE CITY! ");
    // Add some padding to make it longer
    strcat(buffer, "FLY CAREFULLY! ");
    strcat(buffer, "AVOID THE BUILDINGS! ");
  }
}

void show_scrolling_message(const char* message, int scroll_pos, int row) {
  int len = strlen(message);
  int width = COLS;
  
  // Make the message loop by tripling it with spaces in between
  char extended[FORTUNE_LENGTH * 3];
  snprintf(extended, sizeof(extended), "%s   %s   %s", message, message, message);
  len = strlen(extended);
  
  if (has_colors()) {
    attron(COLOR_PAIR(PINK_TEXT_COLOR));
  }
  
  for (int i = 0; i < width; i++) {
    int msg_pos = (scroll_pos + i) % len;
    mvprintw(row, i, "%c", extended[msg_pos]);
  }
  
  if (has_colors()) {
    attroff(COLOR_PAIR(PINK_TEXT_COLOR));
  }
}
 
void show_help_screen(const char* fortune_msg, int* scroll_pos) {
  clear();
  // Draw help screen with new formatting
  int start_row = 2;
  mvprintw(start_row++, COLS/2-10, "=== BOMBER GAME HELP ===");
  mvprintw(start_row+=2, 2, "Controls:");
  mvprintw(++start_row, 4, "Down Arrow - Drop bomb");
  mvprintw(++start_row, 4, "Spacebar - Machine gun (%d ammo)", MAX_AMMO);
  mvprintw(++start_row, 4, "P - Pause game");
  mvprintw(++start_row, 4, "Q - Quit game");
  mvprintw(++start_row, 4, "H - This help screen");
  start_row += 2;
  mvprintw(start_row++, 2, "Game Rules:");
  mvprintw(++start_row, 4, "- Destroy all city blocks (#) to win");
  mvprintw(++start_row, 4, "- Avoid crashing into buildings");
  mvprintw(++start_row, 4, "- Bombs destroy 3-block wide area");
  mvprintw(LINES-2, COLS/2-15, "Press H to return to game");
  show_scrolling_message(fortune_msg, *scroll_pos, LINES-1);
  refresh();

 
  int ch = getch();
  if (ch == HELP_KEY || ch == HELP_KEY-32) {
  (*scroll_pos)++;
  show_scrolling_message(fortune_msg, *scroll_pos, LINES-1);
  refresh();
  nanosleep(&(struct timespec){0, SCROLL_DELAY/2}, NULL);
  }
  flushinp(); // Clear buffer after exiting
}

void show_menu() {
  HighScore scores[MAX_SCORES];
  load_scores(scores);
  
  clear();
  mvprintw(0, COLS/2-10, "=== BOMBER GAME ===");
  display_scores(scores);
  mvprintw(18, 0, "1. Start New Game");
  mvprintw(19, 0, "2. Game Info");
  mvprintw(20, 0, "3. Help");
  mvprintw(21, 0, "4. Show All Scores");
  mvprintw(22, 0, "5. Quit");
  refresh();
}

void pause_game(const char* fortune_msg, int* scroll_pos) {
  clear();
  mvprintw(LINES/2, COLS/2-5, "PAUSED");
  mvprintw(LINES/2+1, COLS/2-10, "Press P to continue");
  show_scrolling_message(fortune_msg, *scroll_pos, LINES-1);
  refresh();

  
    int ch = getch();
    if (ch == PAUSE_KEY || ch == PAUSE_KEY-32) {
    (*scroll_pos)++;
    show_scrolling_message(fortune_msg, *scroll_pos, LINES-1);
    refresh();
    nanosleep(&(struct timespec){0, SCROLL_DELAY/2}, NULL);
  }
  flushinp(); // Clear buffer after exiting
}

void end_game_pause() {
  struct timespec ts = { .tv_sec = END_GAME_PAUSE, .tv_nsec = 0 };
  nanosleep(&ts, NULL);
}
