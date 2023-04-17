#include "env.h"

/**
 * @brief réinitialise l'environnement
 *
 * @param menu score et timer
 * @param pinball la balle et les batteurs
 *
 * @return l'état de la balle
 */
state_t resetEnv(menu_part_t *menu, pinball_part_t *pinball) {
  menu->score = 0;
  // int alea = rand() % 650 + 50;
  pinball->ball =
      initBall(initVect(0, 0, 0), initVect(0, 0.4, 0), initVect(650, 50, 25));

  return (state_t){-1, -1, 0};
}

/**
 * @brief initialise les perception
 * 
 * @param grid 
 */

void initGrid(grid_t *grid) {
  int l_tmpx[] = {-50, -25, -15, -5, 0, 5, 15, 25};
  int l_tmpy[] = {-50, -10, 0, 10, 30};
  grid->rx = 600;
  grid->ry = 830;
  grid->lx = 150;
  grid->ly = 700;
  grid->column_number = 20;
  grid->line_number = 5;
  grid->angle_numberx = 8;
  grid->angle_numbery = 5;
  grid->angle_number = grid->angle_numberx * grid->angle_numbery;
  int *lix = malloc(grid->angle_numberx * sizeof(int));
  for (int i = 0; i < grid->angle_numberx; i++) {
    lix[i] = l_tmpx[i];
  }
  grid->angle_listx = lix;
  int *liy = malloc(grid->angle_numbery * sizeof(int));
  for (int i = 0; i < grid->angle_numbery; i++) {
    liy[i] = l_tmpy[i];
  }
  grid->angle_listy = liy;

  grid->x_step =
      (grid->rx - grid->lx + grid->column_number) / grid->column_number;
  grid->y_step = (grid->ry - grid->ly + grid->line_number) / grid->line_number;
}

/**
 * @brief associe une position, une vitesse en x et en y à une perception / ligne de Q
 * 
 * @param posx la position de la ball en x
 * @param posy la position de la ball en y
 * @param grid la grille pour connaitre les angles
 * @param ball la balle pour connaitre les vitesses en x et y
 * @return state_t 
 */

state_t state_detect(int posx, int posy, grid_t grid, ball_t ball) {
  state_t r_state;
  int tmp = 0;
  if ((posx < grid.lx) || (posx > grid.rx) || (posy < grid.ly) || (posy > grid.ry)) {
    r_state.x = -1;
    r_state.y = -1;
  } else {
    r_state.x = (posx - grid.lx) / grid.x_step;
    r_state.y = (posy - grid.ly) / grid.y_step;
  }
  r_state.angle = 0;
  for(int i=0;i<grid.angle_numberx;i++){
    if(ball.speed.x>grid.angle_listx[i]){
      r_state.angle = i;
    }
  }
  for(int j=0;j<grid.angle_numbery;j++){
    if(ball.speed.y>grid.angle_listy[j]){
      tmp = j;
    }
  }
  r_state.angle += tmp * grid.angle_numberx;
  return r_state;
}

/**
 * @brief fais évoluer l'environnement jusqu'au prochain état
 *
 * on applique la physique jusqu'à arriver dans un nouvel état
 *
 * @param ball
 * @return feedback_t
 */

void stepPlayer(pinball_part_t *pinball, menu_part_t *menu,
                pinball_part_sdl_t *pinballSDL, menu_part_sdl_t *menuSDL,
                graphique_t *graph, SDL_bool *running, int *IA,
                int *graphique) {

  ACTION action = NONE;
  int done = 1;

  SDL_Event event;
  while (done) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        *running = 0;
        done = 0;
        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
          *running = 0;
          done = 0;
          break;
        case SDLK_p:
          *graphique = *graphique ? 0 : 1;
          menuSDL->sono->on = *graphique;
          playmusic(menuSDL->sono);
          break;
        case SDLK_i:
          *IA = 1;
          break;
        case SDLK_a:
          if (menuSDL->sono->on && action != L) {
            playsound(menuSDL->sono, BATTEUR);
          }
          action = L;
          break;
        case SDLK_e:
          if (graphique && menuSDL->sono->on && action != R) {
            playsound(menuSDL->sono, BATTEUR);
          }
          action = R;
          break;
        }
        break;
      case SDL_KEYUP:
        switch (event.key.keysym.sym) {
        case SDLK_a:
        case SDLK_e:
          action = NONE;
          break;
        }
      }
    }

    collider_t c = collisionEnv(pinball, menu, menuSDL->sono);
    collisionBar(pinball, menuSDL->sono);
    actionOnBar(pinball, action);
    applyPhysics(&(pinball->ball));

    if (*graphique) {
      modifyTimer(menu, 16.66);
      drawSDL(graph, pinball, menu, pinballSDL, menuSDL);
      bumperBump(graph, pinballSDL, &c);
      SDL_RenderPresent(graph->rend);
    } else {
      drawMenu(graph, pinballSDL);
    }

    modifyBar(&pinball->bar_r, 13 * M_PI / 12 - pinball->ang_r * M_PI / 30);
    modifyBar(&pinball->bar_l, -M_PI / 12 + pinball->ang_l * M_PI / 30);

    if (pinball->ball.structure.y > 1000) {
      done = 0;
      if (graphique && menuSDL->sono->on) {
        playsound(menuSDL->sono, LOOSE);
      }
    }
  }
}

/**
 * @brief donne le prochain état ou l'ia doit se demander quelle action faire
 * 
 * @param action l'action précédent
 * @param pinball 
 * @param menu 
 * @param prev_state 
 * @param pinballSDL 
 * @param menuSDL 
 * @param graph 
 * @param grid 
 * @param running 
 * @param IA 
 * @param graphique 
 * @return feedback_t la nouvelle position de la balle et la reward obtenue jusqua présent
 */

feedback_t step(ACTION action, pinball_part_t *pinball, menu_part_t *menu,
                state_t prev_state, pinball_part_sdl_t *pinballSDL,
                menu_part_sdl_t *menuSDL, graphique_t *graph, grid_t grid,
                SDL_bool *running, int *IA, int *graphique) {
  int i = 0;
  feedback_t new_feed;
  new_feed.done = 0;
  state_t new_state =
      state_detect(pinball->ball.structure.x, pinball->ball.structure.y, grid,
                   pinball->ball);
  while ((prev_state.x == new_state.x) && (prev_state.y == new_state.y) &&
         !new_feed.done && i < 100) {

    new_state = state_detect(pinball->ball.structure.x,
                             pinball->ball.structure.y, grid, pinball->ball);

    collider_t c = collisionEnv(pinball, menu, menuSDL->sono);
    collisionBar(pinball, menuSDL->sono);
    actionOnBar(pinball, action);

    modifyBar(&pinball->bar_r, 13 * M_PI / 12 - pinball->ang_r * M_PI / 30);
    modifyBar(&pinball->bar_l, -M_PI / 12 + pinball->ang_l * M_PI / 30);

    applyPhysics(&(pinball->ball));
    i++;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        *running = SDL_FALSE;
        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
          *running = SDL_FALSE;
          break;
        case SDLK_i:
          *IA = 0;
          break;
        case SDLK_p:
          *graphique = *graphique ? 0 : 1;
          playmusic(menuSDL->sono);
          menuSDL->sono->on = *graphique;
          drawMenu(graph, pinballSDL);
          break;
        }
        break;
      }
    }

    if (pinball->ball.structure.y > 1000) {
      new_feed.done = 1;
    }

    if (*graphique) {
      modifyTimer(menu, 16.66);
      drawSDL(graph, pinball, menu, pinballSDL, menuSDL);
      bumperBump(graph, pinballSDL, &c);
      SDL_RenderPresent(graph->rend);
    }
  }
  new_feed.reward = 0;

  if (pinball->ball.structure.y > 1000) {
    new_state.x = -2;
    new_state.y = -2;
    new_feed.reward = menu->score;
    if (menuSDL->sono->on) {
      playsound(menuSDL->sono, LOOSE);
    }
  }

  new_feed.state = new_state;

  return new_feed;
}
