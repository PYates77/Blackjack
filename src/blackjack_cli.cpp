#include "blackjack.h"

int main()
{
    int i;
    BlackjackGame game(1);
    for(i=0; i<10; ++i){
        game.play_round();
    }

    return 0;
}
