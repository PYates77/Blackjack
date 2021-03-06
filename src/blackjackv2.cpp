#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <chrono>
#include <thread>

/* TODO:
 *  Overall you should probably redo this whole thing with better design practices, it's a huge mess
 *  Insurance
 *  Blackjack payout
 */

static bool dramatic_delay_enabled = true;

enum BlackjackCardValue 
{
    TWO, THREE,
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

enum BlackjackPlayerActions
{
    NO_ACTION,
    STAND,
    HIT,
    DOUBLE,
    SPLIT,
    SURRENDER
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
    bool surrender;
    float blackjack_payout; // 3 to 2 would be 1.5, 6 to 5 would be 1.2
    bool continuous_shuffle;
    int shuffle_threshold; // number of cards left in the deck which trigger a shuffle
};

static const struct BlackjackRules default_ruleset = {
    .num_decks = 8,
    .dealer_hit_soft_17 = false,
    .double_after_split = true,
    .min_double_value = 0,
    .max_split_hands = 4,
    .resplit_aces = false,
    .hit_split_aces = false,
    .dealer_checks_blackjack = true,
    .surrender = false,
    .blackjack_payout = 1.5,
    .continuous_shuffle = false,
    .shuffle_threshold = 10,
};


struct BlackjackHandInfo
{
    unsigned int value;
    bool pair;
    bool soft;
};

void dramatic_delay()
{
    if(dramatic_delay_enabled){
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}



class BlackjackCard
{
    public:
        enum BlackjackSuit suit;
        enum BlackjackCardValue value;
        BlackjackCard(enum BlackjackSuit s, enum BlackjackCardValue v) {
            suit = s;
            value = v;
        }

        bool operator<(const BlackjackCard& r) {
            return this->value < r.value;
        }


};

class BlackjackDeck
{
    int num_decks;
    std::vector<BlackjackCard> deck;

    public:
    /* reinits the deck with all cards and shuffles the cards */
    void shuffle(){
        std::cout << "shuffling..." << std::endl;
        deck.clear();
        for (int i=0; i<num_decks; ++i) {
            for(int s=CLUBS; s<_NUM_BLACKJACK_SUITS_; ++s) {
                for(int v=TWO; v<_NUM_BLACKJACK_CARDS_; ++v) {
                    deck.push_back(BlackjackCard(static_cast<BlackjackSuit>(s), static_cast<BlackjackCardValue>(v))); 
                }
            }
        }

        std::random_shuffle(deck.begin(), deck.end());
    }

    BlackjackDeck() {
        num_decks = 1;
        shuffle();
    }

    BlackjackDeck(int num_decks) {
        this->num_decks = num_decks;
        shuffle();
    }

    BlackjackCard& deal(){
        // pop card off of the deck
        BlackjackCard &c = deck.back();
        deck.pop_back();
        return c;
    }

    int cards_remaining() {
        return deck.size();
    }

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

    public:
        void deal(BlackjackCard &card1, BlackjackCard &card2, unsigned int initial_bet)
        {
            bet = initial_bet;
            cards.push_back(card1);
            cards.push_back(card2);
            can_double = true;
            if (this->info().pair) {
                can_split = true;
            }

            if (this->info().value == 21) {
                blackjack = true;
            }
        }	

        void hit(BlackjackCard &card)
        {
            can_split = false;
            blackjack = false;
            cards.push_back(card);
            if (this->info().value > 21) {
                is_busted = true;
            }
        }

        void double_down(BlackjackCard &card)
        {
            cards.push_back(card);
            bet *= 2;
        }

        void clear()
        {
            cards.clear();
            bet = 0;
            can_split = false;
            can_double = false;
            is_soft = false;
            is_busted = false;
            blackjack = false;
        }

        struct BlackjackHandInfo info()
        {
            std::vector<BlackjackCard> sorted_hand = cards;
            std::sort(sorted_hand.begin(), sorted_hand.end()); //need to get aces at the end for correct soft/hard calcualtion
            struct BlackjackHandInfo info;
            info.value = 0;
            info.soft = false;
            info.pair = false;
            if(sorted_hand.size() == 2){
                if(card_scores[sorted_hand[0].value] == card_scores[sorted_hand[1].value]){
                    //std::cout << "DEBUG: that's a pair" << std::endl;
                    info.pair = true;
                    can_split = true;
                } 
            }

            for(BlackjackCard card : sorted_hand){
                switch(card.value){
                    case ACE:
                        if(info.value + 11 > 21){
                            info.value+= 1;
                        } else {
                            info.value+= 11;
                            info.soft = true;
                        }
                        break;
                    default: 
                        info.value += card_scores[card.value];
                }
            }
            return info;
        }
};

std::ostream& operator<<(std::ostream& str, BlackjackHand& v) {
    struct BlackjackHandInfo info = v.info();
    str << "[" << v.cards.size() << "] ";
    if(info.soft){
        for(BlackjackCard card : v.cards){
            str << card_strings[card.value] << card_suits[card.suit] << " ";
        }
        str << " (soft " << info.value << ")";
    } else {
        for(BlackjackCard card : v.cards){
            str << card_strings[card.value] << card_suits[card.suit] << " ";
        }
        str << " (" << info.value << ")";
    }
    return str;
}

class BlackjackPlayer
{
    public:
        int bankroll;
        unsigned int initial_bet;
        std::vector<BlackjackHand> hands;

        BlackjackPlayer()
        {
            bankroll = 1000;
            initial_bet = 10;
            hands.clear();
        }

        void payout(BlackjackHand &h) 
        {
            bankroll += 2*h.bet;
            h.clear();
        }

        void push(BlackjackHand &h)
        {
            bankroll += h.bet;
            h.clear();
        }

        virtual enum BlackjackPlayerActions get_player_action(BlackjackHand &hand)
        {
            enum BlackjackPlayerActions action = NO_ACTION;
            std::cout << "Hand is: " << hand << std::endl << "Choose an Action ([H]it, ";
            if (hand.can_double) {
                std::cout << "[D]ouble, ";
            }
            if (hand.can_split) {
                std::cout << "S[p]lit, "; 
            }
            std::cout << "[S]tand)" << std::endl; 
            std::string player_action;
            while(action == NO_ACTION){
                std::cin >> player_action;
                switch(player_action[0]){
                    case('H'):
                    case('h'):
                        action = HIT;
                        break;
                    case('S'):
                    case('s'):
                        action = STAND;
                        break;
                    case('P'):
                    case('p'):
                        if (hand.can_split) {
                            action = SPLIT;
                        } else {
                            std::cout << "Can't split now" << std::endl;
                        }
                        break;
                    case('D'):
                    case('d'):
                        if (hand.can_double) {
                            action = DOUBLE;
                        } else {
                            std::cout << "Can't double down now" << std::endl;
                        }
                        break;
                    default:
                        std::cout << "Invalid Player Action" << std::endl;
                }
            }
            return action;
        }
}; 

class BlackjackDealer : public BlackjackPlayer 
{
    private:
        const struct BlackjackRules * ruleset;
    public:
        BlackjackDealer(){
            ruleset = &default_ruleset;
        }
        enum BlackjackPlayerActions get_player_action()
        {
            // dealer can only have one hand, so just use hands[0]
            enum BlackjackPlayerActions action = HIT;
            struct BlackjackHandInfo info = hands[0].info();
            if(info.value > 17){
                action = STAND;
            } else if(info.value == 17) {
                if(info.soft && ruleset->dealer_hit_soft_17){
                    action = HIT;
                } else {
                    action = STAND;
                }
            } 
            return action;
        }
        BlackjackCard showing()
        {
            return hands[0].cards[0];
        }
};

class BlackjackGame
{
    std::vector<BlackjackPlayer> players;
    BlackjackDealer dealer;
    BlackjackDeck deck;
    const struct BlackjackRules *ruleset;

    public:
    BlackjackGame()
    {
        ruleset = &default_ruleset;
        players.push_back(BlackjackPlayer());
    }

    BlackjackGame(int num_players) 
    {
        ruleset = &default_ruleset;
        for(int i=0; i<num_players; ++i) {
            players.push_back(BlackjackPlayer());
        }
    }

    void deal_hand()
    {
        if (ruleset->continuous_shuffle) {
            deck.shuffle();
        } else if (deck.cards_remaining() <= ruleset->shuffle_threshold) {
            deck.shuffle();
        }

        for (int i=0; i < players.size(); ++i) {
            players[i].hands.clear();

            if (players[i].bankroll - players[i].initial_bet > 0) {
                players[i].hands.push_back(BlackjackHand());
                players[i].bankroll -= players[i].initial_bet; 
                players[i].hands[0].deal(deck.deal(), deck.deal(), players[i].initial_bet);
            } else {
                std::cout << "Player " << i+1 << " is bankrupt" << std::endl;
            }
        }

        dealer.hands.clear();
        dealer.hands.push_back(BlackjackHand());
        dealer.hands[0].deal(deck.deal(), deck.deal(), 0);

        std::cout << "Dealer showing " << card_strings[dealer.showing().value] << card_suits[dealer.showing().suit] << std::endl << std::endl;

        for (int i=0; i < players.size(); ++i) {
            BlackjackPlayer *p = &players[i];
            // we print bankroll + initial bet because the player hasn't actually lost their initial bet yet
            // but internally the initial bet currently belongs to the dealt hand 
            std::cout << "Player " << i+1  << ": $" << p->bankroll + p->initial_bet << std::endl;
            bool player_done = false;
            // easier to use while here than for loop because # of hands may change
            int j=0;
            while(!player_done) {
                if (p->hands.size() > 1) {
                    std::cout << "Hand #" << j+1 << std::endl;
                }
                BlackjackHand &hand = p->hands[j];
                enum BlackjackPlayerActions action = NO_ACTION;
                if (hand.blackjack) {
                    std::cout << "Blackjack!!!" << std::endl;
                    action = STAND;
                }
                while (action != STAND && action != DOUBLE && action != SPLIT && !hand.is_busted) {
                    action = p->get_player_action(hand);
                    switch (action) {
                        case HIT:
                            hand.hit(deck.deal());
                            std::cout << "Player Hits: " << hand << std::endl;
                            if (hand.is_busted) {
                                std::cout << "Busted" << std::endl;
                            }
                            break;
                        case STAND:
                            break;
                        case DOUBLE:
                            p->bankroll -= p->initial_bet;
                            hand.double_down(deck.deal());
                            std::cout << "Player Doubles Down: " << hand << std::endl;
                            break;
                        case SPLIT:
                            {
                                p->bankroll -= p->initial_bet;
                                BlackjackHand new1 = BlackjackHand();
                                BlackjackHand new2 = BlackjackHand();
                                new1.deal(hand.cards[0], deck.deal(), p->initial_bet);
                                new2.deal(hand.cards[1], deck.deal(), p->initial_bet);
                                p->hands[j] = new1;
                                p->hands.insert(p->hands.begin()+j+1, new2);
                                j = j-1; // need to reprocess this hand 
                            }

                            std::cout << "Player Splits and now has " << p->hands.size() << " hands" << std::endl;
                            break;
                        default: 
                            std::cout << "Unreachable state: The programmer is a moron" << std::endl;
                    }
                }

                if (j+1 < p->hands.size()) {
                    ++j;
                } else {
                    player_done = true;
                }
            }
        }

        std::cout << "Dealer's hand: " << dealer.hands[0] << std::endl;
        dramatic_delay();

        enum BlackjackPlayerActions action = NO_ACTION;
        while(action != STAND){
            action = dealer.get_player_action();
            if(action == HIT){
                dealer.hands[0].hit(deck.deal());
                std::cout << "Dealer Hits: " << dealer.hands[0] << std::endl;
            } else if (action == NO_ACTION){
                std::cout << "ERROR: Dealer Had a Stroke\n" << std::endl;
                break;
            }
            if(dealer.hands[0].is_busted){
                std::cout << "Dealer Busted\n" << std::endl;
                break;
            }
            dramatic_delay();
        }

        for(int i=0; i<players.size(); ++i){
            std::cout << "Player " << i+1 << " ";
            for (int j=0; j<players[i].hands.size(); ++j) {
                if (players[i].hands.size() > 1) {
                    std::cout << std::endl << "Hand " << j+1 << " ";
                }

                //std::cout << "(" << players[i].hands[j].info().value << ") ";
                std::cout << "(" << players[i].hands[j].info().value << ") ";
                if(players[i].hands[j].is_busted){
                    std::cout << "Loses\n" << std::endl;
                } else if (dealer.hands[0].is_busted){
                    std::cout << "Wins!\n" << std::endl;
                    players[i].payout(players[i].hands[j]);
                } else if(players[i].hands[j].info().value > dealer.hands[0].info().value){
                    std::cout << "Wins!\n" << std::endl;
                    players[i].payout(players[i].hands[j]);
                } else if (players[i].hands[0].info().value == dealer.hands[0].info().value){
                    std::cout << "Pushes\n" << std::endl;
                    players[i].push(players[i].hands[j]);
                } else {
                    std::cout << "Loses\n" << std::endl;
                }
                dramatic_delay();
            }
        }

    }
};

int main()
{
    srand(time(0));
    int i;
    BlackjackGame game(1);
    for(i=0; i<10; ++i){
        game.deal_hand();
    }

    return 0;
}


