
#include "blackjack.h"

/* TODO:
 *  Insurance
 *  Double after split
 *  Min double value
 *  Max split hands
 *  Resplit aces
 *  Hit split aces
 *  Surrender
 */

static bool dramatic_delay_enabled = true;

enum BlackjackGamePhases
{
    DEAL,
    INSURANCE,
    PROCESS_MOVES,
    PAYOUT,
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
    .insurance = true,
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

/* for calculating statistics */
struct BlackjackDeckComposition
{
    unsigned int num_cards;
    unsigned int num_A;
    unsigned int num_10;
    unsigned int num_9;
    unsigned int num_8;
    unsigned int num_7;
    unsigned int num_6;
    unsigned int num_5;
    unsigned int num_4;
    unsigned int num_3;
    unsigned int num_2;
};

//void get_composition(BlackjackDeck &d, BlackjackDeckComposition &c)
//{
//    c = {}; // zero out struct
//    for (auto &card : d.deck) {
//    }
//}

void dramatic_delay()
{
    if(dramatic_delay_enabled){
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

BlackjackCard::BlackjackCard(enum BlackjackSuit s, enum BlackjackCardValue v) {
    suit = s;
    value = v;
}

bool BlackjackCard::operator<(const BlackjackCard& r) {
    return this->value < r.value;
}

bool BlackjackCard::operator==(enum BlackjackCardValue v) {
    return this->value == v;
}

/* reinits the deck with all cards and shuffles the cards */
void BlackjackDeck::shuffle()
{
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

BlackjackDeck::BlackjackDeck() 
{
    num_decks = 1;
    shuffle();
}

BlackjackDeck::BlackjackDeck(int num_decks) 
{
    this->num_decks = num_decks;
    shuffle();
}

BlackjackCard& BlackjackDeck::deal()
{
    // pop card off of the deck
    BlackjackCard &c = deck.back();
    deck.pop_back();
    return c;
}

int BlackjackDeck::cards_remaining()
{
    return deck.size();
}

void BlackjackHand::deal(BlackjackCard &card1, BlackjackCard &card2, unsigned int initial_bet)
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

void BlackjackHand::hit(BlackjackCard &card)
{
    can_split = false;
    blackjack = false;
    cards.push_back(card);
    if (this->info().value > 21) {
        is_busted = true;
    }
}

void BlackjackHand::double_down(BlackjackCard &card)
{
    cards.push_back(card);
    bet *= 2;
    if (this->info().value > 21) {
        is_busted = true;
    }
}

void BlackjackHand::clear()
{
    cards.clear();
    bet = 0;
    can_split = false;
    can_double = false;
    is_soft = false;
    is_busted = false;
    blackjack = false;
}

struct BlackjackHandInfo BlackjackHand::info()
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

std::ostream& operator<<(std::ostream& str, BlackjackHand &h) {
    struct BlackjackHandInfo info = h.info();
    str << "[" << h.cards.size() << "] ";
    if(info.soft){
        for(BlackjackCard card : h.cards){
            str << card_strings[card.value] << card_suits[card.suit] << " ";
        }
        str << " (soft " << info.value << ")";
    } else {
        for(BlackjackCard card : h.cards){
            str << card_strings[card.value] << card_suits[card.suit] << " ";
        }
        str << " (" << info.value << ")";
    }
    return str;
}

BlackjackPlayer::BlackjackPlayer()
{
    bankroll = 1000;
    initial_bet = 10;
    hands.clear();
}

void BlackjackPlayer::payout(BlackjackHand &h)
{
    /* player gets back their initial bet plus the value of the bet */
    bankroll += 2*h.bet;
    h.clear();
}

void BlackjackPlayer::payout_blackjack(BlackjackHand &h)
{
    /* player gets back their initial bet plus the value of the blackjack */
    bankroll += h.bet + ruleset->blackjack_payout * h.bet;
    h.clear();
}

void BlackjackPlayer::push(BlackjackHand &h)
{
    bankroll += h.bet;
    h.clear();
}

enum BlackjackPlayerActions BlackjackPlayer::get_player_action(BlackjackHand &hand)
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

BlackjackDealer::BlackjackDealer(){
    ruleset = &default_ruleset;
}

enum BlackjackPlayerActions BlackjackDealer::get_player_action()
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

BlackjackCard BlackjackDealer::showing()
{
    return hands[0].cards[0];
}

BlackjackGame::BlackjackGame()
{
    srand(time(0));
    ruleset = &default_ruleset;
    players.push_back(BlackjackPlayer());
}

BlackjackGame::BlackjackGame(int num_players)
{
    srand(time(0));
    ruleset = &default_ruleset;
    for(int i=0; i<num_players; ++i) {
        players.push_back(BlackjackPlayer());
    }
}

void BlackjackGame::ask_insurance()
{
    // TODO
    std::cout << "TODO: ask for insurance" << std::endl;
}

void BlackjackGame::payout_insurance()
{
    // TODO
    std::cout << "TODO: payout insurance" << std::endl;
}

void BlackjackGame::deal_hand()
{
    /*
     * TODO: mathematically it shouldn't make a difference
     * but possibly want to refactor this to deal cards in the correct order
     * that way it could be animated later
     */
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
}

void BlackjackGame::process_player_move(BlackjackPlayer &p)
{
    // we print bankroll + initial bet because the player hasn't actually lost their initial bet yet
    // but internally the initial bet currently belongs to the dealt hand
    bool player_done = false;
    // easier to use while here than for loop because # of hands may change
    int j=0;
    while(!player_done) {
        if (p.hands.size() > 1) {
            std::cout << "Hand #" << j+1 << std::endl;
        }
        BlackjackHand &hand = p.hands[j];
        enum BlackjackPlayerActions action = NO_ACTION;
        if (hand.blackjack) {
            std::cout << "Blackjack!!!" << std::endl;
            action = STAND;
        }
        while (action != STAND && action != DOUBLE && action != SPLIT && !hand.is_busted) {
            action = p.get_player_action(hand);
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
                    p.bankroll -= p.initial_bet;
                    hand.double_down(deck.deal());
                    std::cout << "Player Doubles Down: " << hand << std::endl;
                    break;
                case SPLIT:
                    {
                        p.bankroll -= p.initial_bet;
                        BlackjackHand new1 = BlackjackHand();
                        BlackjackHand new2 = BlackjackHand();
                        new1.deal(hand.cards[0], deck.deal(), p.initial_bet);
                        new2.deal(hand.cards[1], deck.deal(), p.initial_bet);
                        p.hands[j] = new1;
                        p.hands.insert(p.hands.begin()+j+1, new2);
                        j = j-1; // need to reprocess this hand
                    }

                    std::cout << "Player Splits and now has " << p.hands.size() << " hands" << std::endl;
                    break;
                default:
                    std::cout << "Unreachable state: The programmer is a moron" << std::endl;
            }
        }

        if (j+1 < p.hands.size()) {
            ++j;
        } else {
            player_done = true;
        }
    }

}

void BlackjackGame::process_dealer_move()
{
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

}

void BlackjackGame::payout_player(BlackjackPlayer &p)
{
    for (int j=0; j<p.hands.size(); ++j) {
        if (p.hands.size() > 1) {
            std::cout << std::endl << "Hand " << j+1 << " ";
        }

        //std::cout << "(" << p.hands[j].info().value << ") ";
        std::cout << "(" << p.hands[j].info().value << ") ";
        if(p.hands[j].is_busted){
            std::cout << "Loses\n" << std::endl;
        } else if (dealer.hands[0].is_busted){
            std::cout << "Wins!\n" << std::endl;
            p.payout(p.hands[j]);
        } else if(p.hands[j].blackjack && p.hands[j].info().value > dealer.hands[0].info().value){
            std::cout << "Wins with Blackjack!\n" << std::endl;
            p.payout_blackjack(p.hands[j]);
        } else if(p.hands[j].info().value > dealer.hands[0].info().value){
            std::cout << "Wins!\n" << std::endl;
            p.payout(p.hands[j]);
        } else if (p.hands[0].info().value == dealer.hands[0].info().value){
            std::cout << "Pushes\n" << std::endl;
            p.push(p.hands[j]);
        } else {
            std::cout << "Loses\n" << std::endl;
        }
        dramatic_delay();
    }
}

void BlackjackGame::play_round()
{
    bool dealer_blackjack = false;
    deal_hand();

    std::cout << "Dealer showing " << card_strings[dealer.showing().value] << card_suits[dealer.showing().suit] << std::endl << std::endl;

    if (dealer.showing() == ACE &&& ruleset->dealer_checks_blackjack) {
        if (ruleset->insurance) {
            ask_insurance();
        }

        if (dealer.hands[0].blackjack) {
            std::cout << "Dealer Blackjack!" << std::endl;
            dealer_blackjack = true;

            if (ruleset->insurance) {
                payout_insurance();
            }
        }

    }

    /* if dealer got a blackjack, then no need to process player moves */
    if (!dealer_blackjack) {
        for (int i=0; i < players.size(); ++i) {
            std::cout << "Player " << i+1  << ": $" << players[i].bankroll + players[i].initial_bet << std::endl;
            process_player_move(players[i]);
        }
    }

    process_dealer_move();

    for (int i=0; i < players.size(); ++i) {
        std::cout << "Player " << i+1 << " ";
        payout_player(players[i]);
    }
}
