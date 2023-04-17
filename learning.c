
#include "learning.h"
#include <time.h>

/**
 * @brief libère une liste chainée
 * 
 * @param l la liste à libérer
 */
void freeLinkedList(linked_list_t l)
{
	cell_t * curr = l;
	cell_t * prec;

	while(curr != NULL) {
		prec = curr;
		curr = curr->next;
		free(prec);
	}
}

/**
 * @brief insere un élément en tête de liste chainée
 * 
 * @param l pointeur sur la liste
 * @param val l'élément
 * @return int 0 si l'insertion s'est bien passée, 1 sinon
 */
int stack(linked_list_t *l, sar_t val) {
  float r = 0;

  cell_t *new = malloc(sizeof(cell_t));
  if (new != NULL) {
    new->val = val;
    new->next = *l;
    *l = new;
  } else {
    fprintf(stderr, "erreur d allocation lors de l insertion\n");
    r = 1;
  }

  return r;
}

/**
 * @brief supprimme et retourne l'élément en tête d'une liste chainée
 * 
 * @param l la liste
 * @return sar_t la valeur de la tête
 */
sar_t unstack(linked_list_t *l) {
  sar_t val = {{.x = -2, .y = -2}};
  if (*l != NULL) {
    val = (*l)->val;
    cell_t *next = (*l)->next;
    free(*l);
    *l = next;
  } else {
    fprintf(stderr, "erreur : suppression liste vide\n");
  }

  return val;
}

/**
 * @brief renvoie la valeur max d'un tableau
 * 
 * @param line 
 * @param action_number le nombre d'éléments
 * @return float
 */
float maxQuality(float *line, int action_number) {
  float max = line[0];
  for (int i = 1; i < action_number; i++) {
    if (line[i] > max)
      max = line[i];
  }

  return max;
}

/**
 * @brief renvoie l'indice de la valeur max d'un tableau
 * 
 * @param line 
 * @param action_number le nombre d'éléments
 * @return int 
 */
int maxQualityIndex(float * line, int action_number) 
{
	float max = line[0];
	int index = 0;
	for(int i = 1 ; i < action_number ; i++)
	{
		if(line[i] > max) 
		{
			max = line[i];
			index = i;
		}
	}

	return index;
}

/**
 * @brief revoie une action choisie aléatoirement
 * 
 * @return ACTION 
 */
ACTION randomAction()
{
	float r = random()/(RAND_MAX + 1.0);
	if(r < 0.25) return NONE;
	if(r < 0.5) return L;
	if(r < 0.75) return R;
	return LR;
}

/**
 * @brief effectue une run
 * 
 * @param param les paramètres de l'IA
 * @param pinball les paramètres d'environnement
 * @param menu les variables du menu
 * @param pinballSDL 
 * @param menuSDL 
 * @param graph 
 * @param grid la grille
 * @return run_result_t 
 */
run_result_t run(param_t *param, pinball_part_t *pinball, menu_part_t *menu,
                 pinball_part_sdl_t *pinballSDL, menu_part_sdl_t *menuSDL,
                 graphique_t *graph, grid_t grid, SDL_bool *running, int *IA,
                 int *graphique) {
  float r;
  ACTION action;
  ACTION prec_action = NONE;
  run_result_t result;
  result.states = NULL;
  result.actions = NULL;
  result.rewards = NULL;

  feedback_t feedback;
  feedback.done = 0;

  clock_t begin = clock();
  int nbr_step = 0;

  sum(param);

  stack(&(result.states), (sar_t){.s = resetEnv(menu, pinball)});

  while (!feedback.done && (clock() - begin < MAX_TICKS) && (nbr_step < 150)) {

    r = random() / (RAND_MAX + 1.0);
    if (r < param->eps)
      action = randomAction();
    else {
      action =
          maxQualityIndex(param->Q[stateToLine(result.states->val.s, grid)],
                          param->action_number);
    }
    if (menuSDL->sono->on) {
      if (action != NONE && action != prec_action) {
        playsound(menuSDL->sono, BATTEUR);
      }
    }
    prec_action = action;

    stack(&(result.actions), (sar_t){.a = action});
    feedback = step(action, pinball, menu, result.states->val.s, pinballSDL,
                    menuSDL, graph, grid, running, IA, graphique);
    if (action != NONE) {
      nbr_step++;
    }
    stack(&(result.states), (sar_t){.s = feedback.state});
    stack(&(result.rewards), (sar_t){.r = feedback.reward});
  }

  param->iteration_count += 1;

  return result;
}

/**
 * @brief apprentissage mise à jour de la table de qualité avec le retour
 *  d'une run
 * 
 * @param result les résultats de la run à prendre en compte
 * @param param les paramètres d'apprentissage
 * @param grid la grille
 */
void learn(run_result_t *result, param_t *param, grid_t grid) {

  float gamma1 = 0.999;
  float gamma2 = 0.4;
  int loose_reward = 0;

  float alpha = random() / (RAND_MAX + 1.0);
  if (alpha < 0.5) {
    // Q1 est toujours la table d'apprentissage
    // et on échange les tables aléatoirement
    float **Q = param->Q1;
    param->Q1 = param->Q2;
    param->Q2 = Q;
  }

  unstack(&(result->states));

  int s, a, r, s1, a1, r1;

  s = stateToLine(unstack(&(result->states)).s, grid);
  a = unstack(&(result->actions)).a;
  r = unstack(&(result->rewards)).r;

  if (result->states->val.s.x == -2) {
    loose_reward = -7000;
  }

  param->Q1[s][a] += param->xi * (r - param->Q1[s][a]);

  while (result->states != NULL) {
    alpha = random() / (RAND_MAX + 1.0);
    if (alpha < 0.5) {
      float **Q = param->Q1;
      param->Q1 = param->Q2;
      param->Q2 = Q;
    }

    s1 = s;
    r1 = r;
    a1 = a;

    s = stateToLine(unstack(&(result->states)).s, grid);
    a = unstack(&(result->actions)).a;
    r = unstack(&(result->rewards)).r;

    param->Q1[s][a] +=
        param->xi *
        (r1 + gamma1 * maxQuality(param->Q2[s1], param->action_number) -
         param->Q1[s][a]);

    loose_reward = gamma2 * loose_reward;
    param->Q2[s1][a1] += param->xi * loose_reward;
  }
}

void learnSarsa(run_result_t *result, param_t *param, grid_t grid) {

  float gamma1 = 0.9985;
  float gamma2 = 0.5;
  float loose_reward = 0;

  float alpha = random() / (RAND_MAX + 1.0);
  if (alpha < 0.5) {
    // Q1 est toujours la table d'apprentissage
    // et on échange les tables aléatoirement
    float **Q = param->Q1;
    param->Q1 = param->Q2;
    param->Q2 = Q;
  }

  unstack(&(result->states));

  int s, a, r, s1, a1, r1;

  s = stateToLine(unstack(&(result->states)).s, grid);
  a = unstack(&(result->actions)).a;
  r = unstack(&(result->rewards)).r;

  if (result->states->val.s.x == -2) {
    loose_reward = -7000;
  }

  param->Q1[s][a] += param->xi * (r - param->Q1[s][a]);

  while (result->states != NULL) {
    alpha = random() / (RAND_MAX + 1.0);
    if (alpha < 0.5) {
      float **Q = param->Q1;
      param->Q1 = param->Q2;
      param->Q2 = Q;
    }

    s1 = s;
    r1 = r;
    a1 = a;

    s = stateToLine(unstack(&(result->states)).s, grid);
    a = unstack(&(result->actions)).a;
    r = unstack(&(result->rewards)).r;

    param->Q1[s][a] +=
        param->xi * (r1 + gamma1 * param->Q2[s1][a1] - param->Q1[s][a]);

    loose_reward = gamma2 * loose_reward;
    param->Q2[s1][a1] += param->xi * loose_reward;
  }
}
