#include "Battleship_Core.h"
#include <ctime>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <utility>
#include <tuple>
#include <algorithm>

using namespace std;

class State_Wrapper;

//#define DEBUG

#ifndef HMCS_Density_H
#define HMCS_Density_H

#define CORNER_BIAS			5
#define MIN_GAME_ANALYSIS	10

#define HUNT	false
#define DESTROY	true

class HMCS_Density : public Player
{
	/*Data collecting*/
	double	e_hit_map[10][10];  //Where the enemy shot probabilities (overall)
	int		m_hit_map[10][10];  //Where we hit the enemy (overall)
	int		games_played;

	/*Current game data*/
	bool			mode;							//false = hunt		true = destroy
	int				ship_to_hit[5];					//#ships left to hit this game (and their lenght)
	int				turns_this_game;				//# of turns this game 
	int				e_temp_map[10][10];				//Hold order where enemy shot on us (current game)
	int				sunk_ship[10][10];				//Map used to approximate which ship has been sunk
	int				smallest_ship_size;				//Smallest ship 'alive'
	pair<int, int>	last_shot;						//temporary holder while shot not confirmed ok
	vector<pair<int, int> > sunk_spot;				//spots where we sunk a ship
	vector<State_Wrapper>	visited_states;			//Vector to hold visited state when backtracking

	/********************************/
	/*		 Private Methods		*/
	/********************************/

	/*Sets all ships to un-tagged(11) and remarks hit spot (12)*/
	void prepare_for_backtrack();
	/*Attempts to fit the ships using the hit spots (12)*/
	void tagg_ships();
	/*Count the ship type remaining and set smallest window*/
	void prepare_ship_remaining();
	/*Recursive function to tag ships*/
	void clusterize(int ships[10][10], vector<pair<int, State_Wrapper> > &possible_configs, vector<pair<int, int> > sink_spot, int ship_types[5]);

public:
	HMCS_Density();

/*************************************************/
/*				Game interface*					 */
/*************************************************/

pair<int, int> fire_at(char enemy_ships[10][10]);

tuple<int, int, int> set_boat_at(char type, char my_ship[10][10]);

void receive_status_code(int status_code, char enemy_ships[10][10]);

void receive_hit_pos(pair<int, int> pos);

void new_game_started();

};

#endif