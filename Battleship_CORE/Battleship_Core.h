#include <iostream>
#include <utility>
#include <tuple>
#include <string>
#include <windows.h>

using namespace std;

#ifndef Battleship_Core_H
#define Battleship_Core_H

#define NUM_OF_GAMES 10
//#define HUMAN_MODE
//#define DELAY

#define FAIL		0
#define MISS		1
#define HIT			2
#define HITNSUNK	3 

#define CARRIER		'5'
#define BATTLESHIP	'4'
#define SUBMARINE	'3'
#define CRUISER		'2'
#define DESTROYER	'1'

class Game_Board
{
	int	ship_stats[5];
public:

	char			my_ships[10][10];
	char			other_ships[10][10];

	Game_Board();
	void set_new_game();
	int take_hit(pair<int, int> pos);
	int update_radar(pair<int, int> pos, int success);
	bool isDead();
	bool place_ship(tuple<int, int, int> pos, char type);
	void print_board();
};

class Player
{
public:
	Game_Board my_board;

	/******************************************************************************************/
	/*								METHODES OBLIGATOIRES									  */
	/******************************************************************************************/

	/*Les deux méthodes suivantes doivent etre défini par la classe du AI sinon il ne*/
	/*seras pas capable de jouer et l'engin traiteras le AI comme un joueur humain	 */

	/*Le AI doit pouvoir retourner une paire du genre <i, j> qui indique au jeu ou tirer*/
	/*Le parametre enemy_ship est un tableau de char avec les codes suivants:           */
	/*						'.'  = case vide											*/
	/*						'O'  = case touchées										*/
	/*						'X'  = case manquées										*/
	virtual pair<int, int> fire_at(char enemy_ships[10][10]);

	/*Le AI doit pouvoir retourner un tuple du genre <i, j, r> qui indique au placer un bateau*/
	/*i et j sont les coordonnés, r est la rotation 0 = vertical	1 = horizontal			  */
	/*Les parametres sont les suivants:														  */
	/*			char type:		Un char représentant le type de bateaux à placé				  */
	/*							 CARRIER	=	'5'     (size: 5)							  */
	/*							 BATTLESHIP	=	'4'		(size: 4)							  */
	/*							 SUBMARINE	=	'3'		(size: 3)							  */
	/*							 CRUISER	=	'2'     (size: 3)							  */
	/*							 DESTROYER	=	'1'		(size: 2)							  */
	/*			my_ships:		Un tableau de char représant les bateau du joeur			  */
	/*							 Case vides	=	'.'										      */
	/*							 Les autres cases on le numero du bateau assigner d'après	  */
	/*							 la table précédente (carrier '5', Battleship '4', ...)		  */
	virtual tuple<int, int, int> set_boat_at(char type, char my_ship[10][10]);

	/******************************************************************************************/
	/*								METHODES OPTIONNELS										  */
	/******************************************************************************************/

	/*Les méthodes suivantes ne sont pas obligatoires, elle permettent au AI de recevoir du    */
	/*feedback sur divers éléments du jeu qui peuvent supporter la prise de décision		   */

	/*Le méthode suivante recois un hit représentant ce qui est arrivé après avoir tirer       */
	/*Les valeures de status_code sont les suivantes:			FAIL		0				   */
	/*															MISS		1				   */
	/*															HIT			2				   */
	/*															HITNSUNK	3				   */
	/*			*FAIL indique que le coup pris est invalide (out of bounds ou déjà fait avant) */
	virtual void receive_status_code(int status_code, char enemy_ships[10][10]);

	/*Le méthode suivante recois un une pair indiquant ou l'enemy à tirer	pos(i,j)		   */
	virtual void receive_hit_pos(pair<int, int> pos);
	
	/*Le méthode indique qu'une nouvelle partie commencer									   */
	virtual void new_game_started();
};

class Game_Engine
{
	Player *A;
	Player *B;
	int num_of_turns;
public:
	Game_Engine(Player *A, Player *B);
	void main_game_loop();
	void place_ships(Player *player);
};

#endif