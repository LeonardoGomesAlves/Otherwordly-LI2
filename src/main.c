#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <stdbool.h>

#include "state.h"
#include "mapa.h"

#define ROWS 40
#define COLS 130
#define DAMAGE_TIMER 2000

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

int luzLigada = 0; //global





//--------------------------------------------------------MOVIMENTAÇÃO--------------------------------------------------------//
/*
* a104614 - Marco Soares Goncalves
* Função para movimento do player.
*/

int do_movement_action(STATE *st, int key, WINDOW *wnd, char (*mapa)[st->ncols], int save_direction) 
{
	mvwaddch(wnd, st->playerY, st->playerX, '.');	

	if ((key == KEY_UP || key == '8') && (mapa[st->playerY-1][st->playerX] == '.' || mapa[st->playerY-1][st->playerX] == 'O'))
	{
		st->playerY--;
		save_direction = 8;
		return save_direction;
	}
	if ((key == KEY_DOWN || key == '2') && (mapa[st->playerY+1][st->playerX] == '.' || mapa[st->playerY+1][st->playerX] == 'O'))
	{
		st->playerY++;
		save_direction = 2;
		return save_direction;
	}
	if ((key == KEY_LEFT || key == '4') && (mapa[st->playerY][st->playerX-1] == '.' || mapa[st->playerY][st->playerX-1] == 'O'))
	{
		st->playerX--;
		save_direction = 4;
		return save_direction;
	}
	if ((key == KEY_RIGHT || key == '6') && (mapa[st->playerY][st->playerX+1] == '.' || mapa[st->playerY][st->playerX+1] == 'O'))
	{
		st->playerX++;
		save_direction = 6;
		return save_direction;
	}
	if ((key == '9') && mapa[st->playerY-1][st->playerX+1] == '.')
	{
		st->playerY--;
		st->playerX++;
		return save_direction;
	}
	if ((key == '1') && mapa[st->playerY+1][st->playerX-1] == '.')
	{
		st->playerY++;
		st->playerX--;
		return save_direction;
	}
	if ((key == '7') && mapa[st->playerY-1][st->playerX-1] == '.')
	{
		st->playerY--;
		st->playerX--;
		return save_direction;
	}
	if ((key == '3') && mapa[st->playerY+1][st->playerX+1] == '.')
	{
		st->playerY++;
		st->playerX++;
		return save_direction;

	}
	if (key == 'q' || key == 27)
	{
		endwin(); 
		exit(0);
		return save_direction;

	}
	
	return save_direction;
}



//------------------------------------------------------- DESENHAR -------------------------------------------------------//

/*
* a104093 - Leonardo Gomes Alves
* Desenhar mob.
*/

void draw_mob (MONSTER ms, WINDOW *gameplay) {
    if (ms.msT == 0)
	{
 		wattron(gameplay, COLOR_PAIR(2) | A_BOLD);	
 		mvwaddch(gameplay, ms.monstroY, ms.monstroX, 'G' | A_BOLD); 
 		wattroff(gameplay, COLOR_PAIR(2) | A_BOLD);
	}
	if (ms.msT == 1)
	{
		wattron(gameplay, COLOR_PAIR(1) | A_BOLD);	
 		mvwaddch(gameplay, ms.monstroY, ms.monstroX, 'S' | A_BOLD); 
 		wattroff(gameplay, COLOR_PAIR(1) | A_BOLD);
	}
	if (ms.msT == 2)
	{
		wattron(gameplay, COLOR_PAIR(1) | A_BOLD);	
 		mvwaddch(gameplay, ms.monstroY, ms.monstroX, 's' | A_BOLD); 
 		wattroff(gameplay, COLOR_PAIR(1) | A_BOLD);
	}
}

/*
* a104093 - Leonardo Gomes Alves
* Draw modo showcase.
*/

void draw_Showcase(STATE *st, MONSTER ms[], WINDOW *gameplay, char (*mapa)[st->ncols], int n_monstros) {
	for (int r = 0; r < st->nrows+1; r++) {
		for (int c = 0; c < st->ncols+1; c++) {
			if (mapa[r][c] == '#') {
				wattron(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
				mvwaddch(gameplay, r, c, '#' | A_BOLD);
				wattroff(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
			}
			if (mapa[r][c] == '.') {
				wattron(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
				mvwaddch(gameplay, r, c, '.' | A_BOLD);
				wattroff(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
			}
			if (mapa[r][c] == 'O') {
				wattron(gameplay, COLOR_PAIR(4) | A_BOLD);
				mvwaddch(gameplay, r, c, 'O' | A_BOLD);
				wattroff(gameplay, COLOR_PAIR(4) | A_BOLD);
			}
			if (mapa[r][c] == 'G' || mapa[r][c] == 'S' || mapa[r][c] == 's') {
				for (int i = 0; i < n_monstros; i++) {
						if (ms[i].monstroY == r && ms[i].monstroX == c && ms[i].monstro_hp > 0) {
							draw_mob(ms[i], gameplay);
							break;
						}
					}
			}
		}
	}
}


/*
* a104093 - Leonardo Gomes Alves
* Manter a luz ligada.
*/

void drawRastoLigado_mobs (STATE *st, MONSTER ms[], WINDOW *gameplay, char (*mapa)[st->ncols], int n_monstros) {
	for (int r = 0; r < st->nrows; r++) {
		for (int c = 0; c < st->ncols; c++) {
			if (st->map[r][c].is_viewable == 0 && (mapa[r][c] == 'G' || mapa[r][c] == 'S' || mapa[r][c] == 's')) {
				for (int i = 0; i < n_monstros; i++) {
					if (ms[i].monstroY == r && ms[i].monstroX == c) {
						mvwaddch(gameplay, r, c, ' ');
					}
				}
			}
			if (st->map[r][c].is_viewable == 1 && (mapa[r][c] == 'G' || mapa[r][c] == 'S' || mapa[r][c] == 's')) {
				for (int i = 0; i < n_monstros; i++) {
					if (ms[i].monstroY == r && ms[i].monstroX == c) {
						draw_mob(ms[i], gameplay);
					}
				}
			}
		}
	}
}


/*
* a104093 - Leonardo Gomes Alves
* Desenhar a luz em volta do player.
*/

void draw_Light(STATE *st, MONSTER ms[], WINDOW *gameplay, char (*mapa)[st->ncols], int n_monstros) 
{

	for (int r = 1; r < st->nrows; r++) {
		for (int c = 1; c < st->ncols; c++) {
			if (st->map[r][c].dist != 50 && mapa[r+1][c] == '#') {
				wattron(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
				mvwaddch(gameplay, r+1, c, '#' | A_BOLD);
				wattroff(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
			}

			if (st->map[r][c].dist != 50 && mapa[r+1][c+1] == '#') {
				wattron(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
				mvwaddch(gameplay, r+1, c+1, '#' | A_BOLD);
				wattroff(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
			}

			if (st->map[r][c].dist != 50 && mapa[r+1][c-1] == '#') {
				wattron(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
				mvwaddch(gameplay, r+1, c-1, '#' | A_BOLD);
				wattroff(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
			}

			if (st->map[r][c].dist != 50 && mapa[r][c+1] == '#') {
				wattron(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
				mvwaddch(gameplay, r, c+1, '#' | A_BOLD);
				wattroff(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
			}

			if (st->map[r][c].dist != 50 && mapa[r][c-1] == '#') {
				wattron(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
				mvwaddch(gameplay, r, c-1, '#' | A_BOLD);
				wattroff(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
			}

			if (st->map[r][c].dist != 50 && mapa[r-1][c] == '#') {
				wattron(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
				mvwaddch(gameplay, r-1, c, '#' | A_BOLD);
				wattroff(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
			}

			if (st->map[r][c].dist != 50 && mapa[r-1][c+1] == '#') {
				wattron(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
				mvwaddch(gameplay, r-1, c+1, '#' | A_BOLD);
				wattroff(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
			}

			if (st->map[r][c].dist != 50 && mapa[r-1][c-1] == '#') {
				wattron(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
				mvwaddch(gameplay, r-1, c-1, '#' | A_BOLD);
				wattroff(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
			}

			if (st->map[r][c].dist != 50 && mapa[r][c] == '.') {
				wattron(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
				mvwaddch(gameplay, r, c, '.' | A_BOLD);
				wattroff(gameplay, COLOR_PAIR(COLOR_BLACK) | A_BOLD);
			} 

			if (st->map[r][c].dist != 50 && mapa[r][c] == 'O') {
				wattron(gameplay, COLOR_PAIR(4) | A_BOLD);
				mvwaddch(gameplay, r, c, 'O' | A_BOLD);
				wattroff(gameplay, COLOR_PAIR(4) | A_BOLD);
			}

			else if (st->map[r][c].dist != 50 && (mapa[r][c] == 'G' || mapa[r][c] == 'S' || mapa[r][c] == 's')) {
				for (int i = 0; i < n_monstros; i++) {
					if (ms[i].monstroY == r && ms[i].monstroX == c) {
						draw_mob(ms[i], gameplay);
						break;
					}
				}
			}
		}
	}	
}


/*
* a104093 - Leonardo Gomes Alves
* Desenhar escuro atrás do player quando ele se mexe no modo default.
*/

void draw_Escuro(STATE *st, WINDOW *gameplay, char (*mapa)[st->ncols]) {
    
    attron(COLOR_PAIR(0) | A_BOLD);
        for (int r = 0; r < st->nrows+1; r++) {
            for (int c = 0; c < st->ncols+1; c++) {
                if (st->map[r][c].dist == 50 && (mapa[r][c] == '.' || mapa[r][c] == 'G' || mapa[r][c] == 'S' || mapa[r][c] == 's' || mapa[r][c]  == '#' || mapa[r][c] == 'O')) {
                    wattron(gameplay, COLOR_PAIR(COLOR_BLACK));
                    mvwaddch(gameplay, r, c, ' ' | A_BOLD);
                    wattroff(gameplay, COLOR_PAIR(COLOR_BLACK));
                }
            }
        }
        attroff(COLOR_PAIR(0) | A_BOLD);
}

/*
* a104614 - Marco Soares Goncalves 
* a104275 - Andre Filipe Soares Pereira
* Funcoes de desenhar jogador e "bola de fogo".
*/

void draw_Player(STATE *st, WINDOW *gameplay)
{
	curs_set(0);	//Esconde o cursor
	wattron(gameplay, COLOR_PAIR(3) | A_BOLD);
	mvwaddch(gameplay, st->playerY, st->playerX, '@' | A_BOLD); //Desenha @ na posição inicial
	wattroff(gameplay, COLOR_PAIR(3) | A_BOLD);
}



void draw_bullet(BULLET *b, char c, WINDOW *gameplay)
{
	wattron(gameplay, COLOR_PAIR(1) | A_BOLD);	
	mvwaddch(gameplay, b->bulletY, b->bulletX, c);
	wattroff(gameplay, COLOR_PAIR(1) | A_BOLD);
}



//---------------------------------------------------------------------------------------------------------------------//
/*
* a104614 - Marco Soares Goncalves 
* a104275 - Andre Filipe Soares Pereira
* Funcao para atualizar a bala e para apagar o seu rasto.
*/

void do_shooting_action(BULLET *b, STATE *st, int direcao, char (*mapa)[st->ncols], WINDOW *gameplay) 
{
		if(direcao == 8 && mapa[b->bulletY-1][b->bulletX] != '#' && mapa[b->bulletY-1][b->bulletX] != 'O')
		{
			b->bulletY--;
			mvwaddch(gameplay, b->bulletY + 1, b->bulletX , '.');
		}
		else if (direcao == 2 && mapa[b->bulletY+1][b->bulletX] != '#' && mapa[b->bulletY+1][b->bulletX] != 'O')
		{
			b->bulletY++;
			mvwaddch(gameplay, b->bulletY - 1, b->bulletX, '.');
		}
		else if(direcao == 4 && mapa[b->bulletY][b->bulletX-1] != '#' && mapa[b->bulletY][b->bulletX-1] != 'O')
		{
			b->bulletX--;
			mvwaddch(gameplay, b->bulletY, b->bulletX + 1, '.');
		}
		else if(direcao == 6 && mapa[b->bulletY][b->bulletX+1] != '#' && mapa[b->bulletY][b->bulletX+1] != 'O')
		{
			b->bulletX++;
			mvwaddch(gameplay, b->bulletY, b->bulletX - 1, '.');
		} 
		draw_bullet(b,'+',gameplay);
}

/*
* a104614 - Marco Soares Goncalves 
* a104275 - Andre Filipe Soares Pereira
* Funcao para verificar o caminho da bala.
*/

int bullet_path(BULLET *b, STATE *st, int save_direction, char (*mapa)[st->ncols])
{
	if(save_direction == 8)
	{
		return ((mapa[b->bulletY-1][b->bulletX] != '#') && (mapa[b->bulletY-1][b->bulletX] != 'G') && (mapa[b->bulletY-1][b->bulletX] != 'S') && (mapa[b->bulletY-1][b->bulletX] != 's') && (mapa[b->bulletY-1][b->bulletX] != 'O'));
	}
	else if(save_direction == 2)
	{
		return ((mapa[b->bulletY+1][b->bulletX] != '#') && (mapa[b->bulletY+1][b->bulletX] != 'G') && (mapa[b->bulletY+1][b->bulletX] != 'S') && (mapa[b->bulletY+1][b->bulletX] != 's') && (mapa[b->bulletY+1][b->bulletX] != 'O'));
	}
	else if(save_direction == 4)
	{
		return ((mapa[b->bulletY][b->bulletX-1] != '#') && (mapa[b->bulletY][b->bulletX-1] != 'G') && (mapa[b->bulletY][b->bulletX-1] != 'S') && (mapa[b->bulletY][b->bulletX-1] != 's') && (mapa[b->bulletY][b->bulletX-1] != 'O'));
	}
	else if(save_direction == 6)
	{
		return ((mapa[b->bulletY][b->bulletX+1] != '#') && (mapa[b->bulletY][b->bulletX+1] != 'G') && (mapa[b->bulletY][b->bulletX+1] != 'S') && (mapa[b->bulletY][b->bulletX+1] != 's') && (mapa[b->bulletY][b->bulletX+1] != 'O'));
	}
	return 1;
}

/*
* a104614 - Marco Soares Goncalves 
* a104520 - Salvador Duarte Magalhaes Barreto 
* Funcao para detetar se o player morreu e atualizar o seu estado.
*/

int detect_death(STATE *st, char (*mapa)[st->ncols], int dano)
{
	int save_hp = st->player_hp;

	if (dano == 1)
	{
		if(mapa[st->playerY-1][st->playerX] == 'G') 
		{
			if(st->player_gold > 0){ 
				st->player_gold -= 5;
				if(st->player_gold < 0) st->player_gold = 0;
			}
			else st->player_hp -= 5;
		}
		else if(mapa[st->playerY+1][st->playerX] == 'G') 
		{
			if(st->player_gold > 0){ 
				st->player_gold -= 5;
				if(st->player_gold < 0) st->player_gold = 0;
			}
			else st->player_hp -= 5;
		}
		else if(mapa[st->playerY][st->playerX-1] == 'G') 
		{
			if(st->player_gold > 0){ 
				st->player_gold -= 5;
				if(st->player_gold < 0) st->player_gold = 0;
			}
			else st->player_hp -= 5;
		}
		else if(mapa[st->playerY][st->playerX+1] == 'G') 
		{
			if(st->player_gold > 0){ 
				st->player_gold -= 5;
				if(st->player_gold < 0) st->player_gold = 0;
			}
			else st->player_hp -= 5;
		}
		else if(mapa[st->playerY-1][st->playerX-1] == 'G') 
		{
			if(st->player_gold > 0){ 
				st->player_gold -= 5;
				if(st->player_gold < 0) st->player_gold = 0;
			}
			else st->player_hp -= 5;
		}
		else if(mapa[st->playerY-1][st->playerX+1] == 'G') 
		{
			if(st->player_gold > 0){ 
				st->player_gold -= 5;
				if(st->player_gold < 0) st->player_gold = 0;
			}
			else st->player_hp -= 5;
		}
		else if(mapa[st->playerY+1][st->playerX-1] == 'G') 
		{
			if(st->player_gold > 0){ 
				st->player_gold -= 5;
				if(st->player_gold < 0) st->player_gold = 0;
			}
			else st->player_hp -= 5;
		}
		else if(mapa[st->playerY+1][st->playerX+1] == 'G') 
		{
			if(st->player_gold > 0){ 
				st->player_gold -= 5;
				if(st->player_gold < 0) st->player_gold = 0;
			}
			else st->player_hp -= 5;
		}
		else if(mapa[st->playerY-1][st->playerX] == 'S') 
		{
			st->player_hp -= 4;
		}
		else if(mapa[st->playerY+1][st->playerX] == 'S') 
		{
			st->player_hp -= 4;
		}
		else if(mapa[st->playerY][st->playerX-1] == 'S') 
		{
			st->player_hp -= 4;
		}
		else if(mapa[st->playerY][st->playerX+1] == 'S') 
		{
			st->player_hp -= 4;
		}
		else if(mapa[st->playerY-1][st->playerX-1] == 'S') 
		{
			st->player_hp -= 4;
		}
		else if(mapa[st->playerY-1][st->playerX+1] == 'S') 
		{
			st->player_hp -= 4;
		}
		else if(mapa[st->playerY+1][st->playerX-1] == 'S') 
		{
			st->player_hp -= 4;
		}
		else if(mapa[st->playerY+1][st->playerX+1] == 'S') 
		{
			st->player_hp -= 4;
		}
		else if(mapa[st->playerY-1][st->playerX] == 's') 
		{
			st->player_hp -= 1;
		}
		else if(mapa[st->playerY+1][st->playerX] == 's') 
		{
			st->player_hp -= 1;
		}
		else if(mapa[st->playerY][st->playerX-1] == 's') 
		{
			st->player_hp -= 1;
		}
		else if(mapa[st->playerY][st->playerX+1] == 's') 
		{
			st->player_hp -= 1;
		}
		else if(mapa[st->playerY-1][st->playerX-1] == 's') 
		{
			st->player_hp -= 1;
		}
		else if(mapa[st->playerY-1][st->playerX+1] == 's') 
		{
			st->player_hp -= 1;
		}
		else if(mapa[st->playerY+1][st->playerX-1] == 's') 
		{
			st->player_hp -= 1;
		}
		else if(mapa[st->playerY+1][st->playerX+1] == 's') 
		{
			st->player_hp -= 1;
		}

		if(save_hp != st->player_hp)
		{	
			if(st->player_hp < 1) return 1;
			else return 2;
		} 
		else return 0;
	}
	else 
	{
		if(mapa[st->playerY-1][st->playerX] == 'G') 
		{
			 return 2;
		}
		else if(mapa[st->playerY+1][st->playerX] == 'G') 
		{
			 return 2;
		}
		else if(mapa[st->playerY][st->playerX-1] == 'G') 
		{
			 return 2;
		}
		else if(mapa[st->playerY][st->playerX+1] == 'G') 
		{
			 return 2;
		}
		else if(mapa[st->playerY-1][st->playerX-1] == 'G') 
		{
			 return 2;
		}
		else if(mapa[st->playerY-1][st->playerX+1] == 'G') 
		{
			 return 2;
		}
		else if(mapa[st->playerY+1][st->playerX-1] == 'G') 
		{
			 return 2;
		}
		else if(mapa[st->playerY+1][st->playerX+1] == 'G') 
		{
			 return 2;
		}
		else if(mapa[st->playerY-1][st->playerX] == 'S') 
		{
			 return 2;
		}
		else if(mapa[st->playerY+1][st->playerX] == 'S') 
		{
			 return 2;
		}
		else if(mapa[st->playerY][st->playerX-1] == 'S') 
		{
			 return 2;
		}
		else if(mapa[st->playerY][st->playerX+1] == 'S') 
		{
			 return 2;
		}
		else if(mapa[st->playerY-1][st->playerX-1] == 'S') 
		{
			 return 2;
		}
		else if(mapa[st->playerY-1][st->playerX+1] == 'S') 
		{
			 return 2;
		}
		else if(mapa[st->playerY+1][st->playerX-1] == 'S') 
		{
			 return 2;
		}
		else if(mapa[st->playerY+1][st->playerX+1] == 'S') 
		{
			 return 2;
		}
		else if(mapa[st->playerY-1][st->playerX] == 's') 
		{
			 return 2;
		}
		else if(mapa[st->playerY+1][st->playerX] == 's') 
		{
			 return 2;
		}
		else if(mapa[st->playerY][st->playerX-1] == 's') 
		{
			 return 2;
		}
		else if(mapa[st->playerY][st->playerX+1] == 's') 
		{
			 return 2;
		}
		else if(mapa[st->playerY-1][st->playerX-1] == 's') 
		{
			 return 2;
		}
		else if(mapa[st->playerY-1][st->playerX+1] == 's') 
		{
			 return 2;
		}
		else if(mapa[st->playerY+1][st->playerX-1] == 's') 
		{
			 return 2;
		}
		else if(mapa[st->playerY+1][st->playerX+1] == 's') 
		{
			 return 2;
		}
	}
	
return 0;
}



//----------------------------------------------- UI DO PLAYER ---------------------------------------------------//
/*
* a104520 - Salvador Duarte Magalhaes Barreto 
* Funcao para aumentar o dano do jogador consoante o seu "gold".
*/

void gold_usage(STATE *st, WINDOW *gameplay) // incrementar a vida
{
	srand(time(NULL));
    if (st->player_gold >= 100) 
    {
    	st->player_damage += 5;
    	st->player_gold -= 100;
    	mvwprintw(gameplay, 44, 10, "+5 damage!!");
    	wrefresh(gameplay);
    	wgetch(gameplay);  //Espera por um input do jogador
    	mvwprintw(gameplay, 44, 10, "                    ");
    	wrefresh(gameplay);
	}
    
}

/*
* a104520 - Salvador Duarte Magalhaes Barreto 
* Funcao para dar display de informacao do player.
*/

void player_UI (STATE *st, WINDOW *gameplay, int rip, int timer_dano, int melee_counter)
{
	mvwprintw(gameplay, 41, 10, "                                     ");
	mvwprintw(gameplay, 42, 10, "                                     ");
	mvwprintw(gameplay, 43, 10, "                                     ");
	mvwprintw(gameplay, 44, 10, "                                     ");

	wattron (gameplay, COLOR_PAIR(1) | A_BOLD);
	mvwprintw(gameplay, 42, 10, "HP: %d", st->player_hp);
	wattroff (gameplay, COLOR_PAIR(1) | A_BOLD);

	wattron (gameplay, COLOR_PAIR(2) | A_BOLD);
	mvwprintw(gameplay, 43, 10, "GOLD: %d", st->player_gold);
	wattroff (gameplay, COLOR_PAIR(2) | A_BOLD);

	wattron (gameplay, COLOR_PAIR(3) | A_BOLD);
	mvwprintw(gameplay, 44, 10, "DAMAGE: %d", st->player_damage);

	if(melee_counter == 100) mvwprintw(gameplay, 44, 23, "BURNING HELL READY"); 
	else mvwprintw(gameplay, 44, 23, "Charging...       "); 
	wattroff (gameplay, COLOR_PAIR(3) | A_BOLD);

	if (rip == 2)
	{
		mvwprintw(gameplay, 41, 10, "                                            ");
		mvwprintw(gameplay, 42, 23, "                                            ");

		wattron (gameplay, COLOR_PAIR(1)| A_BOLD);
		mvwprintw(gameplay, 41, 10, "Estás a levar dano!!");
		wattroff (gameplay, COLOR_PAIR(1)| A_BOLD);
		mvwprintw(gameplay, 42, 23, "Próximo ataque em: %.2f", (1 - (float)timer_dano/60) );
	}

	mvwprintw(gameplay, 42, 55, "INVENTARIO");
	mvwprintw(gameplay, 43, 55, "_____________________");
	mvwprintw(gameplay, 44, 55, "|   |   |   |   |   |");
	mvwprintw(gameplay, 45, 55, "|___|___|___|___|___|");

	wattron(gameplay, COLOR_PAIR(1) | A_BOLD);
	if(st->n_potion == 1) mvwprintw(gameplay, 44, 57, "P");
	if(st->n_potion == 2) {
		mvwprintw(gameplay, 44, 57, "P");
		mvwprintw(gameplay, 44, 61, "P");
	}
	if(st->n_potion == 3){
		mvwprintw(gameplay, 44, 57, "P");
		mvwprintw(gameplay, 44, 61, "P");
		mvwprintw(gameplay, 44, 65, "P");
	} 
	if(st->n_potion == 4){
		mvwprintw(gameplay, 44, 57, "P");
		mvwprintw(gameplay, 44, 61, "P");
		mvwprintw(gameplay, 44, 65, "P");
		mvwprintw(gameplay, 44, 69, "P");
	}
	if(st->n_potion == 5){
		mvwprintw(gameplay, 44, 57, "P");
		mvwprintw(gameplay, 44, 61, "P");
		mvwprintw(gameplay, 44, 65, "P");
		mvwprintw(gameplay, 44, 69, "P");
		mvwprintw(gameplay, 44, 73, "P");
	}
	wattroff (gameplay, COLOR_PAIR(1)| A_BOLD);
}



//-------------------------------------------------------- UPDATE ------------------------------------------------------------//

/*
* a104093 - Leonardo Gomes Alves
* a104614 - Marco Soares Goncalves 
* a104275 - Andre Filipe Soares Pereira
* a104520 - Salvador Duarte Magalhaes Barreto 
* Função update.
*/

int update(STATE *st, BULLET *b, MONSTER ms[], int n_monstros, int key, WINDOW *gameplay, char (*mapa)[st->ncols], int timer_dano, int mob_counter, int melee_counter) 
{

	key = wgetch(gameplay);	//Recebe o input do utilizador 
	int direcao = 6;
	int rip = 0;
	int save_direction = 0;
	int inter = 0;
	
	while ((key = wgetch(gameplay)) != 71 || (key = wgetch(gameplay)) != 27)
	{
		if (luzLigada == 2 && inter == 0) {
			draw_Showcase(st,ms,gameplay,mapa, n_monstros);
			inter++;
			}
		if (luzLigada == 0) {draw_Escuro(st,gameplay, mapa);}
    	for (int i = 0; i < st->nrows+1; i++) { //dps do floodfill, dá reset às dist
            for (int j = 0; j < st->ncols+1; j++) {
                st->map[i][j].dist = 50;
            }
        } 
			
		if(mob_counter == 10)
		{
			floodfill(st->playerY, st->playerX, 8, 8, st, gameplay, mapa); 
			mob_counter = 0;
			int check_pos = detect_death(st, mapa, 0); 
			monster_movement(st, ms, n_monstros, mapa, gameplay);
			
			if(check_pos == 0) rip = detect_death(st, mapa, 1);

			if(rip == 1) break;

			if (luzLigada == 0) draw_Escuro(st,gameplay, mapa);
			if (luzLigada == 1) drawRastoLigado_mobs(st, ms, gameplay, mapa, n_monstros);
		}

		wtimeout(gameplay, DAMAGE_TIMER/60);
		timer_dano++;

		if (timer_dano == 60) timer_dano = 0;

		direcao = do_movement_action(st, key, gameplay, mapa, save_direction);

		if (direcao != save_direction) rip = detect_death (st, mapa, 1);
	
		save_direction = direcao;
		int hit = 0;

		floodfill(st->playerY, st->playerX, 8, 8, st, gameplay, mapa);
		if(luzLigada == 0 || luzLigada == 1) draw_Light(st, ms, gameplay, mapa, n_monstros);
		if (luzLigada == 2) draw_Showcase(st,ms,gameplay,mapa,n_monstros);
		draw_Player(st, gameplay);
		gold_usage(st, gameplay);

		wrefresh(gameplay);

		if (timer_dano == 0) 
		{
			rip = detect_death(st, mapa, 1); 
		}

		if(rip == 1) break;

		if(st->map[st->playerY][st->playerX].is_portal == true) return 2;

		// --------------------- update da bala ---------------------------- //
		if(key == 'r')
		{
			b->bulletX = st->playerX;
			b->bulletY = st->playerY;
			
	        draw_bullet(b, '+', gameplay);		
			draw_Player(st, gameplay);
			do_movement_action(st, key, gameplay, mapa, save_direction);
			wtimeout(gameplay, 50);

			while (bullet_path(b, st, save_direction, mapa) == 1 && hit == 0)
			{
				direcao = do_movement_action(st, key, gameplay, mapa, save_direction);
				do_shooting_action(b, st, save_direction, mapa, gameplay);			
				draw_Player(st, gameplay);
				hit = detect_bullet(b, st, ms, n_monstros, save_direction, mapa, gameplay);
				spawn_slimes(st,b,ms, n_monstros, mapa,gameplay,save_direction);	
				player_UI(st, gameplay, rip, timer_dano, melee_counter);
				wrefresh(gameplay);
				key = wgetch(gameplay);
			}
			wtimeout(gameplay, -1);
			mvwaddch(gameplay, b->bulletY, b->bulletX, '.');
			draw_Player(st, gameplay);
			wrefresh(gameplay);
		}

		// ------------------------ update do meele ---------------------- //
		if(key == 'e')
		{
			if(melee_counter == 100){
				meeleDamage(st, ms, n_monstros, mapa, gameplay);
				melee_counter = 0;
			}
		}
		
		// ------------------------ update das poções -------------------- //
		if(key == 'h')
		{
			if(st->n_potion > 0){
				st->player_hp += 10;
				if(st->player_hp > 100) st->player_hp = 100;
			}
			st->n_potion--;
		}

		player_UI(st, gameplay, rip, timer_dano, melee_counter);
		gold_usage(st, gameplay);
		mob_counter++;
		if(melee_counter != 100) melee_counter++;
		wrefresh(gameplay);
	} 

	if(rip == 1)
	{
		wclear(gameplay);
		wrefresh(gameplay);
		return -1;
	}
	wtimeout(gameplay, -1);
	return 0;
}



//--------------------------------------------------------------MENU--------------------------------------------------------//


/*
* a104614 - Marco Soares Goncalves 
* a104275 - Andre Filipe Soares Pereira
* a104093 - Leonardo Gomes Alves
* Inicio do jogo para aparecer aquela animação
*/
void cutScene()
{
	initscr();
    noecho();
    curs_set(0);
    start_color();
	flushinp(); // limpa o buffer do teclado

	init_color(AMARELO, 900, 900, 0);
	init_pair(1, AMARELO, BLACK);
	init_pair(2, AMARELO, BLACK);

    WINDOW *cutscene = newwin(42, 131, 0, 0);
 
	wattron(cutscene, COLOR_PAIR(1));
	int i = 0;
	int i1 = 131;

	while (i1 > 53)
	{
		wclear(cutscene);
		mvwprintw(cutscene, 4, i, " _____ _____ _   _  ___________ _    _  ___________ _    ______ _   __   __\n");
		mvwprintw(cutscene, 5, i, "|  _  |_   _| | | ||  ___| ___ \\ |  | ||  _  | ___ \\ |   |  _  \\ |  \\ \\ / /\n");
		mvwprintw(cutscene, 6, i, "| | | | | | | |_| || |__ | |_/ / |  | || | | | |_/ / |   | | | | |   \\ V / \n");
		mvwprintw(cutscene, 7, i, "| | | | | | |  _  ||  __||    /| |/\\| || | | |    /| |   | | | | |    \\ / \n");
		mvwprintw(cutscene, 8, i, "\\ \\_/ / | | | | | || |___| |\\\\ \\  /\\  /\\ \\_/ / |\\ \\| |___| |/ /| |____| |  \n");
		mvwprintw(cutscene, 9, i, " \\___/  \\_/ \\_| |_/\\____/\\_| \\_|\\/  \\/  \\___/\\_| \\_\\_____/___/ \\_____/\\_/  \n");
	 
		 
		mvwprintw(cutscene, 10, i1, "______ _   _ _   _ _____  _____ _____ _   _ \n");
		mvwprintw(cutscene, 11, i1, "|  _  \\ | | | \\ | |  __ \\|  ___|  _  | \\ | |  \n");
		mvwprintw(cutscene, 12, i1, "| | | | | | |  \\| | |  \\/| |__ | | | |  \\| | \n");
		mvwprintw(cutscene, 13, i1, "| | | | | | | . ` | | __ |  __|| | | | . ` |  \n");
		mvwprintw(cutscene, 14, i1, "| |/ /| |_| | |\\  | |_\\ \\| |___\\ \\_/ / |\\  | \n");
		mvwprintw(cutscene, 15, i1, "|___/  \\___/\\_| \\_/\\____/\\____/ \\___/\\_| \\_/  \n");
		usleep(60000);
		wrefresh(cutscene);

		if(i < 20)
		{
				i++;
		}
		i1--;
	}

	wattroff(cutscene,COLOR_PAIR(1));
}


/*
* a104614 - Marco Soares Goncalves 
* a104275 - Andre Filipe Soares Pereira 
* a104093 - Leonardo Gomes Alves
* Menu principal
*/
int menu()
{
    initscr();
    noecho();
    curs_set(0);
    start_color();
	flushinp(); // limpa o buffer do teclado

	init_color(AMARELO, 900, 900, 0);
	init_pair(1, AMARELO, BLACK);
	init_pair(2, AMARELO, BLACK);

    WINDOW *menu = newwin(42, 131, 0, 0);
 
	wattron(menu, COLOR_PAIR(1));
		
	wclear(menu);
	mvwprintw(menu, 4, 20, " _____ _____ _   _  ___________ _    _  ___________ _    ______ _   __   __\n");
	mvwprintw(menu, 5, 20, "|  _  |_   _| | | ||  ___| ___ \\ |  | ||  _  | ___ \\ |   |  _  \\ |  \\ \\ / /\n");
	mvwprintw(menu, 6, 20, "| | | | | | | |_| || |__ | |_/ / |  | || | | | |_/ / |   | | | | |   \\ V / \n");
	mvwprintw(menu, 7, 20, "| | | | | | |  _  ||  __||    /| |/\\| || | | |    /| |   | | | | |    \\ / \n");
	mvwprintw(menu, 8, 20, "\\ \\_/ / | | | | | || |___| |\\\\ \\  /\\  /\\ \\_/ / |\\ \\| |___| |/ /| |____| |  \n");
	mvwprintw(menu, 9, 20, " \\___/  \\_/ \\_| |_/\\____/\\_| \\_|\\/  \\/  \\___/\\_| \\_\\_____/___/ \\_____/\\_/  \n");
	 
		 
	mvwprintw(menu, 10, 52, "______ _   _ _   _ _____  _____ _____ _   _ \n");
	mvwprintw(menu, 11, 52, "|  _  \\ | | | \\ | |  __ \\|  ___|  _  | \\ | |  \n");
	mvwprintw(menu, 12, 52, "| | | | | | |  \\| | |  \\/| |__ | | | |  \\| | \n");
	mvwprintw(menu, 13, 52, "| | | | | | | . ` | | __ |  __|| | | | . ` |  \n");
	mvwprintw(menu, 14, 52, "| |/ /| |_| | |\\  | |_\\ \\| |___\\ \\_/ / |\\  | \n");
	mvwprintw(menu, 15, 52, "|___/  \\___/\\_| \\_/\\____/\\____/ \\___/\\_| \\_/  \n");
		
	wrefresh(menu);
	wattroff(menu,COLOR_PAIR(1));
                                                                           
    mvwprintw(menu, 18, 60, "Jogar");
    mvwprintw(menu, 19, 60, "Ajuda");
    mvwprintw(menu, 20, 60, "Sair");
	mvwprintw(menu, 23, 43, "Pressiona ENTER para selecionar uma opção!");


    keypad(menu, TRUE);

    int key;
    int selected = 0;

    do //Coloca a opção selecionada a piscar e em negrito
	{
        for (int i = 0; i < 4; i++) 
		{
            if (i == selected) 
			{
                wattron(menu, A_BLINK | A_BOLD);	//Caso esteja selecionado ativamos os atributos
				wattron(menu, COLOR_PAIR(1));
            }

			switch (i)	//Conforme o i em que nos encontramos movemos o cursor para a opção correta
			{
			case 0:
            		mvwprintw(menu, 18, 60, "Jogar");
					break;
			case 1:
            		mvwprintw(menu, 19, 60, "Ajuda");
					break;
			case 2:
            		mvwprintw(menu, 20, 60, "Opções");
					break;
			case 3: 
					mvwprintw(menu, 21, 60, "Sair");
					break;
			
			default:
					mvwprintw(menu, 18, 60, "Jogar");
    				mvwprintw(menu, 19, 60, "Ajuda");
    				mvwprintw(menu, 20, 60, "Opções");
					mvwprintw(menu, 21, 60, "Sair");
					break;
			}
            wattroff(menu, A_BLINK | A_BOLD);	//Desliga o atributo quando trocamos de opção
			wattroff(menu, COLOR_PAIR(1));
		}

        wrefresh(menu);

        key = wgetch(menu);	//Input do utilizador

        if (key == '8' || key == KEY_UP) 
		{
            selected--;
            if (selected < 0)
			{
				selected = 3;	//Caso tentemos exceder o topo dá wormhole para baixo
			} 
        }
        else if (key == '2' || key == KEY_DOWN) 
		{
            selected++;
            if (selected > 3)
			{
				selected = 0; //Caso tentemos exceder o minimo dá wormhole para cima
			} 
        }

    } while (key != '\n');

    endwin();
    return selected;	//Conforme este valor a minha main decide o que vai fazer
}

/*
* a104614 - Marco Soares Goncalves 
* a104520 - Salvador Duarte Magalhaes Barreto 
* Menu de "ajuda".
*/
void ajuda ()
{
	initscr();
    noecho();
    curs_set(0);
    start_color();

	WINDOW *ajuda = newwin(42, 131, 0, 0);
	box(ajuda, 0, 0);

	keypad(ajuda,TRUE);
    int key;


	mvwprintw(ajuda, 3, 10, "Bem vindo ao OTHERWORDLY DUNGEON!!");
	mvwprintw(ajuda, 5, 10, "Vais embarcar na jornada de Valerian, um mago aprisionado numa masmorra.");
	mvwprintw(ajuda, 6, 10, "Explora as profundezas sinistras, derrota monstros e melhora as tuas habilidades!");
	mvwprintw(ajuda, 8, 10, "O teu objetivo é encontrar o portal de cada nível da masmorra, para que possas avançar para o seguinte.");
	mvwprintw(ajuda, 9, 10, "Quanto mais avanças, mais monstros vais encontrar!!");
	mvwprintw(ajuda, 11, 10, "Para te movimentares pelo mapa podes utilizar as setas do teu teclado ou");
	mvwprintw(ajuda, 12, 10, "o numpad(as teclas 1,3,7 e 9 serevem para te movimentares nas diagonais!).");
	mvwprintw(ajuda, 14, 10, "Os monstros não vão descansar até te alcançarem, e assim que te avistarem começaram a perseguir-te!");		
	mvwprintw(ajuda, 15, 10, "Para te defenderes podes usar as TUAS HABILIDADES:");
	mvwprintw(ajuda, 17, 12, "'r' -> conjuga uma FIREBALL, um projétil de fogo que reduzirá os monstros a cinzas!");
	mvwprintw(ajuda, 19, 12, "'e' -> conjuga BURNING HELL, uma aura de chamas ao redor do jogador que causa o dobro do dano");
	mvwprintw(ajuda, 20, 19, "(extremamente eficaz contra slimes). Mas atenção, uma habilidade tão poderosa precisa de recarregar!");
	mvwprintw(ajuda, 23, 10, "Derrota monstros para obteres ouro. Ao coletar 100, o teu dano aumentará!");
	mvwprintw(ajuda, 24, 10, "Alem disso, monstros podem ocasionalmente deixar poções de cura que podem ser usadas pressionando 'h'.");	
	mvwprintw(ajuda, 27, 10, "BESTIARIO:");
	mvwprintw(ajuda, 29, 12, "'G' -> GOBLIN, um monstro astuto e ganacioso com 20 pontos de vida. Ao atacar, rouba 5 de ouro.");
	mvwprintw(ajuda, 30, 19, "Caso não tenhas ouro ele inflige 5 pontos de dano diretamente na tua vida!");
	mvwprintw(ajuda, 32, 12, "'S' -> SLIME, uma gelatina gigante com 40 pontos de vida. Os seus ataques causam 4 pontos de dano.");
	mvwprintw(ajuda, 33, 19, "Cuidado! Ao derrotá-lo, ele pode se dividir até 7 mini-slimes, 's', cada um com 15 pontos de vida.");
	mvwprintw(ajuda, 34, 19, "Os mini-slimes são ágeis e infligem 1 ponto de dano cada.");
	mvwprintw(ajuda, 36, 10, "Já tens o conhecimento necessário para começar esta aventura!");
	mvwprintw(ajuda, 38, 10, "Boa sorte e diverte-te!!");
	mvwprintw(ajuda, 40, 51, "Pressiona Q para voltar a trás");


	while(1){
		key = wgetch(ajuda);

		if (key == 'q') {
			endwin();
			break;
		}
	}
}


/*
* a104093 - Leonardo Gomes Alves
* a104614 - Marco Soares Goncalves 
* a104275 - Andre Filipe Soares Pereira 
* Menu opções.
*/
void opcoes () {
	initscr();
	noecho();
	curs_set(0);
	start_color();

	WINDOW *opcoes = newwin(42,131,0,0);
	box(opcoes, 0, 0);

	keypad(stdscr,TRUE);
	int key;

	if (luzLigada == 0) mvwprintw(opcoes, 21, 53, "Modo de Luz: Ligado  ");
	if (luzLigada == 1) mvwprintw(opcoes, 21, 53, "Modo de Luz: Rasto   ");
	if (luzLigada == 2) mvwprintw(opcoes, 21, 53, "Modo de Luz: Showcase");

	mvwprintw(opcoes, 22, 44, "Pressiona ENTER para mudar o modo da luz");
	mvwprintw(opcoes, 40, 51, "Pressiona Q para voltar a trás");

	
	
	while (1) {
		key = wgetch(opcoes);
		if (key == '\n') {
			luzLigada++;
			if (luzLigada == 3) luzLigada = 0;
			if (luzLigada == 0)
				mvwprintw(opcoes, 21, 53, "Modo de Luz: Ligado  ");
			if (luzLigada == 1)
				mvwprintw(opcoes, 21, 53, "Modo de Luz: Rasto   ");
			if (luzLigada == 2)
				mvwprintw(opcoes, 21, 53, "Modo de Luz: Showcase");
			
			mvwprintw(opcoes, 22, 44, "Pressiona ENTER para mudar o modo da luz");
		} 
		if (key == 'q') {
			endwin();
			break;
		}

	}
}


/*
* a104614 - Marco Soares Goncalves 
* a104520 - Salvador Duarte Magalhaes Barreto 
* Menu de game over.
*/
int game_over (STATE *st)
{
	initscr();
    noecho();
    curs_set(0);
    start_color();

	WINDOW *gameOver = newwin(42, 131, 0, 0);

	keypad(gameOver, TRUE);
	
	mvwprintw(gameOver, 14, 38, " _____   ___  ___  ___ _____   _____  _   _ ___________ \n");
	mvwprintw(gameOver, 15, 38, "|  __ \\ / _ \\ |  \\/  ||  ___| |  _  || | | |  ___| ___ \\\n");
	mvwprintw(gameOver, 16, 38, "| |  \\// /_\\ \\| .  . || |__   | | | || | | | |__ | |_/ /\n");
	mvwprintw(gameOver, 17, 38, "| | __ |  _  || |\\/| ||  __|  | | | || | | |  __||    /\n");
	mvwprintw(gameOver, 18, 38, "| |_\\ \\| | | || |  | || |___  \\ \\_/ /\\ \\_/ / |___| |\\ \\\n");
	mvwprintw(gameOver, 19, 38, " \\____/\\_| |_/\\_|  |_/\\____/   \\___/  \\___/\\____/\\_| \\_|\n");

	mvwprintw(gameOver, 23, 50, "Atingiste o nível: %d da masmorra.", st->lvl); 

	wrefresh(gameOver);
	
	sleep(4);
	return 3;
}

// acresecentar o texto de fim de jogo;

//-------------------------------------------------------------MAIN--------------------------------------------------------//

/*
* a104093 - Leonardo Gomes Alves
* a104614 - Marco Soares Goncalves 
* a104275 - Andre Filipe Soares Pereira
* a104520 - Salvador Duarte Magalhaes Barreto 
* Função main do jogo.
*/

int main() 
{
	cutScene();

	bool running = true;

	int selecao = menu();

	while(running)
	{
		int finish = 0;
		
		while(selecao != 0)
		{
			if(selecao == 1)
			{
				ajuda();
				selecao = menu();
			}
			if(selecao == 2)
			{
				opcoes();
				selecao = menu();
			}
			if(selecao == 3){
				finish = 1;
				running = false;
				break;
			}
		}


		if(finish == 1)
		{
			endwin();
		}
		else
		{
			// inicializar a state com valores padrão
			STATE st;
			st.playerX = 0;
			st.playerY = 0; 
			st.player_hp = 100;
			st.player_damage = 10;
			st.player_gold = 0;
			st.lvl = 1;
			st.ncols = 130; // x 
			st.nrows = 40; // y
			st.prob_wall = 0.30;
			st.n_potion = 0;
			// Inicializar a matriz com valores padrão
			for(int r = 0; r < st.nrows; r++)
			{
				for(int c = 0; c < st.ncols; c++)
				{
					st.map[r][c].is_wall = false;
					st.map[r][c].is_portal = false;
					st.map[r][c].dist = 50;
					st.map[r][c].is_viewable = 0;
				}
			}

			BULLET b;

			int n_monstros = 30;
			MONSTER ms[1000];

			WINDOW *gameplay = newwin(st.nrows+7, st.ncols+1, st.playerY, st.playerX); 
			// o +7= +1+6 onde 6 é o tamanho do ui do player
		
			initscr();

			int key;
			start_color();	
			cbreak();
			noecho();
			nonl();
			intrflush(gameplay, false);
			keypad(gameplay, true);

			init_color(BLUE, 0, 600, 1000);
			init_color(FLOOR, 0, 0, 0);
			init_color(WALL, 1000, 1000, 1000);
			init_color(AMARELO, 1000, 1000, 0);

			init_pair(4, BLUE, BLACK);
			init_pair(1, RED, BLACK);
			init_pair(3, GREEN, BLACK);
			init_pair(2, AMARELO, BLACK);
			init_color(FLOOR, 1000, 1000, 1000);
			init_color(WALL, 800, 800, 800);
			init_pair(PORTAL, BLUE, BLACK);
			init_pair(CHAO, WALL, BLACK);
			init_pair(PAREDE, FLOOR, BLACK);

			key = wgetch(gameplay);

			char mapa[st.nrows + 1][st.ncols + 1];

			generate_map(&st, ms, n_monstros, mapa);
			
			int check;
			int mob_counter = 0;
			int melee_counter = 0;
			while(1) 
			{
			    check = update(&st, &b, ms, n_monstros, key, gameplay, mapa, 0, mob_counter, melee_counter);

				if (check == 2) {
					for(int r = 0; r < st.nrows+1; r++)
					{
						for(int c = 0; c < st.ncols+1; c++)
						{
							st.map[r][c].is_wall = false;
							st.map[r][c].is_portal = false;
							st.map[r][c].dist = 50;
							mvwaddch(gameplay, r, c, ' ');
						}
					}

					st.lvl++;

					n_monstros += 10;
				
					generate_map(&st, ms, n_monstros, mapa);
				}
			    if(check == -1) break;
			}
			if(check == -1)
			{
				selecao = game_over(&st);
			}
		}
	}

	return 0;
}