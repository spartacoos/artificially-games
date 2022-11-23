#include <iostream>
#include "game.h"


auto main() -> int {
    Game::score s = 4;
    int val = Game::BadInput::non_exempt();
    // using Game::SquareState;
    // Empty;
    Game::BoardState bds{};
    // bds.winner = true;
    bds.make_winner_true();
    std::cout << "Winner: " << bds.winner << std::endl;
    Game::SquareState e = Game::SquareState::Empty;
    std::cout << "Hey "<< s << Game::WinStates[4].name << std::endl;
}
