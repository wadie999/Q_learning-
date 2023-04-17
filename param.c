#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "param.h"

float **createTable(int state_number, int action_number) 
{
	float ** Q = malloc(state_number * sizeof(float*));
	if(Q != NULL)
	{
		for(int i = 0 ; i < state_number ; i++) 
		{
			Q[i] = malloc(action_number * sizeof(float));
			if(Q[i] == NULL)
			{
				fprintf(stderr, "erreur d'allocation\n");
				exit(EXIT_FAILURE);
			}
		}
	}

	return Q;	
}

/**
 * @brief permet de libérer un table de qualités
 * 
 * @param Q la table à libérer
 * @param state_number l nombre de lignes
 */
void freeTable(float **Q, int state_number) {
  for (int i = 0; i < state_number; i++)
    free(Q[i]);
  free(Q);
}

/**
 * @brief quitte le programme en libérant les tables de qualité
 * 
 * @param param 
 * @param code 
 */
void exit_program(param_t param, int code)
{
	freeTable(param.Q1, param.state_number);
	freeTable(param.Q2, param.state_number);
	freeTable(param.Q2, param.state_number);
	exit(code);
}

/**
 * @brief permet de charger les paramètres d'apprentissage depuis un fichier
 *
 * @param filename le nom du fichier
 * @param param les paramètres d'apprentissage
 */
void load(const char *filename, param_t * param)
{
	FILE * f = NULL;
	f = fopen(filename, "r");
	if(f != NULL)
	{
		if(fscanf(f, "%d\n%f\n%f\n", &(param->iteration_count), &(param->eps), &(param->xi)) != 3)
		{
			fprintf(stderr, "erreur de lecture\n");
			fclose(f);
			exit_program(*param, EXIT_FAILURE);
		}

		for(int i = 0 ; i < param->state_number ; i++)
		{
			for(int j = 0 ; j < param->action_number ; j++)
			{
				if(fscanf(f, "%f", param->Q1[i]+j) != 1)
				{
					fprintf(stderr, "erreur de lecture\n");
					fclose(f);
					exit_program(*param, EXIT_FAILURE);
				}
			}
		}

		for(int i = 0 ; i < param->state_number ; i++)
		{
			for(int j = 0 ; j < param->action_number ; j++)
			{
				if(fscanf(f, "%f", param->Q2[i]+j) != 1)
				{
					fprintf(stderr, "erreur de lecture\n");
					fclose(f);
					exit_program(*param, EXIT_FAILURE);
				}
			}
		}

		fclose(f);
	}
	else
	{
		perror("erreur ouverture fichier : ");
		exit_program(*param, EXIT_FAILURE);
	}
}

/**
 * @brief permet de sauvegarder les paramètres d'apprentissage dans un fichier
 *
 * @param filename le nom du fichier
 * @param param les paramètres d'apprentissage
 */
void save(const char *filename, param_t param)
{
	FILE *f = NULL;
	if (!strcmp(filename, "stdout"))
		f = stdout;
	else
		f = fopen(filename, "w");
	
	if (f != NULL)
	{
		if (fprintf(f, "%d\n%f\n%f\n", param.iteration_count, param.eps, param.xi) != EOF)
		{
			if(saveTable(f, param.Q1, param.state_number, param.action_number))
			{
				perror("erreur ecriture");
				fclose(f);
				save("stdout", param);
				return;
			}
			if(saveTable(f, param.Q2, param.state_number, param.action_number))
			{
				perror("erreur ecriture");
				fclose(f);
				save("stdout", param);
				return;
			}
		}
		else
		{
			perror("erreur ecriture");
			fclose(f);
			save("stdout", param);
			return;
		}

		if (strcmp(filename, "stdout"))
		{
			fclose(f);
		}
	}
}

/**
 * @brief permet d'effectuer la sauvegarde d'une table de qualité à la suite d'un fichier
 * 
 * @param f 
 * @param Q 
 * @param state_number 
 * @param action_number 
 * @return int 0 si bien terminé, autre chose en cas de problème
 */
int saveTable(FILE *f, float **Q, int state_number, int action_number)
{
	for (int i = 0; i < state_number; i++)
	{
		for (int j = 0; j < action_number; j++)
		{
			if (fprintf(f, "%f ", Q[i][j]) == EOF) return 1;
		}

		if (fprintf(f, "\n") == EOF) return 2;
	}

	return 0;
}

/**
 * @brief calcul la nouvlle valeur d'epsilon
 * 
 * @param eps valeur actuelle d'epsilon, paramètre de l'epsilon-greedy
 * @param itearation_number nombre de runs effectuées
 * @return float nouvelle valeur d'epsilon
 */
float calcEps(float eps, int iteration_number) {
  if (iteration_number % 8000 == 0) {
    eps = 0.98 * eps;
  }
  return eps;
}

/**
 * @brief calcul la nouvlle valeur de Xi
 * 
 * @param xi valeur actuelle d'epsilon, paramètre de la formule d'apprentissage
 * @param iteration_number nombre de runs effectuées
 * @return float nouvelle valeur de Xi
 */
float calcXi(float xi, int iteration_number) {
  if (iteration_number % 15000 == 0) {
    xi = 0.99 * xi;
  }
  return xi;
}

/**
 * @brief somme les 2 tables de qualité
 *
 * @param param, les paramètres d'apprentissage
 */
void sum(param_t * param) {
  for (int i = 0; i < param->state_number; i++) {
    for (int j = 0; j < param->action_number; j++) {
     param->Q[i][j] = param->Q1[i][j] + param->Q2[i][j];
    }
  }
}

/**
 * @brief transforme un état en numéro de ligne
 * 
 * @param state l'état
 * @param grid la grille
 * @return int le numéro de ligne
 */
int stateToLine(state_t state, grid_t grid) {
  if (state.x == -1)
    return 0;
  if (state.x == -2)
    return 1;

  return grid.column_number * state.y + state.x + 2 + state.angle*(grid.column_number*grid.line_number);
}
