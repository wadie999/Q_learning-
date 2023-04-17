
#ifndef __PARAM_H__
#define __PARAM_H__

#include <stdio.h>
#include "env.h"

typedef struct parameters {
	int state_number; 			//nombre d'états
	int action_number;			//nombre d'actions
	int iteration_count; 		//nombre de runs effectuées
	float eps;					//variable pour l'epsilon-greedy
	float xi;					//variable de la formule d'aprentissage
	float ** Q1;				//permière table de qualités
	float ** Q2; 				//deuxième table de qualités
	float ** Q;					//somme de Q1 et Q2
} param_t;

float **createTable(int state_number, int action_number);
void freeTable(float ** Q, int state_number);
void exit_program(param_t param, int code);
void load(const char *filename, param_t * param);
void save(const char *filename,  param_t param);
int saveTable(FILE *f, float **Q, int state_number, int action_number);
float calcEps(float eps, int itearation_number);
float calcXi(float xi, int interation_number);
void sum(param_t * param);
int stateToLine(state_t state, grid_t grid);

#endif