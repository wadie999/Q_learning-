
#ifndef __LEARNING_H__
#define __LEARNING_H__

#include "env.h"
#include "param.h"

#define MAX_TICKS 120e9

typedef union sar {
  state_t s;
  ACTION a;
  int r;
} sar_t;

typedef struct cell {
	sar_t val;
	struct cell * next;
} cell_t;

typedef cell_t * linked_list_t;

/**
 * @brief tableaux pour l'apprentissage obtenus après une run
 * 
 */
typedef struct run_feedback {
	linked_list_t states;
	linked_list_t actions;
	linked_list_t rewards;
} run_result_t;

run_result_t run(param_t *param, pinball_part_t *pinball, menu_part_t *menu,
                 pinball_part_sdl_t *pinballSDL, menu_part_sdl_t *menuSDL,
                 graphique_t *graph, grid_t grid, SDL_bool *running, int *IA,
                 int *graphique);

void learn(run_result_t * result, param_t * param, grid_t grid);
void learnSarsa(run_result_t *result, param_t *param, grid_t grid);
void freeLinkedList(linked_list_t l);
int stack(linked_list_t * l, sar_t val);
sar_t unstack(linked_list_t * l);
float maxQuality(float * line, int action_number) ;
int maxQualityIndex(float * line, int action_number);
ACTION randomAction();
#endif
