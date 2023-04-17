
#ifndef __ENV_H__
#define __ENV_H__

#include "SDL.h"
#include "engine.h"


typedef struct state {
  int x; // -1 si hors grille, -2 si perdu
  int y; // -1 si hors grille, -2 si perdu
  int angle;
} state_t;

/**
 * @brief informations retournées après une action
 * 
 */
typedef struct action_feedback {
  state_t state;
  int reward;
  int done; // booléen pour savoir si le jeu est terminé ou non
} feedback_t;

typedef struct grid {
	int lx;
	int rx;
	int ly;
	int ry;
	int x_step;
	int y_step;
	int line_number;
	int column_number;
  int angle_numberx;
  int angle_numbery;
  int angle_number;
  int * angle_listx;
  int * angle_listy;
} grid_t;

state_t resetEnv(menu_part_t *menu, pinball_part_t *pinball);
state_t state_detect(int posx, int posy, grid_t grid, ball_t ball);
void initGrid(grid_t * grid);
feedback_t step(ACTION action, pinball_part_t *pinball, menu_part_t *menu,
                state_t prev_state, pinball_part_sdl_t *pinballSDL,
                menu_part_sdl_t *menuSDL, graphique_t *graph, grid_t grid,
                SDL_bool *running, int *IA, int *graphique);
void stepPlayer(pinball_part_t *pinball, menu_part_t *menu,
                pinball_part_sdl_t *pinballSDL, menu_part_sdl_t *menuSDL,
                graphique_t *graph, SDL_bool *running, int *IA, int *graphique);

#endif
