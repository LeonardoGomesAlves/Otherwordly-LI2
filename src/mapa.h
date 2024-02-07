#include "state.h"

int radius_count(STATE *st, int r, int c, int raio);

void generate_map(STATE *st, MONSTER ms[], int n_monstros, char (*mapa)[st->ncols]);

int detect_bullet(BULLET *b,STATE *st, MONSTER ms[],int n_monstros,int save,char (*mapa)[st->ncols],WINDOW *gameplay);

void floodfill (int yy, int xx, int valor, int valor1, STATE *st, WINDOW *gameplay, char (*mapa)[st->ncols]);

void monster_movement(STATE *st, MONSTER ms[],int n_monstros,char (*mapa)[st->ncols], WINDOW *gameplay);

void meeleDamage(STATE *st, MONSTER ms[], int n_monstros, char (*mapa)[st->ncols], WINDOW *gameplay);

void spawn_slimes(STATE *st,BULLET *b, MONSTER ms[], int n_monstros, char (*mapa)[st->ncols], WINDOW *gameplay, int save);
