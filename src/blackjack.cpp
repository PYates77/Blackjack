#include <map>
#include <set>
#include <vector>
#include <algorithm>

enum BlackjackCards 
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
    ACE
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
    .continuous_shuffle = false
};

struct BlackjackHandInfo
{
    unsigned int value;
    bool pair;
    bool soft;
};

class BlackjackHand
{
private:
    std::vector<enum BlackjackCards> cards;
    unsigned int bet;

public:
    void place_bet(unsigned int new_bet)
    {
        bet = new_bet;
    }
    void add_card(enum BlackjackCards card)
    {
        cards.push_back(card);
    }
    struct BlackjackHandInfo info()
    {
        std::sort(cards.begin(), cards.end()); //need to get aces at the end for correct soft/hard calcualtion
        struct BlackjackHandInfo info;
        info.value = 0;
        info.soft = false;
        info.pair = false;
        if(cards.size() == 2){
            if(cards[0] == cards[1]){
                info.pair = true;
            }
        }
        for(enum BlackjackCards card : cards){
            switch(card){
                case ACE:
                    if(info.value + 11 > 21){
                        info.soft = true;
                        info.value+= 1;
                    } else {
                        info.value+= 11;
                    }
                    break;
                case TWO:
                    info.value += 2;
                    break;
                case THREE:
                    info.value += 3;
                    break;
                case FOUR:
                    info.value += 4;
                    break;
                case FIVE:
                    info.value += 5;
                    break;
                case SIX:
                    info.value += 6;
                    break;
                case SEVEN:
                    info.value += 7;
                    break;
                case EIGHT:
                    info.value += 8;
                    break;
                case NINE:
                    info.value += 9;
                    break;
                case TEN: //fallthrough
                case JACK:
                case QUEEN:
                case KING:
                    info.value += 10;
                    break;
            }
        }
        return info;
    }
};

class BlackjackPlayer 
{
private:
    unsigned int bankroll;
    unsigned int starting_bet;
    BlackjackHand hand; 

public:
    BlackjackPlayer()
    {
        bankroll = 1000;
        starting_bet = 10;
    }
    BlackjackPlayer(unsigned int initial_bankroll, unsigned int initial_bet)
    {
        bankroll = initial_bankroll;
        starting_bet = initial_bet;
    }
    void deal(BlackjackCards card1, BlackjackCards card2)
    {
        hand.add_card(card1);
        hand.add_card(card2);
        hand.place_bet(starting_bet);
    }
    void set_bet(unsigned int bet_amount)
    {
        starting_bet = bet_amount;
    }
    void hit(enum BlackjackCards card)
    {
        hand.add_card(card);
    }
    void double_down(enum BlackjackCards card)
    {
        hand.add_card(card);
        hand.place_bet(2*starting_bet);
    }
    //TODO split

    bool busted()
    {
        return (hand.info().value > 21);
    }

};

class BlackjackGame 
{
private:
    std::vector<BlackjackPlayer> players;
    struct BlackjackRules ruleset;
public:
    BlackjackGame()
    {
        //initialize one player
        players.push_back(BlackjackPlayer());
    }
    BlackjackGame(int num_players)
    {
        for(int i=0; i<num_players; ++i){
            players.push_back(BlackjackPlayer());
        }
    }
};
