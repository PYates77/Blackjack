#ifndef BLACKJACK_H
#define BLACKJACK_H

#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <chrono>
#include <thread>

enum BlackjackCardValue
{
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN,
    JACK,
    QUEEN,
    KING,
    ACE,
    _NUM_BLACKJACK_CARDS_
};

static std::string card_strings[] = {
    "Two",
    "Three",
    "Four",
    "Five",
    "Six",
    "Seven",
    "Eight",
    "Nine",
    "Ten",
    "Jack",
    "Queen",
    "King",
    "Ace"
};

static int card_scores[] = {
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    10,
    10,
    10,
    11
};

enum BlackjackSuit
{
    CLUBS,
    HEARTS,
    SPADES,
    DIAMONDS,
    _NUM_BLACKJACK_SUITS_
};

static std::string card_suits[] = {
    "♣",
    "♥",
    "♠",
    "♦"
};

enum BlackjackPlayerActions
{
    NO_ACTION,
    STAND,
    HIT,
    DOUBLE,
    SPLIT,
    SURRENDER
};

static std::string player_action_strings[] = {
    "NO ACTION",
    "STAND",
    "HIT",
    "DOUBLE",
    "SPLIT",
    "SURRENDER",
};

struct BlackjackRules
{
    int num_decks;
    bool dealer_hit_soft_17;
    bool double_after_split;
    int min_double_value; //usually 0, 9, or 10, the smallest valued hand the player may double down on
    int max_split_hands;
    bool resplit_aces;
    bool hit_split_aces;
    bool dealer_checks_blackjack; //whether or not the player only loses their initial bet against a dealer blackjack
    bool insurance;
    bool surrender;
    float blackjack_payout; // 3 to 2 would be 1.5, 6 to 5 would be 1.2
    bool continuous_shuffle;
    int shuffle_threshold; // number of cards left in the deck which trigger a shuffle
};

class BlackjackCard
{
    public:
        enum BlackjackSuit suit;
        enum BlackjackCardValue value;

        BlackjackCard();
        BlackjackCard(enum BlackjackSuit s, enum BlackjackCardValue v);
        bool operator<(const BlackjackCard& r);
        bool operator==(enum BlackjackCardValue v);
        friend std::ostream& operator<<(std::ostream& str, BlackjackCard &h);
};

struct BlackjackHandInfo
{
    unsigned int value;
    bool pair;
    bool soft;
};

class BlackjackHand
{
    public:
        unsigned int bet;
        bool blackjack;
        bool can_split;
        bool can_double;
        bool is_soft;
        bool is_busted;
        std::vector<BlackjackCard> cards;
        struct BlackjackHandInfo info;

        void deal(BlackjackCard &card1, BlackjackCard &card2, unsigned int initial_bet);
        void hit(BlackjackCard &card);
        void double_down(BlackjackCard &card);
        void clear();
        friend std::ostream& operator<<(std::ostream& str, BlackjackHand &h);

    private:
        void add_card(BlackjackCard &card);
        void update_info();

};

class BlackjackDeck
{
    int num_decks;
    std::vector<BlackjackCard> deck;

    public:
    BlackjackDeck();
    BlackjackDeck(int num_decks);
    void shuffle();
    int cards_remaining();
    BlackjackCard& deal();
};

class BlackjackPlayer
{
    public:
        int bankroll;
        unsigned int initial_bet;
        std::vector<BlackjackHand> hands;
        const struct BlackjackRules * ruleset;
        BlackjackPlayer();
        void payout(BlackjackHand &h);
        void payout_blackjack(BlackjackHand &h);
        void push(BlackjackHand &h);
        virtual enum BlackjackPlayerActions get_player_action(BlackjackHand &hand);
};

class BlackjackDealer : public BlackjackPlayer
{
    public:
        BlackjackDealer();
        enum BlackjackPlayerActions get_player_action();
        BlackjackCard showing();
};

class BlackjackGame
{
    private:
        std::vector<BlackjackPlayer> players;
        BlackjackDealer dealer;
        BlackjackDeck deck;
        const struct BlackjackRules *ruleset;

        void ask_insurance();
        void payout_insurance();
        void deal_hand();
        void process_player_move(BlackjackPlayer &p);
        void process_dealer_move();
        void payout_player(BlackjackPlayer &p);
        bool all_hands_busted(void);

    public:
        BlackjackGame();
        BlackjackGame(int num_players);
        void play_round();
};

#endif /* BLACKJACK_H */
