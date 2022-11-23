#ifndef GAME_H_
#define GAME_H_

#include <iostream>

#define X_DIM 3
#define Y_DIM 3
#define BOARD_SIZE (X_DIM)*(Y_DIM)

namespace Game{
    // Types (Typedefs/Enums/Structs/Objects)
    typedef int score;
    struct WinningStates {
        std::bitset<BOARD_SIZE> bits{};
        std::string name;
    };
    enum class SquareState {Empty, Circle, Cross};
    struct Position;
    struct Result;
    struct Move;
    struct BoardState {
        bool winner;
        SquareState grid[X_DIM][Y_DIM]{};
        BoardState();
        bool is_full();
        std::string read_state_at_location(std::size_t x, std::size_t y);
        void print_state_at_location(std::size_t x, std::size_t y);
        void print_board_state();
        auto serialize_board_state_for_player(Game::SquareState player)-> std::bitset<BOARD_SIZE>;
        auto get_position_for_bitset_bit(std::size_t index);
        auto exhaustive_check_for_winner(Game::SquareState player) -> bool;
        auto string_to_piece_type_enum(std::string const& piece_str);
        auto get_user_input();
        std::vector<Game::Position> empty_slots();
        bool is_draw();
        void make_winner_true();
    };
    struct AI {
        SquareState maximizing_player;
        AI(SquareState maximizing_player);
        ~AI();
    virtual Position next_move(BoardState board)=0;
    };
    extern struct WinningStates WinStates[];
    // Error Handling
    namespace BadInput{
        int non_exempt();
    }
}
// Forward declarations
Game::SquareState inverse_player(Game::SquareState player);

#endif // GAME_H_
