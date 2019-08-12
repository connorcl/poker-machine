/*
 * Project: Slot machine game for the Windows console, with basic and poker modes
 * Author: Connor Claypool
 * Date: 27-11-2018
 */

#include <iostream>
#include <string>
#include <random>
#include <algorithm>
#include <thread>
#include <chrono>
#include <cmath>
#include <Windows.h>

using namespace std;


//// SECTION: GLOBAL VARIABLES (GENERAL)

// random number generator
mt19937 rng;
// mode to play
string mode;
// current points
int points = 100;
// maximum points achieved in current game
int max_points = points;
// cost of playing one round
int cost = 20;
// columns/cards are stopped/dealt one by one, number still to stop/deal
int moving_cols = 0;
int &cards_to_deal = moving_cols;
// whether waiting for input
bool waiting = false;
// whether key is currently pressed
bool key_pressed = false;


//// SECTION: FUNCTION PROTOTYPES

// helper functions / general game functions
void move_cursor(short, short);
void clear_screen(int);
void main_menu();
void set_cost();
void start();
void play();
void score_continue();
void quit(const string&);

// basic mode functions
void initialize_lines();
void rotate_lines(int);
void render_lines();
void score_basic();
void play_basic();

// poker mode functions
void initialize_deck();
void deal_cards();
void deal_hand();
void redeal_card();
void redeal_cards();
void render_hand();
vector<int> get_ords();
bool check_flush();
bool check_straight();
int check_pairs();
int check_n_of_a_kind();
bool check_full_house();
bool check_straight_flush();
bool check_royal_flush();
void score_poker();
void play_poker();


//// SECTION: HELPER FUNCTIONS AND GENERAL GAME FUNCTIONS

// moves the console cursor to the row and column specified
void move_cursor(short col, short row) {
	// create COORD structure
	COORD pos = { col, row };
	// move cursor
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

// fills number of lines specified with 100 space characters
void clear_screen(int lines) {
	// for number of lines specified
	for (int j = 0; j < lines; j++) {
		// print 100 space characters (10 x 10)
		for (int i = 0; i < 10; i++) {
			cout << "          ";
		}
		cout << endl;
	}
}

// main menu where mode to play is selected
void main_menu() {
	// move cursor to top left corner
	move_cursor(0, 0);
	// print message
	cout << "Select mode to play. Press B for basic mode or P for poker mode." << endl;
	// set mode string based on keypress
	waiting = true;
	while (waiting) {
		if (GetKeyState('B') & 0x8000) {
			waiting = false;
			mode = "basic";
		}
		else if (GetKeyState('P') & 0x8000) {
			waiting = false;
			mode = "poker";
		}
	}
	// start game
	start();
}

// set cost of playing based on current points
void set_cost() {
	// cost is 20 or 10% of current points, whichever is more
	cost = max(20, points / 10);
}

// prints points and game instructions (based on mode), gives options to continue or quit
void start() {
	// clear screen
	move_cursor(0, 0);
	clear_screen(16);
	move_cursor(0, 0);
	// display points
	cout << "Points: " << points << "\n" << endl;
	// print game instructions based on mode
	cout << "Mode: " << mode << "\n" << endl;
	if (mode == "basic") {
		cout << "Press Space to stop each of the columns from moving, starting with the leftmost column." << endl;
		cout << "The aim is to end up with rows of matching characters once all the columns are stopped." << endl;
		cout << "Points are granted for the longest sequence of matching characters in each row.\n" << endl;
		cout << "Payouts - 10 points for 2 matching characters, 100 for 3, 1,000 for 4 and 10,000 for 5.\n" << endl;
	}
	else if (mode == "poker") {
		cout << "Press Space to deal each of your 5 cards, starting with the leftmost card." << endl;
		cout << "After all 5 cards are dealt, you may select any card to re-deal." << endl;
		cout << "You may do this up to 5 times, or not at all." << endl;
		cout << "The aim is to end up with the highest scoring poker hand possible.\n" << endl;
		cout << "Payouts - Pair: 10 points, Two Pair: 25 points, Three of a Kind: 50 points," << endl;
		cout << "Straight: 100 points, Flush: 150 points, Full House: 200 points," << endl;
		cout << "Four of a Kind: 250 points, Straight Flush: 1,000 points," << endl;
		cout << "Royal Flush: 10,000 points.\n" << endl;
	}
	// set cost of playing
	set_cost();
	// display cost of playing
	cout << "Cost of playing is 20 or 10% of current points, whichever is higher." << endl;
	cout << "Playing currently costs " << cost << " points." << endl;
	// play or quit based on keypress
	cout << "\nPress Enter to play, or Q to quit." << endl;
	waiting = true;
	while (waiting) {
		if (GetKeyState(VK_RETURN) & 0x8000) {
			waiting = false;
			play();
		}
		else if (GetKeyState('Q') & 0x8000) {
			waiting = false;
			quit("You quit the game.");
		}
	}
}

// selects play function based on mode
void play() {
	if (mode == "basic") {
		play_basic();
	}
	else if (mode == "poker") {
		play_poker();
	}
}

// prompts to continue, and quits if insufficient points remain
void score_continue() {
	// wait for C to be pressed
	cout << "\nPress C to continue." << endl;
	waiting = true;
	while (waiting) {
		if (GetKeyState('C') & 0x8000) {
			waiting = false;
		}
	}
	// ensure cost is up to date
	set_cost();
	// return to start screen if sufficient points remain, quit otherwise
	if (points >= cost) {
		start();
	}
	else {
		quit("You have run out of points! Game over.");
	}
}

// prints message, final points and maximum points achieved
void quit(const string &message) {
	// clear screen
	move_cursor(0, 0);
	clear_screen(20);
	move_cursor(0, 0);
	// print message and score details
	cout << message << endl;
	cout << "\nFinal points: " << points << "    Maximum points: " << max_points << "\n" << endl;
}


//// SECTION: BASIC MODE GLOBAL VARIABLES AND FUNCTIONS

// constants for number of unique characters and size of slot machine
const int N_CHARS = 12;
const int N_ROWS = 5;
const int N_COLS = 5;

// array containing each unique character
char characters[N_CHARS] = { 'A', 'B', 'C', 'X', 'Y', 'Z', '$', '%', '@', '#', '!', '~' };
// 2d array for slot machine lines
char lines[N_ROWS][N_COLS] = { 0 };

// uniform random distribution used to generate characters
uniform_int_distribution<mt19937::result_type> unif_chars(0, N_CHARS - 1);

// fills slot machine lines with random characters
void initialize_lines() {
	// loop over row indices
	for(int i = 0; i < N_ROWS; i++) {
		// loop over column indices
		for(int j = 0; j < N_COLS; j++) {
			// set element to random character
			lines[i][j] = characters[unif_chars(rng)];
		}
	}
}

// 'rotates' slot machine lines by shifting them down and adding a new random character at top
void rotate_lines(int num) {
	// loop over rightmost num columns
	for(int col = N_COLS - num; col < N_COLS; col++) {
		// loop backwards over row indices and set element to
		// element from row above
		for(int row = N_ROWS - 1; row > 0; row--) {
			lines[row][col] = lines[row - 1][col];
		}
		// set top element in column to new random character
		lines[0][col] = characters[unif_chars(rng)];
	}
}

// prints slot machine lines to console
void render_lines() {
	// move cursor to top left
	move_cursor(0, 0);
	// display current points
	cout << "Points: " << points << "\n" << endl;
	// loop over rows and cols of 2d array and print elements
	for(int i = 0; i < N_ROWS; i++) {
		for(int j = 0; j < N_COLS; j++) {
			cout << lines[i][j];
			// print space between columns
			if(j < N_COLS - 1) {
				cout << "   ";
			}
		}
		// space out rows
		cout << "\n\n";
	}
}

// calculates and prints score
void score_basic() {
	// current matching items as row is traversed
	int current_matched = 0;
	// highest number of matching items in row
	int max_matched = 0;
	// how many lines contain all matching characters
	int jackpot = 0;
	// score from this round
	int score = 0;

	// loop over row indices
	for (int i = 0; i < N_ROWS; i++) {
		// reset current and max matched to 0
		current_matched = 0;
		max_matched = 0;
		// loop over column indices from 1 onward
		for (int j = 1; j < N_COLS; j++) {
			// if element matches element in previous column
			if (lines[i][j] == lines[i][j - 1]) {
				// increment current matched
				current_matched += 1;
			}
			// if element does not match previous or row traversal is finished
			if (lines[i][j] != lines[i][j - 1] || j == N_COLS - 1) {
				// set max matched to highest of current matched and max matched
				max_matched = max(current_matched, max_matched);
				// reset current matched
				current_matched = 0;
			}
		}
		// if there are any matches, increment score by 10^max_matched
		if (max_matched > 0) {
			score += (int)pow(10, max_matched);
		}
		// if all items match, increment jackpot counter
		if (max_matched == 4) {
			jackpot += 1;
		}
	}
	// add score to points
	points += score;
	// update max points
	if (points > max_points) {
		max_points = points;
	}
	// display score
	render_lines();
	cout << "Score: " << score;
	// display jackpot message if jackpot was hit
	if (jackpot) {
		cout << " - You hit the jackpot! (x" << jackpot << ")";
	}
	cout << endl;
	// prompt to continue
	score_continue();
}

// play function for basic mode
void play_basic() {
	// reset number of moving columns to total number of columns
	moving_cols = N_COLS;
	// subtract points based on cost of playing
	points -= cost;
	// clear screen, initialize and render lines
	move_cursor(0, 0);
	clear_screen(20);
	initialize_lines();
	render_lines();

	// reset key pressed to false
	key_pressed = false;
	// loop until all columns are stopped
	while (moving_cols > 0) {
		// rotate lines based on current number of moving columns
		rotate_lines(moving_cols);
		// if space is pressed and was not previously
		if (GetKeyState(VK_SPACE) & 0x8000 && !key_pressed) {
			key_pressed = true;
			// decrement moving cols (stops leftmost moving column)
			moving_cols--;
		}
		// if space is not pressed and was previously
		else if (!(GetKeyState(VK_SPACE) & 0x8000) && key_pressed) {
			key_pressed = false;
		}
		// display lines and pause for 0.1s
		render_lines();
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	// calculate score
	score_basic();
}


//// SECTION: POKER MODE GLOBAL VARIABLES AND FUNCTIONS

// struct for card
struct card {
	// char for value
	char val;
	// char for suit
	char suit;
	// int for ordinal value (index in vector of values)
	int ord;
};

// arrays for value and suit characters
char vals[13] = { '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A' };
char suits[4] = { 'D', 'C', 'H', 'S' };

// vector for deck and array for hand
vector<card> deck;
card hand[5];

// variables for remaining re-deals and which card to re-deal
int redeals_remaining;
int redeal_card_num;

// generates deck of 52 cards from vectors of suits and values and shuffles
void initialize_deck() {
	deck = {};
	int o;
	for (char s : suits) {
		o = 0;
		for (char v : vals) {
			deck.push_back({ v, s, o });
			o++;
		}
	}
	// shuffle deck
	shuffle(deck.begin(), deck.end(), rng);
}

// renders hand to screen and displays current points
void render_hand() {
	move_cursor(0, 0);
	cout << "Points: " << points << '\n' << endl;
	cout << " ____    ____    ____    ____    ____ " << endl;
	cout << "| ?  |  | ?  |  | ?  |  | ?  |  | ?  |" << endl;
	cout << "|  ? |  |  ? |  |  ? |  |  ? |  |  ? |" << endl;
	cout << "|____|  |____|  |____|  |____|  |____|" << endl;

	cout << " ____    ____    ____    ____    ____ " << endl;
	cout << "| " << hand[0].val << "  |  | " << hand[1].val << 
		"  |  | " << hand[2].val << "  |  | " << hand[3].val << 
		"  |  | " << hand[4].val << "  |" << endl;
	cout << "|  " << hand[0].suit << " |  |  " << hand[1].suit << 
		" |  |  " << hand[2].suit << " |  |  " << hand[3].suit << 
		" |  |  " << hand[4].suit << " |" << endl;
	cout << "|____|  |____|  |____|  |____|  |____|" << endl;

	cout << " ____    ____    ____    ____    ____ " << endl;
	cout << "| ?  |  | ?  |  | ?  |  | ?  |  | ?  |" << endl;
	cout << "|  ? |  |  ? |  |  ? |  |  ? |  |  ? |" << endl;
	cout << "|____|  |____|  |____|  |____|  |____|" << endl;
}

// returns sorted vector of ordinal values of cards in hand
vector<int> get_ords() {
	// create vector for ordinal values
	vector<int> ord_vals = {};
	// extract ordinal values from hand
	for (card c : hand) {
		ord_vals.push_back(c.ord);
	}
	// sort values
	sort(ord_vals.begin(), ord_vals.end());
	// return vector of sorted ordinal values
	return ord_vals;
 }

// checks if hand contains flush
bool check_flush() {
	for (int i = 1; i < 5; i++) {
		if (hand[i].suit != hand[i-1].suit) {
			return false;
		}
	}
	return true;
}

// checks if hand contains straight
bool check_straight() {
	// get sorted ordinal values as vector
	vector<int> ords = get_ords();
	// loop over values from index 1 and return false
	// if value is not equal to previous value + 1
	for (int i = 1; i < 5; i++) {
		if (ords[i] != ords[i - 1] + 1) {
			return false;
		}
	}
	return true;
}

// checks how many pairs hand contains
int check_pairs() {
	// get sorted ordinal values as vector
	vector<int> ords = get_ords();
	int pairs = 0;
	for (int i = 1; i < 5; i++) {
		// increment number of pairs if value is equal to previous and either
		// the index is equal to 1 or the value is not equal to that 2 indices previous
		if (ords[i] == ords[i - 1] && (i == 1 || ords[i] != ords[i - 2])) {
			pairs += 1;
		}
	}
	return pairs;
}

// checks if hand contains 3 or 4 of a kind
int check_n_of_a_kind() {
	// get sorted ordinal values as vector
	vector<int> ords = get_ords();
	int n = 0;
	for (int i = 2; i <= 4; i++) {
		// if value is equal to the two previous values, set n to 3
		if (ords[i] == ords[i - 1] && ords[i] == ords[i - 2]) {
			n = 3;
			// if index is 3 or 4 and value is equal to that 3
			// indices previous, set n to 4
			if (i >= 3 && ords[i] == ords[i - 3]) {
				n = 4;
			}
		}
	}
	return n;
}

// checks if hand contains full house
bool check_full_house() {
	// return true if 2 pairs and 3 of a kind both present
	// (3 of a kind will also be counted as a pair)
	if (check_pairs() == 2 && check_n_of_a_kind() == 3) {
		return true;
	}
	return false;
}

// checks if hand contains straight flush
bool check_straight_flush() {
	if (check_flush() && check_straight()) {
		return true;
	}
	return false;
}

// checks if hand contains royal flush
bool check_royal_flush() {
	// get sorted ordinal values as vector
	vector<int> ords = get_ords();
	// return true if straight flush and ordinal values go from 8 to 12 (10 to A)
	if (check_straight_flush() && ords[0] == 8 && ords[4] == 12) {
		return true;
	}
	return false;
}

// scores hand
void score_poker() {

	string result;
	int score = 0;

	int n_of_a_kind = check_n_of_a_kind();
	int num_pairs = check_pairs();

	// set result string and score based on best hand achieved
	if (check_royal_flush()) {
		result = "Royal Flush";
		score = 10000;
	}
	else if (check_straight_flush()) {
		result = "Straight Flush";
		score = 1000;
	}
	else if (n_of_a_kind == 4) {
		result = "Four of a Kind";
		score = 250;
	}
	else if (check_full_house()) {
		result = "Full House";
		score = 200;
	}
	else if (check_flush()) {
		result = "Flush";
		score =  150;
	}
	else if (check_straight()) {
		result = "Straight";
		score = 100;
	}
	else if (n_of_a_kind == 3) {
		result = "Three of a Kind";
		score = 50;
	}
	else if (num_pairs == 2) {
		result = "Two Pair";
		score = 25;
	}
	else if (num_pairs == 1) {
		result = "Pair";
		score = 10;
	}
	else {
		result = "High Card";
	}
	// add score to points
	points += score;
	// update max points
	if (points > max_points) {
		max_points = points;
	}
	// display hand
	move_cursor(0, 0);
	render_hand();
	cout << endl;
	// clear space below hand
	clear_screen(2);
	move_cursor(0, 15);
	// display score and hand achieved
	cout << "Score: " << score << " (" << result << ")" << endl;
	// prompt to continue
	score_continue();
}

// deals rightmost cards_to_deal cards from top of deck
void deal_cards() {
	for (int i = 5 - cards_to_deal; i < 5; i++) {
		hand[i] = deck[i - (5 - cards_to_deal)];
	}
}

// deals hand of cards one by one
void deal_hand() {
	// while still cards to deal
	while (cards_to_deal > 0) {
		// deal cards based on cards_to_deal
		deal_cards();
		// if space is pressed and wasn't previously
		if (GetKeyState(VK_SPACE) & 0x8000 && !key_pressed)
		{
			key_pressed = true;
			// remove card just dealt from deck
			deck.erase(deck.begin());
			// decrement cards_to_deal
			cards_to_deal--;
		}
		// if space is not pressed and was previously
		else if (!(GetKeyState(VK_SPACE) & 0x8000) && key_pressed) {
			key_pressed = false;
		}
		// shuffle deck
		shuffle(deck.begin(), deck.end(), rng);
		// display hand and pause for 0.1s
		render_hand();
		this_thread::sleep_for(chrono::milliseconds(100));
	}
}

// re-deals a single card based on redeal_card_num
void redeal_card() {
	// index is num - 1
	int redeal_card_index = redeal_card_num - 1;
	// save current card as discard (no chance of being re-dealt same card)
	card discard = hand[redeal_card_index];
	// repeatedly deal new card and stop when space is pressed
	waiting = true;
	while (waiting) {
		// shuffle deck
		shuffle(deck.begin(), deck.end(), rng);
		// deal new card from top of deck
		hand[redeal_card_index] = deck[0];
		// if space is pressed, stop dealing and remove dealt card from deck
		if (GetKeyState(VK_SPACE) & 0x8000) {
			waiting = false;
			deck.erase(deck.begin());
		}
		// display hand and pause for 0.1s
		render_hand();
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	// add discard to bottom of deck
	deck.push_back(discard);
}

// re-deals cards as long as re-deals remain
void redeal_cards() {
	// while some re-deals remain
	while (redeals_remaining > 0) {
		// print instructions and no. of re-deals remaining
		cout << endl;
		cout << "Enter card no. (1-5) to re-deal, and press Space to deal a new card." << endl;
		cout << "You have " << redeals_remaining << " re-deals remaining. Enter 0 to finish." << endl;
		// select card to re-deal based on keypress
		waiting = true;
		while (waiting) {
			for (int i = 0; i <= 5; i++) {
				string key_s = to_string(i);
				char key = key_s[0];
				if (GetKeyState(key) & 0x8000) {
					redeal_card_num = i;
					waiting = false;
				}
			}
		}
		// if 0 is pressed, stop re-dealing
		if (redeal_card_num == 0) {
			redeals_remaining = 0;
		}
		// otherwise, re-deal based on redeal_card_num and decrement remaining re-deals
		else {
			redeal_card();
			redeals_remaining--;
		}
	}
}

// play function for poker mode
void play_poker() {
	// reset cards to deal and remaining redeals
	cards_to_deal = 5;
	redeals_remaining = 5;
	// subtract points based on cost of playing
	points -= cost;
	// initialize deck
	initialize_deck();
	// clear screen
	move_cursor(0, 0);
	clear_screen(20);
	move_cursor(0, 0);
	// deal, re-deal and score
	deal_hand();
	redeal_cards();
	score_poker();
}


//// SECTION: MAIN FUNCTION

int main() {
	// seed rng
	rng.seed(random_device()());
	// go to main menu
	main_menu();

	return 0;
}