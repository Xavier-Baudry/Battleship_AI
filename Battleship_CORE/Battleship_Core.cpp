#include "Battleship_Core.h"

/*------------------------------------------------*/
/*					  PLAYER					  */
/*------------------------------------------------*/

pair<int, int> Player::fire_at(char enemy_ships[10][10])
{
	int x, y;
	cout << "Choisir une coordonee pour tirer" << endl << " x : ";
	cin >> x;
	cout << endl << "y : " ;
	cin >> y;
	cout << endl;
	return make_pair(x,y);
}

tuple<int, int, int> Player::set_boat_at(char type, char my_ship[10][10])
{
	string boat_type;
	switch(type)
	{
		case '5':
			boat_type = "Carrier [5]";
		break;
		case '4':
			boat_type = "Battleship [4]";
		break;
		case '3':
			boat_type = "Submarine [3]";
		break;
		case '2':
			boat_type = "Cruiser [3]";
		break;
		case '1':
			boat_type = "Destroyer [2]";
		break;
	}
	int x, y, z;
	cout << "Choisir une coordonee pour la position du bateau " << boat_type << endl << "x : ";
	cin >> x;
	cout << "y : " ;
	cin >> y;
	cout << endl <<"Choisir une rotation pour [0-3]" << endl << "z : ";
	cin >> z;
	cout << endl;
	return make_tuple(x,y,z);
}

void Player::new_game_started() {}

void Player::receive_hit_pos(pair<int, int> pos) {}

void Player::receive_status_code(int status_code, char enemy_ships[10][10])
{
	if(status_code == HIT)
		cout << "Toucher" << endl;
	if(status_code == HITNSUNK)
		cout << "Toucher & Couler" << endl;
	if(status_code == MISS)
		cout << "Manquer" << endl;
}

/*------------------------------------------------*/
/*					GAME ENGINE					  */
/*------------------------------------------------*/

Game_Engine::Game_Engine(Player *A, Player *B)
{
	this->A = A;
	this->B = B;
}

void Game_Engine::main_game_loop()
{
	int A_games_won = 0;
	int B_games_won = 0;

	for(int i = 0; i < NUM_OF_GAMES; i++)
	{
		A->my_board.set_new_game();
		B->my_board.set_new_game();

		num_of_turns = 0;

		A->new_game_started(); B->new_game_started();

		place_ships(A);
 		place_ships(B);

		/*Game Loop*/
		while(1)
		{
			/*Player A plays*/
			num_of_turns++;

			int status_code = FAIL;
			while(status_code == FAIL)
			{
#ifdef HUMAN_MODE
				cout << "Player A : ";	
#endif
				pair<int, int> pos = A->fire_at(A->my_board.other_ships);
				status_code = B->my_board.take_hit(pos);
				B->receive_hit_pos(pos);
				if(status_code != FAIL)
					A->my_board.update_radar(pos, status_code);
			}
		
			A->receive_status_code(status_code, A->my_board.other_ships);

			/*Check for winner*/
			if(A->my_board.isDead())
			{
				B_games_won++;
				break;
			}
			else if(B->my_board.isDead())
			{
				A_games_won++;
				break;
			}

			/*Player B plays*/
			status_code = FAIL;
			while(status_code == FAIL)
			{
#ifdef HUMAN_MODE
				cout << "Player B : ";	
#endif	
				pair<int, int> pos = B->fire_at(B->my_board.other_ships);
				status_code = A->my_board.take_hit(pos);
				A->receive_hit_pos(pos);
				if(status_code != FAIL)
					B->my_board.update_radar(pos, status_code);
			}
		
			system("cls");
			cout << "======================== Player A =========================" << endl << endl;
			A->my_board.print_board();
			cout << "======================== Player B =========================" << endl << endl;
			B->my_board.print_board();
		
			B->receive_status_code(status_code, B->my_board.other_ships);

			/*Check for winner*/
			if(A->my_board.isDead())
			{
				B_games_won++;
				cout << "Player B won." << endl;
				cout << num_of_turns << " turns played!" << endl;
				break;
			}
			else if(B->my_board.isDead())
			{
				A_games_won++;
				cout << "Player A won." << endl;
				cout << num_of_turns << " turns played!" << endl;
				break;
			}

			/*delay*/
#ifdef DELAY
			Sleep(2000);
#endif
		}
	}
	int exit;
	cout << "Player A won " << A_games_won << " games!" << endl;
	cout << "Player B won " << B_games_won << " games!" << endl;
	
	cin >> exit;
}

void Game_Engine::place_ships(Player *player)
{
#ifdef HUMAN_MODE
	player->my_board.print_board();
#endif
	while(!player->my_board.place_ship(player->set_boat_at(CARRIER, player->my_board.my_ships), CARRIER));
#ifdef HUMAN_MODE
	player->my_board.print_board();
#endif
	while(!player->my_board.place_ship(player->set_boat_at(BATTLESHIP, player->my_board.my_ships), BATTLESHIP));
#ifdef HUMAN_MODE
	player->my_board.print_board();
#endif
	while(!player->my_board.place_ship(player->set_boat_at(SUBMARINE, player->my_board.my_ships), SUBMARINE));
#ifdef HUMAN_MODE
	player->my_board.print_board();
#endif
	while(!player->my_board.place_ship(player->set_boat_at(CRUISER, player->my_board.my_ships), CRUISER));
#ifdef HUMAN_MODE
	player->my_board.print_board();
#endif
	while(!player->my_board.place_ship(player->set_boat_at(DESTROYER, player->my_board.my_ships), DESTROYER));
#ifdef HUMAN_MODE
	player->my_board.print_board();
#endif
}

/*------------------------------------------------*/
/*					GAME BOARD					  */
/*------------------------------------------------*/

Game_Board::Game_Board()
{
	set_new_game();
}

void Game_Board::set_new_game()
{
	ship_stats[0] = 2; ship_stats[1] = 3;
	ship_stats[2] = 3; ship_stats[3] = 4;
	ship_stats[4] = 5;

	for(int i = 0; i < 10; i++)
		for(int j = 0; j < 10; j++)
		{
			my_ships[i][j] = '.';
			other_ships[i][j] = '.';
		}
}

int Game_Board::take_hit(pair<int, int> pos)
{
	if(my_ships[pos.first][pos.second] == 'X' || 
	   my_ships[pos.first][pos.second] == 'O' ||
	   pos.first >= 10						  ||
	   pos.second >= 10)
		return FAIL;

	if(my_ships[pos.first][pos.second] == '.')
	{
		my_ships[pos.first][pos.second] = 'X';
		return MISS;
	}

	if(ship_stats[(my_ships[pos.first][pos.second])-49] != 1)
	{
		ship_stats[my_ships[pos.first][pos.second]-49]--;
		my_ships[pos.first][pos.second] = 'O';
		return HIT;
	}
	else
	{	
		ship_stats[(my_ships[pos.first][pos.second])-49]--;
		my_ships[pos.first][pos.second] = 'O';
		return HITNSUNK;
	}
}

int Game_Board::update_radar(pair<int, int> pos, int status)
{
	if(status == HIT || status == HITNSUNK)
		other_ships[pos.first][pos.second] = 'O';
	else
		other_ships[pos.first][pos.second] = 'X';

	return status;
}

bool Game_Board::isDead()
{
	return (ship_stats[0] == 0 &&
		    ship_stats[1] == 0 &&
			ship_stats[2] == 0 &&
			ship_stats[3] == 0 &&
			ship_stats[4] == 0 );
}

bool Game_Board::place_ship(tuple<int, int, int> pos, char type)
{
	int mod;

	if(type == CARRIER)
		mod = 5;

	if(type == BATTLESHIP)
		mod = 4;

	if(type == SUBMARINE || type == CRUISER)
		mod = 3;

	if(type == DESTROYER)
		mod = 2;

	if(get<2>(pos) == 0)
	{ // vertical
			if((int)(get<0>(pos)+mod-1) <= 9)
			{
				/*Check for empty*/
				for(int i = 0; i < mod; i++)
					if(my_ships[get<0>(pos)+i][get<1>(pos)] != '.')
						return false;

				for(int i = 0; i < mod; i++)
					my_ships[get<0>(pos)+i][get<1>(pos)] = type;
				return true;
			}
	}
	else if(get<2>(pos) == 1)
	{
			if((int)(get<1>(pos)+mod-1) <= 9)
			{
				/*Check for empty*/
				for(int i = 0; i < mod; i++)
					if(my_ships[get<0>(pos)][get<1>(pos)+i] != '.')
						return false;

				for(int i = 0; i < mod; i++)
					my_ships[get<0>(pos)][get<1>(pos)+i] = type;
				return true;
			}
	}
	return false; //failure to place ship
}

void Game_Board::print_board()
{
	
	cout << "\tMy Ships \t\t Radar" << endl << endl;

	for(int i = 0; i < 10; i++)
	{
		for(int j = 0; j < 10; j++)
			cout << my_ships[i][j] << " ";

		cout << "\t";

		for(int j = 0; j < 10; j++)
			cout << other_ships[i][j] << " ";

		cout << endl;
	}
	cout << endl << endl;
}