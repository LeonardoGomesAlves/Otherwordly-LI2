#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <stdbool.h>

#include "mapa.h"

#define BLACK 0
#define RED 1
#define GREEN 2
#define YELLOW 3
#define BLUE 4
#define AMARELO 5
#define FLOOR 6
#define WALL 7
#define PORTAL 8
#define CHAO 9
#define PAREDE 10

#define ROWS 40
#define COLS 130



//--------------------------------------------------- FLOODFILL -----------------------------------------------------//

/*
* a104093 - Leonardo Gomes Alves
* Função floodfill que serve para o movimento dos mobs, e luz.
*/
void floodfill (int yy, int xx, int valor, int valor1, STATE *st, WINDOW *gameplay, char (*mapa)[st->ncols])  // valor é o raio -1
{ 
		if (xx >= st->ncols || yy >= st->nrows) return;

		if ((st->map[yy][xx].is_wall)) return;

		int distancia = valor - valor1; // distancia real
		if (distancia < valor1 && distancia <= st->map[yy][xx].dist) {
		st->map[yy][xx].dist = distancia;
		if (mapa[yy][xx] == 'G' || mapa[yy][xx] == 'S' || mapa[yy][xx] == 's') st->map[yy][xx].is_viewable = 1;
		}

		if (distancia >= valor1) return;

		floodfill (yy, xx + 1, valor + 1, valor1, st, gameplay, mapa);
		floodfill (yy, xx - 1, valor + 1, valor1, st, gameplay, mapa);
		floodfill (yy + 1, xx, valor + 1, valor1, st, gameplay, mapa);
		floodfill (yy - 1, xx, valor + 1, valor1, st, gameplay, mapa);
		floodfill (yy + 1, xx + 1, valor + 1, valor1, st, gameplay, mapa);
		floodfill (yy + 1, xx - 1, valor + 1, valor1, st, gameplay, mapa);
		floodfill (yy - 1, xx + 1, valor + 1, valor1, st, gameplay, mapa);
		floodfill (yy - 1, xx - 1, valor + 1, valor1, st, gameplay, mapa);
}


/*
* a104520 - Salvador Duarte Magalhaes Barreto 
* Função que calcula quantas casas livres estão em volta de um dado ponto tendo em conta a um raio.
*/
int radius_count(STATE *st, int r, int c, int raio)
{
    int count = 0;

    for (int i = r-raio; i <= r+raio; i++) 
	{
        for (int j = c-raio; j <= c+raio; j++) 
		{
            // verifica se a posição está dentro do raio e dentro do mapa
            if (i >= 0 && i < st->nrows && j >= 0 && j < st->ncols) 
			{
                // incrementa o contador quando não é uma parede (#)
                if (!st->map[i][j].is_wall) 
				{
                    count++;
                }
            }
        }
    }

	if(!(st->map[r][c].is_wall)) count--; 

    return count;
}



//------------------------------------------------ GERAÇÃO DO MAPA -----------------------------------------------//
/*
* a104520 - Salvador Duarte Magalhaes Barreto 
* Função que cria o mapa.
*/

void generate_map(STATE *st, MONSTER ms[], int n_monstros, char (*mapa)[st->ncols])
{
	srand48(time(NULL));
	start_color();
	init_pair(1, RED,BLACK);
	
	// preencher o mapa completamente com #
	for(int yy = 0; yy <= st->nrows; yy++)
	{
		for(int xx = 0; xx <= st->ncols; xx++)
		{
			mapa[yy][xx] = '#';

			st->map[yy][xx].is_wall = true;
		}
	}

	// preencher o mapa aleatoriamente com paredes e espaços em branco
	for(int r = 1; r <= st->nrows-1; r++)
	{
		for(int c = 1; c <= st->ncols-1; c++)
		{
			st->map[r][c].is_wall = drand48() < st->prob_wall;

			if(!(st->map[r][c].is_wall)) mapa[r][c] = '.';
		}
	}
	


	// faz a primeira passagem pelo algoritmo de aleatoriedade 4 vezes
	for(int i = 0; i < 4; i++)
	{
		bool aux[st->nrows+1][st->ncols+1];   // matriz auxiliar para armazenar o valor boleano de cada posição

		for(int r = 1; r <= st->nrows-1; r++)
		{
			for(int c = 1; c <= st->ncols-1; c++)
			{
				aux[r][c] = (radius_count(st, r, c, 1) >= 5) || (radius_count(st, r, c, 2) <= 2);
			}
		}

		for(int r = 1; r <= st->nrows-1; r++)
		{
			for(int c = 1; c <= st->ncols-1; c++)
			{
				st->map[r][c].is_wall = aux[r][c];
			}
		}
	}

	// faz a segunda passagem pelo algoritmo de aleatoriedade 4 vezes
	for(int i = 0; i < 4; i++)
	{
		bool aux[st->nrows+1][st->ncols+1];   

		for(int r = 1; r <= st->nrows-1; r++)
		{
			for(int c = 1; c <= st->ncols-1; c++)
			{
				aux[r][c] = (radius_count(st, r, c, 1) >= 5);
			}
		}

		for(int r = 1; r <= st->nrows-1; r++)
		{
			for(int c = 1; c <= st->ncols-1; c++)
			{
				st->map[r][c].is_wall = aux[r][c];

				if((st->map[r][c].is_wall)) mapa[r][c] = '#';
				else mapa[r][c] = '.';
			}
		}
	}	

	// colocação aleatoria do wormhole que permitirá ao player avançar pro próximo nível
	int wormholeX;
	int wormholeY;
	bool wormhole_placed = false;

	srand(time(NULL)); 
	do
	{
		wormholeX = rand() % (st->ncols);
		wormholeY = rand() % (st->nrows);
	}
	while (mapa[wormholeY][wormholeX] != '.' || (radius_count(st, wormholeY, wormholeX, 2) <= 20) || (radius_count(st, wormholeY, wormholeX, 1) <= 5) || (radius_count(st, wormholeY, wormholeX, 3) <= 40));
	wormhole_placed = true;

	if(wormhole_placed) 
	{
		mapa[wormholeY][wormholeX] = 'O';
		st->map[wormholeY][wormholeX].is_portal = true;
	}

	// colocação aleatoria do player no mapa numa posição valida
	srand(time(NULL)); 
	do
	{
		st->playerX = rand() % (st->ncols);
		st->playerY = rand() % (st->nrows);
	}
	while (mapa[st->playerY][st->playerX] != '.' || (radius_count(st, st->playerY, st->playerX, 2) <= 20) || (radius_count(st, st->playerY, st->playerX, 1) <= 5) || (radius_count(st, st->playerY, st->playerX, 3) <= 40));

	// responsável por spawnar monstros aleatoriamente pelo mapa
	srand(time(NULL));
    int i = 0;

    while (i < n_monstros)
    {
        do
        {
            ms[i].monstroY = rand() % (st->nrows);
            ms[i].monstroX = rand() % (st->ncols);
        } while (mapa[ms[i].monstroY][ms[i].monstroX] != '.');


		int tipo = rand() % 2;
		if(tipo == 0)
		{
			ms[i].monstro_hp = 20;
        	mapa[ms[i].monstroY] [ms[i].monstroX] = 'G';
        	ms[i].cor = ((rand() % 3) + 1);
			ms[i].msT = 0;
		}
		else if(tipo == 1)
		{
			ms[i].monstro_hp = 40;
        	mapa[ms[i].monstroY] [ms[i].monstroX] = 'S';
        	ms[i].cor = ((rand() % 3) + 1);
			ms[i].msT = 1;
		}
        i++;
    }

}





//--------------------------------------------------------------MONSTROS---------------------------------------------------//

/*
* a104614 - Marco Soares Goncalves 
* a104275 - Andre Filipe Soares Pereira 
* Verificar se a bola de fogo atinge um monstro.
*/

int detect_bullet(BULLET *b, STATE *st, MONSTER ms[], int n_monstros, int save, char (*mapa)[st->ncols], WINDOW *gameplay)
{
	int chance = rand() % 5;

    for(int i = 0; i < n_monstros; i++)
    {
        if((mapa[b->bulletY][b->bulletX+1] == 'G'   && save == 6 && ms[i].monstroY == b->bulletY && ms[i].monstroX == b->bulletX+1 && ms[i].msT == 0) || 
		   (mapa[b->bulletY][b->bulletX+1] == 's'   && save == 6 && ms[i].monstroY == b->bulletY && ms[i].monstroX == b->bulletX+1 && ms[i].msT == 2))
        {
            ms[i].monstro_hp = ms[i].monstro_hp - st->player_damage;
            if(ms[i].monstro_hp < 1)
            {
                mapa[ms[i].monstroY][ms[i].monstroX] = '.';
                mvwaddch(gameplay,ms[i].monstroY, ms[i].monstroX, '.');
                st->player_gold+=2;

				if(chance == 1 && st->n_potion < 5) st->n_potion++;
            }
            return 1;
        }
        else if((mapa[b->bulletY][b->bulletX-1] == 'G' && save == 4 && ms[i].monstroY == b->bulletY && ms[i].monstroX == b->bulletX-1 && ms[i].msT == 0) ||
				 (mapa[b->bulletY][b->bulletX-1] == 's' && save == 4 && ms[i].monstroY == b->bulletY && ms[i].monstroX == b->bulletX-1 && ms[i].msT == 2))
        {
            ms[i].monstro_hp = ms[i].monstro_hp - st->player_damage;
            if(ms[i].monstro_hp < 1)
            {
                mapa[ms[i].monstroY][ms[i].monstroX] = '.';
                mvwaddch(gameplay,ms[i].monstroY, ms[i].monstroX, '.');
                st->player_gold+=2;

				if(chance == 1 && st->n_potion < 5) st->n_potion++;
            }
            return 1;
        }
		else if((mapa[b->bulletY+1][b->bulletX] == 'G' && save == 2 && ms[i].monstroY == b->bulletY+1 && ms[i].monstroX == b->bulletX && ms[i].msT == 0) ||
		 		(mapa[b->bulletY+1][b->bulletX] == 's' && save == 2 && ms[i].monstroY == b->bulletY+1 && ms[i].monstroX == b->bulletX && ms[i].msT == 2))
        {
            ms[i].monstro_hp = ms[i].monstro_hp - st->player_damage;
            if(ms[i].monstro_hp < 1)
            {
                mapa[ms[i].monstroY][ms[i].monstroX] = '.';
                mvwaddch(gameplay,ms[i].monstroY, ms[i].monstroX, '.');
                st->player_gold+=2;

				if(chance == 1 && st->n_potion < 5) st->n_potion++;
            }
            return 1;
        }
        else if((mapa[b->bulletY-1][b->bulletX] == 'G' && save == 8 && ms[i].monstroY == b->bulletY-1 && ms[i].monstroX == b->bulletX && ms[i].msT == 0) ||
				(mapa[b->bulletY-1][b->bulletX] == 's' && save == 8 && ms[i].monstroY == b->bulletY-1 && ms[i].monstroX == b->bulletX && ms[i].msT == 2))
        {
            ms[i].monstro_hp = ms[i].monstro_hp - st->player_damage;
            if(ms[i].monstro_hp < 1)
            {
                mapa[ms[i].monstroY][ms[i].monstroX] = '.';
                mvwaddch(gameplay,ms[i].monstroY, ms[i].monstroX, '.');
                st->player_gold+=2;

				if(chance == 1 && st->n_potion < 5) st->n_potion++;
            }
            return 1;
        }
    }
    return 0;
}


/*
* a104093 - Leonardo Gomes Alves
* a104520 - Salvador Duarte Magalhaes Barreto 
* Meele damage do jogador.
*/
void meeleDamage(STATE *st, MONSTER ms[], int n_monstros, char (*mapa)[st->ncols], WINDOW *gameplay)
{
	int chance = rand() % 5;

	if(mapa[st->playerY+1][st->playerX] == 'G' || mapa[st->playerY+1][st->playerX] == 'S' || mapa[st->playerY+1][st->playerX] == 's'){
		for(int i = 0; i < n_monstros; i++)
		{
			if(ms[i].monstroY == st->playerY+1 && ms[i].monstroX == st->playerX){
				ms[i].monstro_hp -= st->player_damage *2; 

				if(ms[i].monstro_hp < 1)
            	{
            	    mapa[ms[i].monstroY][ms[i].monstroX] = '.';
					st->map[ms[i].monstroY][ms[i].monstroX].is_viewable = 0;
            	    mvwaddch(gameplay,ms[i].monstroY, ms[i].monstroX, '.');
            	    st->player_gold+=2;

					if(chance == 1 && st->n_potion < 5) st->n_potion++;
            	}
			}
			
		}
	}

	if(mapa[st->playerY+1][st->playerX+1] == 'G' || mapa[st->playerY+1][st->playerX+1] == 'S' || mapa[st->playerY+1][st->playerX+1] == 's'){
		for(int i = 0; i < n_monstros; i++)
		{
			if(ms[i].monstroY == st->playerY+1 && ms[i].monstroX == st->playerX+1){
				ms[i].monstro_hp -= st->player_damage *2; 

				if(ms[i].monstro_hp < 1)
            	{
            	    mapa[ms[i].monstroY][ms[i].monstroX] = '.';
					st->map[ms[i].monstroY][ms[i].monstroX].is_viewable = 0;
            	    mvwaddch(gameplay,ms[i].monstroY, ms[i].monstroX, '.');
            	    st->player_gold+=2;

					if(chance == 1 && st->n_potion < 5) st->n_potion++;
            	}
			}
			
		}
	}

	if(mapa[st->playerY+1][st->playerX-1] == 'G' || mapa[st->playerY+1][st->playerX-1] == 'S' || mapa[st->playerY+1][st->playerX-1] == 's'){
		for(int i = 0; i < n_monstros; i++)
		{
			if(ms[i].monstroY == st->playerY+1 && ms[i].monstroX == st->playerX-1){
				ms[i].monstro_hp -= st->player_damage *2; 

				if(ms[i].monstro_hp < 1)
            	{
            	    mapa[ms[i].monstroY][ms[i].monstroX] = '.';
					st->map[ms[i].monstroY][ms[i].monstroX].is_viewable = 0;
            	    mvwaddch(gameplay,ms[i].monstroY, ms[i].monstroX, '.');
            	    st->player_gold+=2;

					if(chance == 1 && st->n_potion < 5) st->n_potion++;
            	}
			}
			
		}
	}

	if(mapa[st->playerY][st->playerX+1] == 'G' || mapa[st->playerY][st->playerX+1] == 'S' || mapa[st->playerY][st->playerX+1] == 's'){
		for(int i = 0; i < n_monstros; i++)
		{
			if(ms[i].monstroY == st->playerY && ms[i].monstroX == st->playerX+1){
				ms[i].monstro_hp -= st->player_damage *2; 

				if(ms[i].monstro_hp < 1)
            	{
            	    mapa[ms[i].monstroY][ms[i].monstroX] = '.';
					st->map[ms[i].monstroY][ms[i].monstroX].is_viewable = 0;
            	    mvwaddch(gameplay,ms[i].monstroY, ms[i].monstroX, '.');
            	    st->player_gold+=2;

					if(chance == 1 && st->n_potion < 5) st->n_potion++;
            	}
			}
			
		}
	}

	if(mapa[st->playerY][st->playerX-1] == 'G' || mapa[st->playerY][st->playerX-1] == 'S' || mapa[st->playerY][st->playerX-1] == 's'){
		for(int i = 0; i < n_monstros; i++)
		{
			if(ms[i].monstroY == st->playerY && ms[i].monstroX == st->playerX-1){
				ms[i].monstro_hp -= st->player_damage *2; 

				if(ms[i].monstro_hp < 1)
            	{
            	    mapa[ms[i].monstroY][ms[i].monstroX] = '.';
					st->map[ms[i].monstroY][ms[i].monstroX].is_viewable = 0;
            	    mvwaddch(gameplay,ms[i].monstroY, ms[i].monstroX, '.');
            	    st->player_gold+=2;

					if(chance == 1 && st->n_potion < 5) st->n_potion++;
            	}
			}
			
		}
	}

	if(mapa[st->playerY-1][st->playerX] == 'G' || mapa[st->playerY-1][st->playerX] == 'S' || mapa[st->playerY-1][st->playerX] == 's'){
		for(int i = 0; i < n_monstros; i++)
		{
			if(ms[i].monstroY == st->playerY-1 && ms[i].monstroX == st->playerX){
				ms[i].monstro_hp -= st->player_damage *2; 

				if(ms[i].monstro_hp < 1)
            	{
            	    mapa[ms[i].monstroY][ms[i].monstroX] = '.';
					st->map[ms[i].monstroY][ms[i].monstroX].is_viewable = 0;
            	    mvwaddch(gameplay,ms[i].monstroY, ms[i].monstroX, '.');
            	    st->player_gold+=2;

					if(chance == 1 && st->n_potion < 5) st->n_potion++;
            	}
			}
			
		}
	}

	if(mapa[st->playerY-1][st->playerX+1] == 'G' || mapa[st->playerY-1][st->playerX+1] == 'S' || mapa[st->playerY-1][st->playerX+1] == 's'){
		for(int i = 0; i < n_monstros; i++)
		{
			if(ms[i].monstroY == st->playerY-1 && ms[i].monstroX == st->playerX+1){
				ms[i].monstro_hp -= st->player_damage *2; 

				if(ms[i].monstro_hp < 1)
            	{
            	    mapa[ms[i].monstroY][ms[i].monstroX] = '.';
					st->map[ms[i].monstroY][ms[i].monstroX].is_viewable = 0;
            	    mvwaddch(gameplay,ms[i].monstroY, ms[i].monstroX, '.');
            	    st->player_gold+=2;

					if(chance == 1 && st->n_potion < 5) st->n_potion++;
            	}
			}
			
		}
	}

	if(mapa[st->playerY-1][st->playerX-1] == 'G' || mapa[st->playerY-1][st->playerX-1] == 'S' || mapa[st->playerY-1][st->playerX-1] == 's'){
		for(int i = 0; i < n_monstros; i++)
		{
			if(ms[i].monstroY == st->playerY-1 && ms[i].monstroX == st->playerX-1){
				ms[i].monstro_hp -= st->player_damage *2; 

				if(ms[i].monstro_hp < 1)
            	{
            	    mapa[ms[i].monstroY][ms[i].monstroX] = '.';
					st->map[ms[i].monstroY][ms[i].monstroX].is_viewable = 0;
            	    mvwaddch(gameplay,ms[i].monstroY, ms[i].monstroX, '.');
            	    st->player_gold+=2;

					if(chance == 1 && st->n_potion < 5) st->n_potion++;
            	}
			}
			
		}
	}
}

/*
* a104614 - Marco Soares Goncalves 
* a104275 - Andre Filipe Soares Pereira 
* Spawn de slimes bebes.
*/

void spawn_slimes(STATE *st,BULLET *b, MONSTER ms[], int n_monstros, char (*mapa)[st->ncols], WINDOW *gameplay, int save)
{
	for(int i = 0; i < n_monstros; i++)
    {
        if((mapa[b->bulletY][b->bulletX+1] == 'S' && save == 6 && ms[i].monstroY == b->bulletY && ms[i].monstroX == b->bulletX+1 && ms[i].msT == 1) ||
		   (mapa[b->bulletY][b->bulletX-1] == 'S' && save == 4 && ms[i].monstroY == b->bulletY && ms[i].monstroX == b->bulletX-1 && ms[i].msT == 1) ||
		   (mapa[b->bulletY+1][b->bulletX] == 'S' && save == 2 && ms[i].monstroY == b->bulletY+1 && ms[i].monstroX == b->bulletX && ms[i].msT == 1) ||
		   (mapa[b->bulletY-1][b->bulletX] == 'S' && save == 8 && ms[i].monstroY == b->bulletY-1 && ms[i].monstroX == b->bulletX && ms[i].msT == 1))
        {
            ms[i].monstro_hp = ms[i].monstro_hp - st->player_damage;
            if(ms[i].monstro_hp < 1)
            {
			    st->player_gold += 3;
				int max_s = rand() % 7;
				int counter = 0;
                for(int j = 0; j < n_monstros && counter < max_s; j++)
			    {
					if(ms[j].monstro_hp < 1)
					{
						ms[j].monstro_hp = 15;
						ms[j].msT = 2;
						if(mapa[ms[i].monstroY - 1][ms[i].monstroX] == '.')
						{
							ms[j].monstroY = ms[i].monstroY--;
							ms[j].monstroX = ms[i].monstroX;
						}
						else if(mapa[ms[i].monstroY + 1][ms[i].monstroX] == '.')
						{
							ms[j].monstroY = ms[i].monstroY++;
							ms[j].monstroX = ms[i].monstroX;
						}
						else if(mapa[ms[i].monstroY ][ms[i].monstroX - 1] == '.')
						{
							ms[j].monstroY = ms[i].monstroY;
							ms[j].monstroX = ms[i].monstroX--;
						}
						else if(mapa[ms[i].monstroY][ms[i].monstroX + 1] == '.')
						{
							ms[j].monstroY = ms[i].monstroY;
							ms[j].monstroX = ms[i].monstroX++;
						}
						else if(mapa[ms[i].monstroY - 1][ms[i].monstroX-1] == '.')
						{
							ms[j].monstroY = ms[i].monstroY--;
							ms[j].monstroX = ms[i].monstroX--;
						}
						else if(mapa[ms[i].monstroY + 1][ms[i].monstroX + 1] == '.')
						{ 
							ms[j].monstroY = ms[i].monstroY++;
							ms[j].monstroX = ms[i].monstroX++;
						}
						else if(mapa[ms[i].monstroY - 1][ms[i].monstroX + 1] == '.')
						{
							ms[j].monstroY = ms[i].monstroY--;
							ms[j].monstroX = ms[i].monstroX++;
						}
						else if(mapa[ms[i].monstroY + 1][ms[i].monstroX - 1] == '.')
						{
							ms[j].monstroY = ms[i].monstroY++;
							ms[j].monstroX = ms[i].monstroX--;
						}
						mapa[ms[j].monstroY][ms[j].monstroX] = 's';
						wattron(gameplay, COLOR_PAIR(1) | A_BOLD);	
						mvwaddch(gameplay,ms[j].monstroY, ms[j].monstroX, 's');
						wattron(gameplay, COLOR_PAIR(1) | A_BOLD);	

						counter++;
					}
							
			    }
            }
		}	
	}
}





//-------------------------------------------------- MOVIMENTAÇÃO DOS MONSTROS ---------------------------------------//
/*
* a104614 - Marco Soares Goncalves 
* a104275 - Andre Filipe Soares Pereira 
* Movimento dos monstros.
*/
void monster_movement(STATE *st, MONSTER ms[], int n_monstros, char (*mapa)[st->ncols], WINDOW *gameplay)
{
	 for(int i = 0; i < n_monstros; i++)
	 {
 		int dist = st->map[ms[i].monstroY][ms[i].monstroX].dist;
 		if(dist < 50 && dist > 1)
 		{
 			int x = ms[i].monstroX;
 			int y = ms[i].monstroY;
 			if(dist > st->map[ms[i].monstroY][ms[i].monstroX+1].dist && ms[i].monstro_hp > 0 && mapa[y][x+1] == '.')
 			{
 				mvwaddch(gameplay,ms[i].monstroY, ms[i].monstroX, '.');
 				mapa[y][x] = '.';
 				ms[i].monstroX++;
 				if (ms[i].msT == 0) 
				{
					mapa[y][x+1] = 'G';
				}
				if (ms[i].msT == 1)
				{
					mapa[y][x+1] = 'S';
				}
				if (ms[i].msT == 2)
				{
					mapa[y][x+1] = 's';
				}
 			}
 			else if(dist > st->map[ms[i].monstroY][ms[i].monstroX-1].dist && ms[i].monstro_hp > 0 && mapa[y][x-1] == '.')
 			{
 				mvwaddch(gameplay,ms[i].monstroY, ms[i].monstroX, '.');
 				mapa[y][x] = '.';
 				ms[i].monstroX--;
 				if (ms[i].msT == 0) 
				{
					mapa[y][x-1] = 'G';
				}
				if (ms[i].msT == 1)
				{
					mapa[y][x-1] = 'S';
				}
				if (ms[i].msT == 2)
				{
					mapa[y][x-1] = 's';
				}
 			}
 			else if(dist > st->map[ms[i].monstroY-1][ms[i].monstroX].dist && ms[i].monstro_hp > 0 && mapa[y-1][x] == '.')
 			{
 				mvwaddch(gameplay,ms[i].monstroY, ms[i].monstroX, '.');
 				mapa[y][x] = '.';
 				ms[i].monstroY--;
 				if (ms[i].msT == 0) 
				{
					mapa[y-1][x] = 'G';
				}
				if (ms[i].msT == 1)
				{
					mapa[y-1][x] = 'S';
				}
				if (ms[i].msT == 2)
				{
					mapa[y-1][x] = 's';
				}
 			}
 			else if(dist > st->map[ms[i].monstroY+1][ms[i].monstroX].dist && ms[i].monstro_hp > 0 && mapa[y+1][x] == '.') 
 			{
 				mvwaddch(gameplay,ms[i].monstroY, ms[i].monstroX, '.');
 				mapa[y][x] = '.';
 				ms[i].monstroY++;
 				if (ms[i].msT == 0) 
				{
					mapa[y+1][x] = 'G';
				}
				if (ms[i].msT == 1)
				{
					mapa[y+1][x] = 'S';
				}
				if (ms[i].msT == 2)
				{
					mapa[y+1][x] = 's';
				}
 			}
 			else if(dist > st->map[ms[i].monstroY-1][ms[i].monstroX-1].dist && ms[i].monstro_hp > 0 && mapa[y-1][x-1] == '.')
 			{
 				mvwaddch(gameplay,ms[i].monstroY, ms[i].monstroX, '.');
 				mapa[y][x] = '.';
 				ms[i].monstroX--;
 				ms[i].monstroY--;
 				if (ms[i].msT == 0) 
				{
					mapa[y-1][x-1] = 'G';
				}
				if (ms[i].msT == 1)
				{
					mapa[y-1][x-1] = 'S';
				}
				if (ms[i].msT == 2)
				{
					mapa[y-1][x-1] = 's';
				}
 			}
 			else if(dist > st->map[ms[i].monstroY+1][ms[i].monstroX+1].dist && ms[i].monstro_hp > 0 && mapa[y+1][x+1] == '.')
 			{
 				mvwaddch(gameplay,ms[i].monstroY, ms[i].monstroX, '.');
 				mapa[y][x] = '.';
 				ms[i].monstroX++;
 				ms[i].monstroY++;
 				if (ms[i].msT == 0) 
				{
					mapa[y+1][x+1] = 'G';
				}
				if (ms[i].msT == 1)
				{
					mapa[y+1][x+1] = 'S';
				}
				if (ms[i].msT == 2)
				{
					mapa[y+1][x+1] = 's';
				}
 			}
 			else if(dist > st->map[ms[i].monstroY+1][ms[i].monstroX-1].dist && ms[i].monstro_hp > 0 && mapa[y+1][x-1] == '.')
 			{
 				mvwaddch(gameplay,ms[i].monstroY, ms[i].monstroX, '.');
 				mapa[y][x] = '.';
 				ms[i].monstroX--;
 				ms[i].monstroY++;
 				if (ms[i].msT == 0) 
				{
					mapa[y+1][x-1] = 'G';
				}
				if (ms[i].msT == 1)
				{	
					mapa[y+1][x-1] = 'S';
				}
				if (ms[i].msT == 2)
				{
					mapa[y+1][x-1] = 's';
				}
 			}
 			else if(dist > st->map[ms[i].monstroY-1][ms[i].monstroX+1].dist && ms[i].monstro_hp > 0 && mapa[y-1][x+1] == '.')
 			{
 				mvwaddch(gameplay,ms[i].monstroY, ms[i].monstroX, '.');
 				mapa[y][x] = '.';
 				ms[i].monstroX++;
 				ms[i].monstroY--;
 				if (ms[i].msT == 0) 
				{
					mapa[y-1][x+1] = 'G';
				}
				if (ms[i].msT == 1)
				{
					mapa[y-1][x+1] = 'S';
				}
				if (ms[i].msT == 2)
				{
					mapa[y-1][x+1] = 's';
				}
 			}
 		}
	 }
}