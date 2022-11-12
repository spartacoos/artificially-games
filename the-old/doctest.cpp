// #include "game.cpp"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "game.cpp"


TEST_CASE("testing the heuristic_score function") {
    BoardState board{};
    MiniMax ai{SquareState::Circle};
    board.grid[0][0] = SquareState::Circle;
    board.grid[1][0] = SquareState::Circle;
    board.grid[1][1] = SquareState::Cross;
    score val = ai.heuristic_score(board, SquareState::Circle);
    int val = 20;
    CHECK(val == 20);
}
