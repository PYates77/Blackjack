all: blackjack_quiz blackjack_cli

blackjack_cli: blackjack_core.cpp blackjack_cli.cpp
	$(CXX) $^ -o $@

blackjack_quiz: blackjack_core.cpp blackjack_quiz.cpp
	$(CXX) $^ -o $@

clean:
	@rm -f blackjack_quiz blackjack_cli


