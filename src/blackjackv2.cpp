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

enum BlackjackPlayerActions
{
    NO_ACTION,
    STAND,
    HIT,
    DOUBLE,
    SPLIT,
    SURRENDER
};

enum BlackjackCardSuit
{
	CLUBS,
	HEARTS,
	SPADES,
	DIAMONDS,
	_NUM_BLACKJACK_SUITS_
};

static std::string card_suits[] = {
	"Clubs",
	"Hearts",
	"Spades",
	"Diamonds"
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

struct BlackjackHandInfo
{
    unsigned int value;
    bool pair;
    bool soft;
};


class BlackjackCard
{
	enum BlackjackSuit suit;
	enum BlackjackCardValue value;
};

class BlackjackDeck
{
	Vector<BlackjackCard> deck;

	void shuffle(){}

	BlackjackCard& deal_card(){
		// pop card off of the deck
		// check remaining deck size here and various game rules
		// to decide whether or not to shuffle 
	}

};

class BlackjackHand
{
	unsigned int bet;
	bool can_split;
	bool can_double;
	bool is_busted;
	std::vector<BlackjackCards> cards;

	void deal(unsigned int initial_bet, BlackjackCard &card1, BlackjackCard &card2)
	{
		bet = initial_bet;
		cards.push_back(card1);
		cards.push_back(card2);
		can_double = true;
		if (this.info().pair) {
			can_split = true;
		}
		if (this.info().value > 21) {
			is_busted = true;
		}
	}	

	void hit(BlackjackCard &card)
	{
		cards.push_back(card);
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
	}

	struct BlackjackHandInfo info()
	{
        std::vector<BlackjackCards> sorted_hand = cards;
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
	int bankroll;
	unsigned int starting_bet;
	std::vector<BlackjackHand> hands;

	BlackjackPlayer()
	{
		bankroll = 1000;
		starting_bet = 10;
		hands.clear();
	}

	void payout(BlackjackHand &h) 
	{
		bankroll += 2*h.bet;
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
                case('P');
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
    enum BlackjackCards showing()
    {
        return hands[0].cards[0];
    }
};

class BlackjackGame
{
	std::vector<BlackjackPlayer> players;
	BlackjackDealer dealer;
	BlackjackDeck deck;
	struct BlackjackRules ruleset;

	BlackjackGame()
	{
		players.push_back(BlackjackPlayer());
	}
	BlackjackGame(int num_players) 
	{
		for(int i=0; i<num_players; ++i) {
			players.push_back(BlackjackPlayer());
		}
	}

	void deal_hand()
	{
		for (BlackjackPlayer p : players) {
			p.hands.clear();
			p.hands.push_back(BlackjackHand());

			p.hands[0].deal(deck.deal(), deck.deal(), p.initial_bet);
		}

		dealer.hands.clear();
		delaer.hands.push_back(BlackjackHand());
		dealer.hands[0].deal(deck.deal(), deck.deal(), 0);

		std::cout << "Dealer showing " << card_strings[dealer.showing()] << std::endl;

		for (int i=0; i < players.size(); ++i) {
			enum BlackjackPlayerActions action = NO_ACTION;
			BlackjackPlayer p = players[i];
			std::cout << "Player " << i+1  << ":" << std::endl;
			bool player_done = false;
			// easier to use while here than for loop because # of hands may change
			int j=0;
			while(!player_done) {
				if (p.hands.size() > 1) {
					std::cout << "Hand #" << j+1 << std::endl;
				}
				BlackjackHand hand = p.hands[j];
				while (action != STAND || action != DOUBLE) {
					action = p.get_player_action(hand);
					switch (action) {
						case HIT:
							p.banroll -= p.initial_bet;
							hand.hit(deck.deal());
							std::cout << "Player Hits: " << hand << std::endl;
							break;
						case STAND:
							break;
						case DOUBLE:
							p.bankroll -= p.initial_bet;
							hand.double(deck.deal());
							std::cout << "Player Doubles Down: " << hand << std::endl;
							break;
						case SPLIT:
							p.bankroll -= p.initial_bet;
							BlackjackHand new1 = BlackjackHand();
							BlackjackHand new2 = BlackjackHand();
							new1.deal(hand[0], deck.deal(), p.initial_bet);
							new2.deal(hand[1], deck.deal(), p.initial_bet);
							p.hands[j] = new1;
							p.hands.insert(j+1, new2);
							j = j-1; // need to reprocess this hand 
							std::cout << "Player Splits and is now player with " << p.hands.size() << "hands" << std::endl;
							break;
						default: 
							std::cout << "Unreachable state: The programmer is a moron" << std::endl;
					}
				}

				if (j < p.hands.size()) {
					++j;
				} else {
					player_done = true;
				}
			}
		}
	}
}
