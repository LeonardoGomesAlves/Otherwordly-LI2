#ifndef ___STATE_H___
#define ___STATE_H___

#define ROWS 40
#define COLS 130
#include <stdbool.h>
 
typedef struct monster
 {
	int monstroX;
	int monstroY;
	int monstro_hp;
	int cor;
	int msT;
 } MONSTER;
 
 typedef struct bullet
 {
 	int bulletX;
 	int bulletY;
 } BULLET;

typedef struct mapa 
{
	bool is_wall;
	bool is_portal;
	int dist;
	int is_viewable;
} MAP;
 
typedef struct state 
{
	int ncols;
	int nrows;
	
	MAP map[ROWS+1][COLS+1];
	float prob_wall;
	int lvl;
	
	int playerX;
	int playerY;
	int player_hp;
	int player_gold;
	int player_damage;
	int n_potion;
} STATE;
#endif


