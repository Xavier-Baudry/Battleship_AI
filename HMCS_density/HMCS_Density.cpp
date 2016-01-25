#include "HMCS_Density.h"
#include <iomanip>

class State_Wrapper
{
	/*This class only servers a wrapper for the 'sunk_ship' board so we can quickly*/
	/*check if we visited a state already										   */
public:
	State_Wrapper(int board[10][10])
	{
		for(int i = 0; i < 10; i++)
			for(int j = 0; j < 10; j++)
				this_state[i][j] = board[i][j];
	}
	int this_state[10][10];

	bool operator==(const State_Wrapper &other) const 
	{
   		for(int i = 0; i < 10; i++)
			for(int j = 0; j < 10; j++)
				if(this_state[i][j] != other.this_state[i][j])
					return false;
		return true;
	}
};

HMCS_Density::HMCS_Density()
{
	srand((int)time(0)+42);

	games_played = 0;

	/*Initialize hit maps*/
	for(int i = 0; i < 10; i++)
		for(int j = 0; j < 10; j++)
			e_hit_map[i][j] = m_hit_map[i][j] = 0;

	/*Add corner bias*/
	e_hit_map[0][0] = e_hit_map[0][9] = e_hit_map[9][9] = e_hit_map[9][0] = CORNER_BIAS;
}

/*std::sort helper function*/
/*Compare to positions, returns position with highest hit chance (heat map)*/
bool sorter(pair<double, pair<int, int> > A, pair<double, pair<int, int> > B)
{
	return A.first > B.first;
}

/*std::sort helper function*/
/*Compare to game state, returns the game with the least untagged spot(11)*/
bool stateSorter(pair<int, State_Wrapper> A, pair<int, State_Wrapper> B)
{
	return A.first < B.first;
}

/******************************************************/
/*				REQUIRED GAME METHODS				  */
/******************************************************/

tuple<int, int, int> HMCS_Density::set_boat_at(char type, char my_ship[10][10])
{
	/*Get ship lenght*/
	int mod;
	if(type == CARRIER)
		mod = 5;
	if(type == BATTLESHIP)
		mod = 4;
	if(type == SUBMARINE || type == CRUISER)
		mod = 3;
	if(type == DESTROYER)
		mod = 2;

	/*Find squares (if any) that a ship should NOT go into*/
	double probability_map[10][10];
	for(int i = 0; i < 10; i++)
		for(int j = 0; j < 10; j++)
			if(my_ship[i][j] != '.')
				probability_map[i][j] = -1.0;
			else
				if(games_played > MIN_GAME_ANALYSIS)
					probability_map[i][j]  =  e_hit_map[i][j];
				else
					probability_map[i][j]  = 0;
	
	/*Find best cluster*/
	vector<tuple<int, int, int> > possible_coords;
	
	while(possible_coords.size() == 0)
	{
		
		int rotation = 0;
		double best_prob = DBL_MAX;

		for(int i = 0; i < 10; i++)
			for(int j = 0; j < 10; j++)
			{
				/*unusable square*/
				if(probability_map[i][j] == -1)
					continue;

				double sum = 0;
				bool failed = false;
				/*check possibility for vertical*/
				if(i + mod <= 9)
				{
					for(int x = 0; x < mod; x++)
					{

						/*check for side-by-side*/
						if((j < 9 && probability_map[i+x][j+1] == -1) ||
							(j > 0 && probability_map[i+x][j-1] == -1))
						{
							failed = true;
							break;
						}

						if(probability_map[i+x][j] != -1)
							sum += probability_map[i+x][j];
						else
						{
							failed = true;
							break;
						}
					}

					if(!failed)
						if(sum <= best_prob)
						{
							best_prob = sum;
							possible_coords.push_back(make_tuple(i, j, 0));
						}
				}

				sum = probability_map[i][j];
				/*check possibility for horizontal*/
				if(j + mod <= 9)
				{
					for(int x = 1; x < mod; x++)
					{
						/*check for side-by-side*/
						if((j < 9 && probability_map[i+1][j+x] == -1) ||
							(j > 0 && probability_map[i-1][j+x] == -1))
						{
							failed = true;
							break;
						}

						if(probability_map[i][j+x] != -1)
							sum += probability_map[i][j+x];
						else
						{
							failed = true;
							break;
						}
					}

					if(!failed)
						if(sum <= best_prob)
						{
							best_prob = sum;
							possible_coords.push_back(make_tuple(i, j, 1));
						}
			}
		}
	}

	if(games_played > MIN_GAME_ANALYSIS)
		return possible_coords[possible_coords.size()-1]; //Best position at the end
	else
		return possible_coords[rand()%possible_coords.size()]; //No analysis, send random
}

pair<int, int> HMCS_Density::fire_at(char enemy_ships[10][10])
{
	double heat_map[10][10];
	/*initialise heat map to 0*/
	for(int i = 0; i < 10; i++)
		for(int j = 0; j < 10; j++)
			heat_map[i][j] = 0;

	/*Ship types*/
	prepare_ship_remaining();

	if(mode == HUNT)
	{
		int num_pos = 0;
		/*calculate ship densities*/
		for(int k = 0; k < 5; k++)
		{
			if(ship_to_hit[k] == 0)
				continue;

			int ship_size = ship_to_hit[k];

			for(int i = 0; i < 10; i++)
				for(int j = 0; j < 10; j++)
				{
					if(enemy_ships[i][j] != '.')
						continue;

					/*check for vertical*/
					if(i + ship_size-1< 10)
					{
						num_pos++;
						bool failed = false;
						for(int x = 0; x < ship_size; x++)
							if(enemy_ships[i+x][j] != '.')
								failed = true;

						if(!failed)
							for(int x = 0; x < ship_size; x++)
								heat_map[i+x][j]++;
					}

					/*check for horizontal*/
					if(j + ship_size-1 < 10)
					{
						num_pos++;
						bool failed = false;
						for(int x = 0; x < ship_size; x++)
							if(enemy_ships[i][j+x] != '.')
								failed = true;

						if(!failed)
							for(int x = 0; x < ship_size; x++)
								heat_map[i][j+x]++;
					}
				}
				
		}//end ship density

#ifdef DEBUG
	/*PRINT HEAT MAP*/
	cout << endl << endl;
	cout << setprecision(3) << fixed;
		for(int i = 0; i < 10; i++)
		{
			for(int j = 0; j < 10; j++)
				cout << heat_map[i][j] << " ";
			cout << endl;
		}
	cout << endl << endl;
#endif

		/*Ship densities evaluated, find largest density*/
		double largest = 0;
		for(int i = 0; i < 10; i++)
				for(int j = 0; j < 10; j++)
					if(heat_map[i][j] > largest)
						largest = heat_map[i][j];

		if(largest == 0)
		{
			/*If largest == 0, we are in an invalid config*/
			prepare_for_backtrack();
			tagg_ships();
			return fire_at(enemy_ships);
		}


		/*Devide densities by largest density*/
		for(int i = 0; i < 10; i++)
			for(int j = 0; j < 10; j++)
				heat_map[i][j] = heat_map[i][j]/largest;

#ifdef DEBUG
	/*PRINT HEAT MAP*/
	cout << endl << endl;
	cout << setprecision(3) << fixed;
		for(int i = 0; i < 10; i++)
		{
			for(int j = 0; j < 10; j++)
				cout << heat_map[i][j] << " ";
			cout << endl;
		}
	cout << endl << endl;
#endif

		/*Begin weighting player moves*/
		if(games_played > MIN_GAME_ANALYSIS)
		{
			for(int i = 0; i < 10; i++)
				for(int j = 0; j < 10; j++)
					if(enemy_ships[i][j] == '.')
						heat_map[i][j] = (0.6*heat_map[i][j])+(0.4*(((double)(m_hit_map[i][j]))/(double)games_played));
		}

#ifdef DEBUG
	/*PRINT HEAT MAP*/
	cout << endl << endl;
	cout << setprecision(3) << fixed;
		for(int i = 0; i < 10; i++)
		{
			for(int j = 0; j < 10; j++)
				cout << heat_map[i][j] << " ";
			cout << endl;
		}
	cout << endl << endl;
#endif

		/*find largest probability that is in the correct parity*/
		vector<pair<double, pair<int, int> > > coords;

		/*find coordinates that respect the firing window size*/
		int line_count = smallest_ship_size;
		for(int i = 0; i < 10; i++)
		{
			for(int j = line_count-1; j < 10; j++)
			{
					pair<int, int> coord = make_pair(i, j);
					coords.push_back(make_pair(heat_map[i][j], coord));
					j += smallest_ship_size-1;
			}
			line_count--;

			if(line_count == 0)//reset mod
				line_count = smallest_ship_size;
		}

		sort(coords.begin(), coords.end(), sorter);

		if(coords.size() > 0)
		{
			double largest = coords[0].first;
			vector<pair<double, pair<int, int> > > vals;
			for(int i = 0; i < coords.size(); i++)
				if(coords[i].first == largest)
					vals.push_back(coords[i]);
				else
					break;

			last_shot = vals[rand()%vals.size()].second;
		
		}
		else
		{
			//We made a mistake, we have to attempt to backtrack
			prepare_for_backtrack();
			tagg_ships();
			return fire_at(enemy_ships);
		}
	}
	else
	{
		/*Destroy Mode*/
		vector<pair<int, int> > plausible_targets;		//Vector to hold all possible targets after hitting a ship

		for(int i = 0; i < 10; i++)
			for(int j = 0; j < 10; j++)
				if(sunk_ship[i][j] == 11)
				{
					if((i - 1 >= 0) && (enemy_ships[i - 1][j] == '.'))
						plausible_targets.push_back(make_pair(i - 1, j));
					if((i + 1 < 10) && (enemy_ships[i + 1][j] == '.'))
						plausible_targets.push_back(make_pair(i + 1, j));
					if((j - 1 >= 0) && (enemy_ships[i][j - 1] == '.'))
						plausible_targets.push_back(make_pair(i, j - 1));
					if((j + 1 < 10) && (enemy_ships[i][j + 1] == '.'))
						plausible_targets.push_back(make_pair(i, j + 1));
				}

			/*No more targets to hunt*/
			if(plausible_targets.size() == 0)
			{
				mode = HUNT;
				return fire_at(enemy_ships);
			}

			/*calculate ship densities*/
			for(int k = 0; k < 5; k++)
			{
				if(ship_to_hit[k] == 0)
					continue;

				int ship_size = ship_to_hit[k];

				for(int i = 0; i < 10; i++)
					for(int j = 0; j < 10; j++)
					{
						if(enemy_ships[i][j] == 'X')
							continue;

						/*check for vertical*/
						if(i + ship_size-1< 10)
						{
							bool failed = false;
							for(int x = 0; x < ship_size; x++)
								if(enemy_ships[i+x][j] == 'X')
									failed = true;

							if(!failed)
								for(int x = 0; x < ship_size; x++)
									heat_map[i+x][j]++;
						}

						/*check for horizontal*/
						if(j + ship_size-1 < 10)
						{
							bool failed = false;
							for(int x = 0; x < ship_size; x++)
								if(enemy_ships[i][j+x] == 'X')
									failed = true;

							if(!failed)
								for(int x = 0; x < ship_size; x++)
									heat_map[i][j+x]++;
						}
					}
			}

			for(int i = 0; i < 10; i++)
				for(int j = 0; j < 10; j++)
					if(enemy_ships[i][j] == 'O')
							heat_map[i][j] = 0;

#ifdef DEBUG
		/*PRINT HEAT MAP*/
		cout << endl << endl;
		cout << setprecision(3) << fixed;
			for(int i = 0; i < 10; i++)
			{
				for(int j = 0; j < 10; j++)
					cout << heat_map[i][j] << " ";
				cout << endl;
			}
		cout << endl << endl;
#endif
			/*Look ammong targets*/
			double best_val = 0;
			pair<int, int> best_pos;
				for(unsigned int x = 0; x < plausible_targets.size(); x++)
					if(heat_map[plausible_targets[x].first][plausible_targets[x].second] > best_val)
					{
						best_pos = plausible_targets[x];
						best_val = heat_map[plausible_targets[x].first][plausible_targets[x].second];
					}

					if(best_val == 0)
					{
						/*We are in a wrong config, backtrack*/
						prepare_for_backtrack();
						tagg_ships();
						return fire_at(enemy_ships);
					}

			last_shot = best_pos;
		}
		
	return last_shot;
}

/******************************************************/
/*				OPTIONAL GAME METHODS				  */
/******************************************************/

void HMCS_Density::receive_status_code(int status_code, char enemy_ships[10][10])
{
	if(status_code == HITNSUNK)
	{
		sunk_spot.push_back(last_shot);
		sunk_ship[last_shot.first][last_shot.second] = 12;
		m_hit_map[last_shot.first][last_shot.second]++;

		/*Figure out what we sunk*/
		tagg_ships(); 
	}
	else if(status_code == HIT)
	{
		/*to retrieve what ship we sunk, later*/
		sunk_ship[last_shot.first][last_shot.second] = 11;
		m_hit_map[last_shot.first][last_shot.second]++;

		mode = DESTROY;
	}
}	

void HMCS_Density::new_game_started()
{
	/*Reset game variables*/
	mode = HUNT;
	ship_to_hit[0] = 2; ship_to_hit[1] = 3; ship_to_hit[2] = 3; ship_to_hit[3] = 4; ship_to_hit[4] = 5;
	visited_states.clear();
	sunk_spot.clear();

	/*Calculate hit importance*/
	if(games_played != 0)
	{
		double mean = 0;
		for(int i = 0; i < 10; i++)
			for(int j = 0; j < 10; j++)
				mean += e_temp_map[i][j];

		mean = (mean/(double)turns_this_game)/(double)turns_this_game;

		for(int i = 0; i < 10; i++)
			for(int j = 0; j < 10; j++)
				if(e_temp_map[i][j] != 0)
					e_hit_map[i][j] += ((double)(turns_this_game - (e_temp_map[i][j]-1)))/((double)turns_this_game);
				else
				{
					e_hit_map[i][j] -= mean;
					if(e_hit_map[i][j] < 0)
						e_hit_map[i][j] = 0;
				}
	}

	games_played++;
	turns_this_game = 0;

	/*reinitialize hit maps*/
	for(int i = 0; i < 10; i++)
		for(int j = 0; j < 10; j++)
		{
			e_temp_map[i][j] = 0;
			sunk_ship[i][j]  = 0;
		}
}

void HMCS_Density::receive_hit_pos(pair<int, int> pos)
{
	turns_this_game++;

	e_temp_map[pos.first][pos.second] = turns_this_game;
}

/******************************************************/
/*				CLASS METHODS						  */
/******************************************************/

/*Find ship types that are believed to be eliminated*/
void HMCS_Density::prepare_ship_remaining()
{
	/*Check for ship types*/
	int three_count = 0;
	for(int i = 0; i < 10; i++)
		for(int j = 0; j < 10; j++)
		{
			if(sunk_ship[i][j] == 2)
				ship_to_hit[0] = 0;
		
			if(sunk_ship[i][j] == 3)
			{
				three_count++;
				if(three_count == 3)
					ship_to_hit[1] = 0;
				if(three_count == 6)
					ship_to_hit[2] = 0;
			}

			if(sunk_ship[i][j] == 4)
				ship_to_hit[3] = 0;

			if(sunk_ship[i][j] == 5)
				ship_to_hit[4] = 0;
		}

	/*Smallest ship alive*/
	for(int i = 0; i < 5; i++)
		if(ship_to_hit[i] != 0)
		{
			smallest_ship_size = ship_to_hit[i];
			break;
		}
}

/*Resets all spots to unknown(11)*/
void HMCS_Density::prepare_for_backtrack()
{
	/*Save the current state*/
	State_Wrapper temp(sunk_ship);
	visited_states.push_back(temp);

	/*Reset all boats to 11 (un-identified)*/
	for(int i = 0; i < 10; i++)
		for(int j = 0; j < 10; j++)
			if(sunk_ship[i][j] != 0)
				sunk_ship[i][j] = 11;

	/*Add sink spot*/
	for(int i = 0; i < sunk_spot.size(); i++)
		sunk_ship[sunk_spot[i].first][sunk_spot[i].second] = 12;

	/*Reset ships*/
	ship_to_hit[0] = 2; ship_to_hit[1] = 3; ship_to_hit[2] = 3; ship_to_hit[3] = 4; ship_to_hit[4] = 5;
}

/*Fits ships to current missing spots(11)*/
void HMCS_Density::tagg_ships()
{
	/*Get positions that need to be tagged*/
	vector<pair<int, int> > positions;
	for(int i = 0; i < sunk_spot.size(); i++)
	{
		if(sunk_ship[sunk_spot[i].first][sunk_spot[i].second] == 12)
			positions.push_back(sunk_spot[i]);
	}

	/*Get all configs*/
	vector<pair<int, State_Wrapper> > possible_configs;
	clusterize(sunk_ship, possible_configs, positions, ship_to_hit);

	if(possible_configs.size() > 0)
	{
		/*We have all possible game states in play now, pick the one with the less unidentied remaining(11)*/
		sort(possible_configs.begin(), possible_configs.end(), stateSorter);

		State_Wrapper temp = possible_configs[0].second;

		for(int i = 0; i < 10; i++)
			for(int j = 0; j < 10; j++)
				sunk_ship[i][j] = temp.this_state[i][j];

		mode = DESTROY;
	}
	else
	{
		/*We failed to fit the ships, we need to backtrack*/
		prepare_for_backtrack();
		tagg_ships();
	}
}

/*Builds ship clusters to find best game state possible*/
void HMCS_Density::clusterize(int ships[10][10], vector<pair<int, State_Wrapper> > &possible_configs, vector<pair<int, int> > sink_spot, int ship_types[5])
{
	if(sink_spot.size() == 0)
	{
		/*Found possible config*/
		
		/*Check for unused*/
		bool alr_visited = false;
		State_Wrapper new_state(ships);
		for(int i = 0; i < visited_states.size(); i++)
		{
			if(new_state == visited_states[i])
			{
				alr_visited = true;
				break;
			}
		}
	
		if(!alr_visited)
		{
			/*Success!*/

			/*count number of 11 spots remaining*/
			int miss_spot = 0;
			for(int i = 0; i < 10; i++)
				for(int j = 0; j < 10; j++)
					if(ships[i][j] == 11)
						miss_spot++;

			possible_configs.push_back(make_pair(miss_spot, ships));
		}
		return;
	}

	/*Continue search*/
	for(int i  = 0; i < sink_spot.size(); i++)
	{
		pair<int, int> spot = sink_spot[i];

		/************find window bounds*****************/

		int highest_y_mod = 0;
		int lowest_y_mod = 0;
		/*Check up & down max distance*/
		for(int y = 1; y < 5; y++)
			if((spot.first - y >= 0) && sunk_ship[spot.first - y][spot.second] == 11)//up
				highest_y_mod = y;
			else
				break;

		for(int y = 1; y < 5; y++)
			if((spot.first + y < 10) && sunk_ship[spot.first + y][spot.second] == 11)//down
				lowest_y_mod = y;
			else break;

		int highest_x_mod = 0;
		int lowest_x_mod = 0;
		/*Check left & right max distance*/
		for(int y = 1; y < 5; y++)
			if((spot.second + y < 10) && sunk_ship[spot.first][spot.second + y] == 11)//right
				highest_x_mod = y;
			else
				break;

		for(int y = 1; y < 5; y++)
			if((spot.second - y >= 0) && sunk_ship[spot.first][spot.second - y] == 11)//left
				lowest_x_mod = y;
			else
				break;

		for(int x = 4; x >= 0; x--)
			if(ship_types[x] != 0)
			{
				/******Vertical*******/
				int ship_size = ship_types[x];
				int start_pos_y = 0;
				int low_offset = 0;

				if(highest_y_mod >= ship_size)
					start_pos_y = spot.first - ship_size+1;
				else
					start_pos_y = spot.first - highest_y_mod;

				if(lowest_y_mod >= ship_size)
					low_offset = spot.first + ship_size-1;
				else
					low_offset = spot.first + lowest_y_mod;
				
				if(low_offset-start_pos_y+1 >= ship_size)
					for(int w = 0; w <= ((low_offset-start_pos_y)+1-ship_size) ; w++)
					{
						/*set ship in place*/
						for(int t = 0; t < ship_size; t++)
							sunk_ship[start_pos_y+w+t][spot.second] = ship_size;

						/*remove ship from types*/
						int temp_val = ship_types[x];
						ship_types[x] = 0;

						/*remove sink spot*/
						vector<pair<int, int> > temp_sink;
						for(int r = 0; r < sink_spot.size(); r++)
							if(sink_spot[r] != spot)
								temp_sink.push_back(sink_spot[r]);

						clusterize(ships, possible_configs, temp_sink, ship_types);

						/*Undo changes*/
						ship_types[x] = temp_val;
						for(int t = 0; t < ship_size; t++)
							sunk_ship[start_pos_y+w+t][spot.second] = 11;
						sunk_ship[spot.first][spot.second] = 12;
					}

				/********horizontal**********/
				int start_pos_x = 0;
				int high_offset = 0;

				if(lowest_x_mod >= ship_size)
					start_pos_x = spot.second - ship_size+1;
				else
					start_pos_x = spot.second - lowest_x_mod;

				if(highest_x_mod >= ship_size)
					high_offset = spot.second + ship_size-1;
				else
					high_offset = spot.second + highest_x_mod;


				
				if(high_offset-start_pos_x+1 >= ship_size)
					for(int w = 0; w <= ((high_offset-start_pos_x)+1-ship_size) ; w++)
					{
						/*set ship in place*/
						for(int t = 0; t < ship_size; t++)
							sunk_ship[spot.first][start_pos_x+w+t] = ship_size;

						/*remove ship from types*/
						int temp_val = ship_types[x];
						ship_types[x] = 0;

						/*remove sink spot*/
						vector<pair<int, int> > temp_sink;
						for(int r = 0; r < sink_spot.size(); r++)
							if(sink_spot[r] != spot)
								temp_sink.push_back(sink_spot[r]);

						clusterize(ships, possible_configs, temp_sink, ship_types);

						/*Undo changes*/
						ship_types[x] = temp_val;
						for(int t = 0; t < ship_size; t++)
							sunk_ship[spot.first][start_pos_x+w+t] = 11;
						sunk_ship[spot.first][spot.second] = 12;
					}
			}

	}
	
}