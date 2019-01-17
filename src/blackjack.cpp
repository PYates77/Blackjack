#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>


// TODO LIST
// Game does not check for player or dealer blackjack
// Game does not print the value of the player's hand after they double or blackjack
// Player cannot split

static bool dramatic_delay_enabled = true;

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

enum BlackjackPlayerActions
{
    NO_ACTION,
    STAND,
    HIT,
    DOUBLE,
    SPLIT,
    SURRENDER
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
    unsigned int bet;

public:
    std::vector<enum BlackjackCards> cards;
    void place_bet(unsigned int new_bet)
    {
        bet = new_bet;
    }
    unsigned int get_bet()
    {
        return bet;
    }
    void add_card(enum BlackjackCards card)
    {
        cards.push_back(card);
    }
    void clear()
    {
        cards.clear();
    }
    struct BlackjackHandInfo info()
    {
        std::vector<enum BlackjackCards> sorted_hand = cards;
        std::sort(sorted_hand.begin(), sorted_hand.end()); //need to get aces at the end for correct soft/hard calcualtion
        struct BlackjackHandInfo info;
        info.value = 0;
        info.soft = false;
        info.pair = false;
        if(sorted_hand.size() == 2){
            if(sorted_hand[0] == sorted_hand[1]){
                info.pair = true;
            }
        }
        for(enum BlackjackCards card : sorted_hand){
            switch(card){
                case ACE:
                    if(info.value + 11 > 21){
                        info.value+= 1;
                    } else {
                        info.value+= 11;
                        info.soft = true;
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
std::ostream& operator<<(std::ostream& str, BlackjackHand& v) {
    struct BlackjackHandInfo info = v.info();
    if(info.pair){
        str << "pair of " << card_strings[v.cards[0]] << "s (" << info.value << ")";
    } else {
        if(info.soft){
            for(enum BlackjackCards card : v.cards){
                  str << card_strings[card] << " ";
            }
            str << " (soft " << info.value << ")";
        } else {
            for(enum BlackjackCards card : v.cards){
                  str << card_strings[card] << " ";
            }
            str << " (" << info.value << ")";
        }
    }
    return str;
}

class BlackjackPlayer 
{
protected:
    int bankroll;
    unsigned int starting_bet;
    BlackjackHand hand; 

public:
    BlackjackPlayer()
    {
        bankroll = 1000;
        starting_bet = 10;
        hand.clear();
    }
    BlackjackPlayer(unsigned int initial_bankroll, unsigned int initial_bet)
    {
        bankroll = initial_bankroll;
        starting_bet = initial_bet;
    }
    void deal(BlackjackCards card1, BlackjackCards card2)
    {
        //std::cout << "[DEBUG] dealing " << card_strings[card1] << " " << card_strings[card2] << std::endl;
        hand.add_card(card1);
        hand.add_card(card2);
        bankroll -= starting_bet;
        hand.place_bet(starting_bet);
        //std::cout << "[DEBUG] hand: " << hand << std::endl; 
    }
    void set_bet(unsigned int bet_amount)
    {
        starting_bet = bet_amount;
    }
    int  get_bankroll()
    {
        return bankroll;
    }

    void hit(enum BlackjackCards card)
    {
        hand.add_card(card);
    }
    void double_down(enum BlackjackCards card)
    {
        hand.add_card(card);
        hand.place_bet(2*starting_bet);
        bankroll -= starting_bet;
    }
    //TODO split

    void pay()
    {
        bankroll += 2*hand.get_bet();
    }
    void push()
    {
        bankroll += hand.get_bet();
    }
    bool busted()
    {
        return (hand.info().value > 21);
    }
    void clear()
    {
        hand.clear();
    }

    virtual enum BlackjackPlayerActions get_player_action()
    {
        enum BlackjackPlayerActions action = NO_ACTION;
        std::cout << "Hand is: " << hand << std::endl << "Choose an Action ([H]it, [S]tand, [D]ouble)" << std::endl; 
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
                case('D'):
                case('d'):
                    action = DOUBLE;
                    break;
                default:
                    std::cout << "Invalid Player Action" << std::endl;
            }
        }
        return action;
    }

    BlackjackHand get_hand()
    {
        return hand;
    }
    
    unsigned int get_hand_value()
    {
        BlackjackHandInfo info = hand.info();
        return info.value;
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
        enum BlackjackPlayerActions action = HIT;
        struct BlackjackHandInfo info = hand.info();
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
    enum BlackjackCards showing()
    {
        return hand.cards[0];
    }
};

void dramatic_delay()
{
    if(dramatic_delay_enabled){
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}


class BlackjackGame 
{
private:
    std::vector<BlackjackPlayer> players;
    BlackjackDealer dealer;
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
    enum BlackjackCards next_card()
    {
        //if(ruleset.continuous_shuffle){ //TODO continuous shuffle doesn't imply complete randomness
            return static_cast<enum BlackjackCards>(rand()%_NUM_BLACKJACK_CARDS_);
        //}
    }

    void deal_hand()
    {
        //TODO: does it matter in any statistical way what order you deal the cards out in? even if you simulate a real deck? I think it doesn't
        int i;
        for(i=0; i<players.size(); ++i){
            std::cout << "Player " << i+1 << " bankroll is " << players[i].get_bankroll() << std::endl;
            players[i].deal(next_card(), next_card()); 
        }
        dealer.deal(next_card(), next_card());

        std::cout << "Dealer showing " << card_strings[dealer.showing()] << std::endl;

        dramatic_delay(); 
        for(i=0; i<players.size(); ++i){
            enum BlackjackPlayerActions action = NO_ACTION;
            while(action != STAND){
                std::cout << "Player " << i+1 << std::endl; 
                action = players[i].get_player_action();
                if(action == HIT){
                    players[i].hit(next_card());
                    BlackjackHand player_hand = players[i].get_hand();
                    std::cout << "Player Hits: " << player_hand << std::endl;
                } else if(action == DOUBLE){
                    players[i].double_down(next_card());
                    BlackjackHand player_hand = players[i].get_hand();
                    std::cout << "Player Doubles Down: " << player_hand << std::endl;
                    break;
                } else if(action != STAND){
                    std::cout << "Unsupported action (sorry)" << std::endl;
                }
                if(players[i].busted()){
                    std::cout << "Busted" << std::endl;
                    break;
                }
                //TODO surrender, split, (how should we handle paying out for surrender?)
                //TODO check for blackjack
            }
            dramatic_delay();
        }

        enum BlackjackPlayerActions action = NO_ACTION;
        while(action != STAND){
            action = dealer.get_player_action();
            if(action == HIT){
                dealer.hit(next_card());
                BlackjackHand dealer_hand = dealer.get_hand();
                std::cout << "Dealer Hits: " << dealer_hand << std::endl;
            } else if (action == NO_ACTION){
                std::cout << "ERROR: Dealer Had a Stroke" << std::endl;
                break;
            }
            if(dealer.busted()){
                std::cout << "Dealer Busted" << std::endl;
                break;
            }
            dramatic_delay();
        }
        BlackjackHand dealer_hand = dealer.get_hand();
        std::cout << "Dealer's hand: " << dealer_hand << std::endl;

        for(i=0; i<players.size(); ++i){
            if(players[i].busted()){
                std::cout << "Player " << i+1 << " Loses\n" << std::endl;
            } else if (dealer.busted()){
                std::cout << "Player " << i+1 << " Wins!\n" << std::endl;
                players[i].pay();
            } else if(players[i].get_hand_value() > dealer.get_hand_value()){
                std::cout << "Player " << i+1 << " Wins!\n" << std::endl;
                players[i].pay();
            } else if (players[i].get_hand_value() == dealer.get_hand_value()){
                std::cout << "Player " << i+1 << " Pushes\n" << std::endl;
                players[i].push();
            } else {
                std::cout << "Player " << i+1 << " Loses\n" << std::endl;
            }
            players[i].clear();
            dramatic_delay();
        }

        dealer.clear();

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

