/*
 * Bomber Game
 * Version: 1.0
 * Copyright (c) 2025 Peter Leukanič 
 * Under MIT License
 *
 */

#include "bomber.h"

int main() {
  srand(time(NULL));
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);

  ensure_score_file();

  // Initialize colors if terminal supports it
  if (has_colors()) {
    start_color();
    if (can_change_color()) {
      init_color(BACKGROUND_COLOR, 1000, 1000, 1000);
    }
    init_pair(HELP_COLOR, COLOR_CYAN, BACKGROUND_COLOR);
    init_pair(BOMBER_COLOR, COLOR_BLUE, BACKGROUND_COLOR);      // Dark blue bomber
    init_pair(BUILDING_COLOR, COLOR_RED, BACKGROUND_COLOR);     // Red buildings
    init_pair(BOMB_COLOR, COLOR_BLACK, BACKGROUND_COLOR);       // Black bomb
    init_pair(TEXT_COLOR, COLOR_BLACK, BACKGROUND_COLOR);       // White text
    init_pair(PINK_TEXT_COLOR, COLOR_MAGENTA, BACKGROUND_COLOR);  // Pink text

    // Set the background color for the whole screen
    bkgd(COLOR_PAIR(TEXT_COLOR)); 
  }

  HighScore test_scores[MAX_SCORES];
  load_scores(test_scores);  
  char player_name[MAX_NAME_LENGTH] = "Player";
  char fortune_msg[FORTUNE_LENGTH];
  get_fortune_message(fortune_msg);
  int scroll_pos = 0;
  
  
  while (1) {
    show_menu();
    int menu_choice = getch();
    
    if (menu_choice == '1') {
      clear();
      get_player_name(player_name);
      break;
    } else if (menu_choice == '2') {
      show_info_screen(fortune_msg, &scroll_pos);
    } else if (menu_choice == '3') {
      show_help_screen(fortune_msg, &scroll_pos);
    } else if (menu_choice == '4') {
      show_all_scores();
    } else if (menu_choice == '5') {
      endwin();
      return 0;
    }
  }
  
  nodelay(stdscr, TRUE);
  
  int world[COLS];
  for (int i = 0; i < COLS; i++) {
    world[i] = rand() % (LINES / 3) + 1;
  }
  
  clear();
  draw_city_with_delay(world, COLS, LINES);
  
  int bomber_x = 0;
  int bomber_y = 1;
  int bomber_dx = 1;
  int score = 0;
  Bomb bomb = {0};
  int game_over = 0;
  int win = 0;
  int paused = 0;
  int crash_reason = 0;
  int shots = MAX_AMMO;
  int machine_gun_active = 0;
  int machine_gun_bullet_x = 0;  
  int machine_gun_bullet_y = 0;
  int bullet_distance = 0;       
  int machine_gun_direction = 1;  // 1 for right, -1 for left
  
  
  struct timespec ts_frame = { .tv_sec = 0, .tv_nsec = 60000000L };
  struct timespec ts_bomb = { .tv_sec = 0, .tv_nsec = 20000000L };
  
  while (!game_over && !win) {
    if (!paused) {
      erase();
      
      int city_destroyed = 1;
      for (int x = 0; x < COLS; x++) {
	if (world[x] > 0) {
	  city_destroyed = 0;
	  for (int y = 0; y < world[x]; y++) {
	    if (has_colors()) {
	      attron(COLOR_PAIR(BUILDING_COLOR));
	    }
	    mvprintw(LINES - y - 2, x, "#");
	    if (has_colors()) {
	      attroff(COLOR_PAIR(BUILDING_COLOR));
	    }
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
      
      if (has_colors()) {
	attron(COLOR_PAIR(BOMBER_COLOR));
      }
      mvprintw(bomber_y, bomber_x, bomber_dx > 0 ? "^==-" : "-==^");
      if (has_colors()) {
	attroff(COLOR_PAIR(BOMBER_COLOR));
      }
      
      int collision_x = bomber_x;
      if (bomber_dx > 0) { // Moving right
	collision_x += 3;
      } else { // Moving left
	collision_x -= 1;
      }
      
      if (collision_x >= 0 && collision_x < COLS && 
	  bomber_y >= LINES - world[collision_x] - 1) {
	// Visual feedback before ending
	for (int i = 0; i < 3; i++) {
	  attron(A_BLINK | COLOR_PAIR(BUILDING_COLOR));
	  mvprintw(bomber_y, bomber_x, bomber_dx > 0 ? "^==-" : "-==^");
	  refresh();
	  nanosleep(&(struct timespec){0, 200000000L}, NULL);
	  attroff(A_BLINK | COLOR_PAIR(BUILDING_COLOR));
	}
	crash_reason = 1; // Building collision
	game_over = 1;
	break;
      }

      if (machine_gun_active) {
	// Erase previous bullet if within bounds
	if (machine_gun_bullet_x >= 0 && machine_gun_bullet_x < COLS) {
	  mvprintw(machine_gun_bullet_y, machine_gun_bullet_x, " ");
	}
	
	// Move bullet forward by 2 positions for faster movement
	machine_gun_bullet_x += machine_gun_direction * 2;
	bullet_distance += 2;
	
	// Draw new position if within bounds
	if (machine_gun_bullet_x >= 0 && machine_gun_bullet_x < COLS) {
	  if (has_colors()) {
            attron(COLOR_PAIR(BOMB_COLOR));
	  }
	  mvprintw(machine_gun_bullet_y, machine_gun_bullet_x, "-");
	  if (has_colors()) {
            attroff(COLOR_PAIR(BOMB_COLOR));
	  }
	}
	
	// Check for hits or max range
	if (bullet_distance >= MACHINE_GUN_RANGE || 
	    (machine_gun_bullet_x >= 0 && machine_gun_bullet_x < COLS && 
	     machine_gun_bullet_y >= LINES - world[machine_gun_bullet_x] - 1)) {
	  
	  // Erase bullet if it's still on screen
	  if (machine_gun_bullet_x >= 0 && machine_gun_bullet_x < COLS) {
            mvprintw(machine_gun_bullet_y, machine_gun_bullet_x, " ");
	  }
	  
	  // Hit detection
	  if (machine_gun_bullet_x >= 0 && machine_gun_bullet_x < COLS && 
	      machine_gun_bullet_y >= LINES - world[machine_gun_bullet_x] - 1) {
            if (world[machine_gun_bullet_x] > 0) {
	      // Destroy 5 blocks in bullet's direction (hit block + 4 more)
	      for (int i = 0; i < 5; i++) {
		int destroy_x = machine_gun_bullet_x + (i * machine_gun_direction);
		if (destroy_x >= 0 && destroy_x < COLS && world[destroy_x] > 0) {
		  world[destroy_x]--;
		  score += 5; // Score for each destroyed block
		}
	      }
              
	      // Visual feedback
	      for (int i = 0; i < 5; i++) {
		int effect_x = machine_gun_bullet_x + (i * machine_gun_direction);
		if (effect_x >= 0 && effect_x < COLS) {
		  if (has_colors()) {
		    attron(COLOR_PAIR(BOMB_COLOR) | A_BLINK);
		  }
		  mvprintw(machine_gun_bullet_y, effect_x, "X");
		  if (has_colors()) {
		    attroff(COLOR_PAIR(BOMB_COLOR) | A_BLINK);
		  }
		}
	      }
	      refresh();
	      nanosleep(&(struct timespec){0, 100000000L}, NULL); // 0.1s hit effect
            }
	  }
	  machine_gun_active = 0;
	}
	// Very short delay for visible but fast movement
	nanosleep(&(struct timespec){0, 10000000L}, NULL); // 10ms delay
      }
      
      if (bomb.active) {
	if (has_colors()) {
	  attron(COLOR_PAIR(BOMB_COLOR));
	}
	mvprintw(bomb.y, bomb.x, "*");
	if (has_colors()) {
	  attroff(COLOR_PAIR(BOMB_COLOR));
	}
	bomb.y++;
	
	if (bomb.y >= LINES - world[bomb.x] - 2) {
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
      
      mvprintw(0, 0, "Player: %s  Score: %d  Ammo: %d", player_name, score, shots);
      show_scrolling_message(fortune_msg, scroll_pos, LINES-1);
      scroll_pos ++; 
      refresh();
    }
    
    int ch = getch();
    switch (ch) {
    case BOMB_KEY:
      if (!bomb.active && !paused) {
	// Check if bomber is at safe altitude
	if (bomber_y < LINES - SAFE_BOMB_HEIGHT) {
	  bomb.x = bomber_x + (bomber_dx > 0 ? 2 : 1);
	  bomb.y = bomber_y + 1;
	  bomb.active = 1;
	} else {
	  // Show warning message
	  mvprintw(1, 0, "TOO LOW TO BOMB! (Need %d units)", SAFE_BOMB_HEIGHT);
	  refresh();
	  nanosleep(&(struct timespec){0, 500000000L}, NULL); // 0.5s warning
	}
      }
      break;     
    case MACHINE_GUN_KEY:
      if (!paused && shots > 0 && !machine_gun_active) {
        machine_gun_active = 1;
        machine_gun_bullet_x = bomber_x + (bomber_dx > 0 ? 5 : -2); // Start bullet one character in front of nose
        machine_gun_bullet_y = bomber_y;
        machine_gun_direction = bomber_dx > 0 ? 1 : -1;
        bullet_distance = 0;
        shots--;
        
        // Draw initial bullet
        if (has_colors()) {
	  attron(COLOR_PAIR(BOMB_COLOR));
        }
        mvprintw(machine_gun_bullet_y, machine_gun_bullet_x, "-");
        if (has_colors()) {
	  attroff(COLOR_PAIR(BOMB_COLOR));
        }
        refresh();
      }
    break;  
    case 'q':
    case 'Q':
      game_over = 1;
      if (has_colors()) {
        attron(COLOR_PAIR(TEXT_COLOR));
      }
      mvprintw(LINES/2, COLS/2 - 5, "Quit Game");
      mvprintw(LINES/2+1, COLS/2-10, "Continue by press any key..");
      if (has_colors()) {
        attroff(COLOR_PAIR(TEXT_COLOR));
      }
      refresh();
      nodelay(stdscr, FALSE);  // Switch to blocking mode for quit confirmation
      getch();
      endwin();
      return EXIT_SUCCESS;
      break;
    case PAUSE_KEY:
    case PAUSE_KEY-32:
      paused = !paused;
      if (paused) {
	flushinp(); 
	pause_game(fortune_msg, &scroll_pos);
	flushinp();
      }
      break;
    case HELP_KEY:
    case HELP_KEY-32: {
      paused = !paused;
      if (paused) {
	flushinp(); 
	show_help_screen(fortune_msg, &scroll_pos);
	flushinp();
      }
      break;
    }}
    nanosleep(&ts_frame, NULL);
  }
  // Enhanced end screen display
  clear();
  if (has_colors()) {
    attron(COLOR_PAIR(TEXT_COLOR));
  }
  mvprintw(LINES/2, COLS/2-4, win ? "WELL DONE!" : "GAME OVER!");
  mvprintw(LINES/2+1, COLS/2-8, "Score: %d", score);

  // In end-game message
  if (!win) {
    const char* crash_msg = crash_reason ? 
        "Crashed into city!" : "Destroyed by own bomb!";
    mvprintw(LINES/2+2, COLS/2-10, crash_msg);
  }
  
  if (has_colors()) {
    attroff(COLOR_PAIR(TEXT_COLOR));
  }
  show_scrolling_message(fortune_msg, scroll_pos, LINES-1);
  refresh();
  
  /* Guaranteed end-game pause */
  time_t start = time(NULL);
  while (difftime(time(NULL), start) < END_GAME_PAUSE) {
    int ch = getch();
    if (ch != ERR) break;
    show_scrolling_message(fortune_msg, ++scroll_pos, LINES-1);
    refresh();
    nanosleep(&(struct timespec){0, 50000000L}, NULL);
  }
  save_score(player_name, score);
  endwin();

  return 0;
}
