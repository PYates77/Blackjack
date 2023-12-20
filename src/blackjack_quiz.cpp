#include <chrono>
#include <iostream>
#include "blackjack.h"
#include <map>

/*
 * TODO: The whole reason I'm writing this is so that the quiz remembers which ones you got wrong and preferentially quizzes you on those
 * implement this
 */

/* < player split possibility, dealer upcard, basic strategy player action > */
/* SPECIAL CASE CHECK REQUIRED FOR (A/A) (soft 12) */
std::map<int, std::map<int,enum BlackjackPlayerActions>> strategy_split = {
	/* 2, 2 */
	{ 4,{{2,SPLIT},	{3,SPLIT},	{4,SPLIT},	{5,SPLIT},	{6,SPLIT},	{7,SPLIT},	{8,HIT},	{9,HIT},	{10,HIT},	{11,HIT}}},
	/* 3, 3 */
	{ 6,{{2,SPLIT},	{3,SPLIT},	{4,SPLIT},	{5,SPLIT},	{6,SPLIT},	{7,SPLIT},	{8,HIT},	{9,HIT},	{10,HIT},	{11,HIT}}},
	/* 4, 4 */
	{ 8,{{2,HIT},	{3,HIT},	{4,HIT},	{5,SPLIT},	{6,SPLIT},	{7,HIT},	{8,HIT},	{9,HIT},	{10,HIT},	{11,HIT}}},
	/* 5, 5 */
	{10,{{2,DOUBLE},	{3,DOUBLE},	{4,DOUBLE},	{5,DOUBLE},	{6,DOUBLE},	{7,DOUBLE},	{8,DOUBLE},	{9,DOUBLE},	{10,HIT},	{11,HIT}}},
	/* 6, 6 */
	{12,{{2,SPLIT},	{3,SPLIT},	{4,SPLIT},	{5,SPLIT},	{6,SPLIT},	{7,HIT},	{8,HIT},	{9,HIT},	{10,HIT},	{11,HIT}}},
	/* 7, 7 */
	{14,{{2,SPLIT},	{3,SPLIT},	{4,SPLIT},	{5,SPLIT},	{6,SPLIT},	{7,SPLIT},	{8,HIT},	{9,HIT},	{10,HIT},	{11,HIT}}},
	/* 8, 8 */
	{16,{{2,SPLIT},	{3,SPLIT},	{4,SPLIT},	{5,SPLIT},	{6,SPLIT},	{7,SPLIT},	{8,SPLIT},	{9,SPLIT},	{10,SPLIT},	{11,SPLIT}}},
	/* 9, 9 */
	{18,{{2,SPLIT},	{3,SPLIT},	{4,SPLIT},	{5,SPLIT},	{6,SPLIT},	{7,STAND},	{8,SPLIT},	{9,SPLIT},	{10,STAND},	{11,STAND}}},
	/* 10, 10 */
	{20,{{2,STAND},	{3,STAND},	{4,STAND},	{5,STAND},	{6,STAND},	{7,STAND},	{8,STAND},	{9,STAND},	{10,STAND},	{11,STAND}}},
};

/* < player soft total, dealer upcard, basic strategy player action > */
/* evaluate this chart only if split doesn't apply */
std::map<int, std::map<int,enum BlackjackPlayerActions>> strategy_soft = {
	/* A, 2 */
	{13,{{2,HIT},	{3,HIT},	{4,HIT},	{5,DOUBLE},	{6,DOUBLE},	{7,HIT},	{8,HIT},	{9,HIT},	{10,HIT},	{11,HIT}}},
	/* A, 3 */
	{14,{{2,HIT},	{3,HIT},	{4,HIT},	{5,DOUBLE},	{6,DOUBLE},	{7,HIT},	{8,HIT},	{9,HIT},	{10,HIT},	{11,HIT}}},
	/* A, 4 */
	{15,{{2,HIT},	{3,HIT},	{4,DOUBLE},	{5,DOUBLE},	{6,DOUBLE},	{7,HIT},	{8,HIT},	{9,HIT},	{10,HIT},	{11,HIT}}},
	/* A, 5 */
	{16,{{2,HIT},	{3,HIT},	{4,DOUBLE},	{5,DOUBLE},	{6,DOUBLE},	{7,HIT},	{8,HIT},	{9,HIT},	{10,HIT},	{11,HIT}}},
	/* A, 6 */
	{17,{{2,HIT},	{3,DOUBLE},	{4,DOUBLE},	{5,DOUBLE},	{6,DOUBLE},	{7,HIT},	{8,HIT},	{9,HIT},	{10,HIT},	{11,HIT}}},
	/* A, 7 */
	{18,{{2,DOUBLE},	{3,DOUBLE},	{4,DOUBLE},	{5,DOUBLE},	{6,DOUBLE},	{7,STAND},	{8,STAND},	{9,HIT},	{10,HIT},	{11,HIT}}},
	/* A, 8 */
	{19,{{2,STAND},	{3,STAND},	{4,STAND},	{5,STAND},	{6,DOUBLE},	{7,STAND},	{8,STAND},	{9,STAND},	{10,STAND},	{11,STAND}}},
	/* A, 9 */
	{20,{{2,STAND},	{3,STAND},	{4,STAND},	{5,STAND},	{6,STAND},	{7,STAND},	{8,STAND},	{9,STAND},	{10,STAND},	{11,STAND}}},
	/* A, 10 (BLACKJACK) */
	{21,{{2,STAND},	{3,STAND},	{4,STAND},	{5,STAND},	{6,STAND},	{7,STAND},	{8,STAND},	{9,STAND},	{10,STAND},	{11,STAND}}},
};

/* < player hard total, dealer upcard, basic strategy player action > */
/* evaluate this chart only if split doesn't apply */
std::map<int, std::map<int,enum BlackjackPlayerActions>> strategy_hard = {
	{ 3,{{2,HIT},	{3,HIT},	{4,HIT},	{5,HIT},	{6,HIT},	{7,HIT},	{8,HIT},	{9,HIT},	{10,HIT},	{11,HIT}}},
	{ 4,{{2,HIT},	{3,HIT},	{4,HIT},	{5,HIT},	{6,HIT},	{7,HIT},	{8,HIT},	{9,HIT},	{10,HIT},	{11,HIT}}},
	{ 5,{{2,HIT},	{3,HIT},	{4,HIT},	{5,HIT},	{6,HIT},	{7,HIT},	{8,HIT},	{9,HIT},	{10,HIT},	{11,HIT}}},
	{ 6,{{2,HIT},	{3,HIT},	{4,HIT},	{5,HIT},	{6,HIT},	{7,HIT},	{8,HIT},	{9,HIT},	{10,HIT},	{11,HIT}}},
	{ 7,{{2,HIT},	{3,HIT},	{4,HIT},	{5,HIT},	{6,HIT},	{7,HIT},	{8,HIT},	{9,HIT},	{10,HIT},	{11,HIT}}},
	{ 8,{{2,HIT},	{3,HIT},	{4,HIT},	{5,HIT},	{6,HIT},	{7,HIT},	{8,HIT},	{9,HIT},	{10,HIT},	{11,HIT}}},
	{ 9,{{2,HIT},	{3,DOUBLE},	{4,DOUBLE},	{5,DOUBLE},	{6,DOUBLE},	{7,HIT},	{8,HIT},	{9,HIT},	{10,HIT},	{11,HIT}}},
	{10,{{2,DOUBLE},	{3,DOUBLE},	{4,DOUBLE},	{5,DOUBLE},	{6,DOUBLE},	{7,DOUBLE},	{8,DOUBLE},	{9,DOUBLE},	{10,HIT},	{11,HIT}}},
	{11,{{2,DOUBLE},	{3,DOUBLE},	{4,DOUBLE},	{5,DOUBLE},	{6,DOUBLE},	{7,DOUBLE},	{8,DOUBLE},	{9,DOUBLE},	{10,DOUBLE},{11,DOUBLE}}},
	{12,{{2,HIT},	{3,HIT},	{4,STAND},	{5,STAND},	{6,STAND},	{7,HIT},	{8,HIT},	{9,HIT},	{10,HIT},	{11,HIT}}},
	{13,{{2,STAND},	{3,STAND},	{4,STAND},	{5,STAND},	{6,STAND},	{7,HIT},	{8,HIT},	{9,HIT},	{10,HIT},	{11,HIT}}},
	{14,{{2,STAND},	{3,STAND},	{4,STAND},	{5,STAND},	{6,STAND},	{7,HIT},	{8,HIT},	{9,HIT},	{10,HIT},	{11,HIT}}},
	{15,{{2,STAND},	{3,STAND},	{4,STAND},	{5,STAND},	{6,STAND},	{7,HIT},	{8,HIT},	{9,SURRENDER},{10,HIT},	{11,HIT}}},
	{16,{{2,STAND},	{3,STAND},	{4,STAND},	{5,STAND},	{6,STAND},	{7,HIT},	{8,HIT},	{9,SURRENDER},{10,SURRENDER},{11,SURRENDER}}},
	{17,{{2,STAND},	{3,STAND},	{4,STAND},	{5,STAND},	{6,STAND},	{7,STAND},	{8,STAND},	{9,STAND},	{10,STAND},	{11,STAND}}},
	{18,{{2,STAND},	{3,STAND},	{4,STAND},	{5,STAND},	{6,STAND},	{7,STAND},	{8,STAND},	{9,STAND},	{10,STAND},	{11,STAND}}},
	{19,{{2,STAND},	{3,STAND},	{4,STAND},	{5,STAND},	{6,STAND},	{7,STAND},	{8,STAND},	{9,STAND},	{10,STAND},	{11,STAND}}},
	{20,{{2,STAND},	{3,STAND},	{4,STAND},	{5,STAND},	{6,STAND},	{7,STAND},	{8,STAND},	{9,STAND},	{10,STAND},	{11,STAND}}},
	{21,{{2,STAND},	{3,STAND},	{4,STAND},	{5,STAND},	{6,STAND},	{7,STAND},	{8,STAND},	{9,STAND},	{10,STAND},	{11,STAND}}},
};


#define STARTING_WEIGHT 100
#define WEIGHT_CHANGE_PCT 25 /* change weights by percent, by fixed value would cause weight to go negative eventually */

int scenario_weights[_NUM_BLACKJACK_CARDS_][_NUM_BLACKJACK_CARDS_][_NUM_BLACKJACK_CARDS_];
int cur_scenario_i;
int cur_scenario_j;
int cur_scenario_k;

void setup_weights()
{
    for (int i=0; i<_NUM_BLACKJACK_CARDS_; i++) {
        for (int j=0; j<_NUM_BLACKJACK_CARDS_; j++) {
            for (int k=0; k<_NUM_BLACKJACK_CARDS_; k++) {
                scenario_weights[i][j][k] = STARTING_WEIGHT;
            }
        }
    }
}

/*
 * Choose the card combination for the player randomly,
 * but more heavily weight scenarios where the player has made mistakes
 */
void choose_cards(BlackjackCard &c1, BlackjackCard &c2, BlackjackCard &c3)
{
    /* COMPLETELY RANDOM METHOD */
    /*
    c1 = BlackjackCard(
            static_cast<enum BlackjackSuit>(rand() % _NUM_BLACKJACK_SUITS_),
            static_cast<enum BlackjackCardValue>(rand() % _NUM_BLACKJACK_CARDS_));
    c2 = BlackjackCard(
            static_cast<enum BlackjackSuit>(rand() % _NUM_BLACKJACK_SUITS_),
            static_cast<enum BlackjackCardValue>(rand() % _NUM_BLACKJACK_CARDS_));
    c3 = BlackjackCard(
            static_cast<enum BlackjackSuit>(rand() % _NUM_BLACKJACK_SUITS_),
            static_cast<enum BlackjackCardValue>(rand() % _NUM_BLACKJACK_CARDS_));
    */
   
    int weight_sum = 0;
    for (int i=0; i<_NUM_BLACKJACK_CARDS_; i++) {
        for (int j=0; j<_NUM_BLACKJACK_CARDS_; j++) {
            for (int k=0; k<_NUM_BLACKJACK_CARDS_; k++) {
                weight_sum+=scenario_weights[i][j][k];
            }
        }
    }


    bool found = false;
    int rnd = rand() % weight_sum;
    for (int i=0; i<_NUM_BLACKJACK_CARDS_; i++) {
        for (int j=0; j<_NUM_BLACKJACK_CARDS_; j++) {
            for (int k=0; k<_NUM_BLACKJACK_CARDS_; k++) {
                if (rnd < scenario_weights[i][j][k]) {
                    /* TODO: these globals janky? */
                    cur_scenario_i = i;
                    cur_scenario_j = j;
                    cur_scenario_k = k;
                    c1 = BlackjackCard(
                            static_cast<enum BlackjackSuit>(rand() % _NUM_BLACKJACK_SUITS_),
                            static_cast<enum BlackjackCardValue>(i));
                    c2 = BlackjackCard(
                            static_cast<enum BlackjackSuit>(rand() % _NUM_BLACKJACK_SUITS_),
                            static_cast<enum BlackjackCardValue>(j));
                    c3 = BlackjackCard(
                            static_cast<enum BlackjackSuit>(rand() % _NUM_BLACKJACK_SUITS_),
                            static_cast<enum BlackjackCardValue>(k));

                    return;
                }
                rnd -= scenario_weights[i][j][k];
            }
        }
    }


}


/* we just need a player object so that we can use get_player_action, this is janky and should be refactored */
BlackjackPlayer player;
BlackjackHand hand;
BlackjackCard card1, card2, card3;

int main()
{
    srand(time(0));
    setup_weights();

	while (1) {
        choose_cards(card1, card2, card3);

        hand.deal(card1, card2, 1);

        std::cout << "Dealer showing " << card3 << std::endl;

        enum BlackjackPlayerActions action = player.get_player_action(hand);
        enum BlackjackPlayerActions expected_action;

        bool correct = false;
        /* TODO: this card_scores[] thing is total bs */
        if (hand.can_split) {
            /* since AA split isn't in the table, check for it here */
            if (hand.info.soft && hand.info.value == 12) {
                expected_action = SPLIT;
            } else {
                expected_action = strategy_split[hand.info.value][card_scores[card3.value]];
            }
        } else if (hand.info.soft) {
            expected_action = strategy_soft[hand.info.value][card_scores[card3.value]];
        } else {
            expected_action = strategy_hard[hand.info.value][card_scores[card3.value]];
        }

        //std::cout << "Hand value = " << hand.info.value << std::endl;
        //std::cout << "Dealer upcard value = " <<  card_scores[card3.value] << std::endl;

        int *weight = &scenario_weights[cur_scenario_i][cur_scenario_j][cur_scenario_k];
        //std::cout << "Debug: cur scenario weight = " << *weight << std::endl;

        if (action == expected_action) {
            std::cout << "\033[32mCORRECT!\033[0m\n" << std::endl;
            *weight = ((100 * (*weight)) - ((*weight) * WEIGHT_CHANGE_PCT))/100;

        } else {
            std::cout << "\033[31mINCORRECT!\033[0m Expected action was " << player_action_strings[expected_action]
                << " you chose " << player_action_strings[action] << std::endl << std::endl;
            *weight = ((100 * (*weight)) + ((*weight) * WEIGHT_CHANGE_PCT))/100;
        }

        //std::cout << "Debug: new scenario weight = " << *weight << std::endl;

        hand.clear();
	}

    return 0;
}
