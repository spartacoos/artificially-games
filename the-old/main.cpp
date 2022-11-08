#include <cstdlib>
#include <iostream>
#include <regex>
#include <string>
#define X_DIM 3
#define Y_DIM 3

/*
** TODO:
** 3. Make dumbAssAI that simply makes a random move out of the possible ones
** 4. Make an AI that looks ahead at the possible states recursively and picks best choice out of all possible states
 */

int non_exempt_bad_input(){
    // for testing during development
    std::cout << "You gave me bad input to something that should not require an exemption! bad programmer, go to sleep!"<<std::endl;
    return EXIT_FAILURE;
}

struct WinningStates {
    std::bitset<9> bits{};
    std::string name;
};

WinningStates WinStates[8] = {  //there are  8 different conditions that the board can be in where somebody has won
    {std::bitset<9>(std::string("100010001")), std::string("diagonal (left-to-right) #1")},
    {std::bitset<9>(std::string("100100100")), std::string("vertical col #1")},
    {std::bitset<9>(std::string("010010010")), std::string("vertical col #2")},
    {std::bitset<9>(std::string("001001001")), std::string("vertical col #3")},
    {std::bitset<9>(std::string("111000000")), std::string("horizontal row #1")},
    {std::bitset<9>(std::string("000111000")), std::string("horizontal row #2")},
    {std::bitset<9>(std::string("000000111")), std::string("horizontal row #3")},
    {std::bitset<9>(std::string("001010100")), std::string("diagonal (right-to-left) #2")}
};

enum class SquareState {
  Empty,
  Circle,
  Cross
};

struct Position {
    unsigned long x{};
    unsigned long y{};
};

struct Result {
    SquareState piece{};
    Position position{};
};

struct BoardState {
    bool winner = false;
    SquareState grid[X_DIM][Y_DIM];

    std::string read_state_at_location(std::size_t x, std::size_t y){
        switch (grid[x][y]) {
            case SquareState::Empty:
            {
                 return "Empty";
            } break;

            case SquareState::Circle:
            {
                 return "Circle";
            } break;


            case SquareState::Cross:
            {
                return "Cross";
            } break;
        }
    }

    void print_state_at_location(std::size_t x, std::size_t y){
        switch (grid[x][y]) {
            case SquareState::Empty:
            {
                std::cout<< "Empty" << std::endl;
            } break;

            case SquareState::Circle:
            {
                std::cout<< "Circle" << std::endl;
            } break;


            case SquareState::Cross:
            {
                std::cout<< "Cross" << std::endl;
            } break;
        }
    }

    void print_board_state(){
        std::string place_holder{};
        std::cout << "\n----[0]-----[1]----[2]---" << std::endl;
        for(auto i=0; i<3; i++){
            std::cout << i <<": ";
            for(auto j=0; j<3; j++){
                // print_state_at_location(i,j);
                place_holder = read_state_at_location(i,j);
                std::cout << place_holder << " |";
            }
        std::cout << "\n-------------------------" << std::endl;
        }
    }


    auto serialize_board_state_for_player(SquareState player)-> std::bitset<9> {
        std::bitset<9> bits{};
        for(auto i = 0; i < X_DIM; i++) {
            for(auto j = 0; j < Y_DIM; j++) {
                if(grid[i][j] == player){
                    bits.set((3*i)+j);
                }
            }
        }
        return bits;
    }

    auto exhaustive_check_for_winner(SquareState player) -> bool{
        // Time-complexity: O(n) or possible O(n^2) depending on whether the "==" operator is efficient on bitsets. A cursory search on google didn't yield useful information, need to look deeper.
        // Like, I assume that on modern hardware a bitset comparison should run at extremely fast speeds (it is just a bunch of parallel XNOR gates) but I'm not certain
        // if std::bitset and bitset<N>::operator== are implemented with specific hardware instructions or (more likely) in a naive way, i.e
        // simply looping over each bit then this means that the "state==serialized" is itself O(n) in which case this function is O(n^2) which is terrible, specially if I make the game-grid bigger.
        //
        // In any case, there is still an advantage in using this approach with bitsets in that:
        // 1. it is easy to save game states (even intermediate - i.e not finished games) during startup or save
        // 2. it is a very compact representation (memory-wise) at only 1-bit per square in grid
        // 3. conceptually easy to understand
        // 4. easy to auto-generate rules later on when we make the grid bigger than 3x3
        // 5. Easier for AI on top:
        //   e.g State-Machine-based -> can operate at the state-level
        //       Neural Net Based    -> can operate at the bits-level
        //
        // FUTURE WORK: the entireeee exhaustive rule check can be done via a single run of parallel binary operation but need to look into how the bitset "==" works under the hood and also
        // parallelize this for-loop using OpenMP or similar framework to do it in an architecture-independent way.
        std::bitset<9> serialized = serialize_board_state_for_player(player);
        for(auto [state,name]:WinStates){
            if(state==(serialized & state)) {
                std::cout<< "rule: " << name << std::endl;
                return true;
            }
        }
        return false;
        }

    auto string_to_piece_type_enum(std::string const& piece_str){
        if(piece_str == "Circle") return SquareState::Circle;
        if(piece_str == "Cross")  return SquareState::Cross;
        std::exit(non_exempt_bad_input());
        return SquareState::Empty;
    }

    auto get_user_input(){
        std::string s;
        std::cout << "Enter move e.g Cross@(x,y): ";
        std::cin >>  s;
        std::regex e{"(Circle|Cross).(\\d+),(\\d+)."};
        std::smatch m;
        if(std::regex_match(s, m, e))
        {
            auto piece = string_to_piece_type_enum(m.str(1));
            auto position = Position{.x = std::stoul(m.str(2)), .y = std::stoul(m.str(3))};
            return Result{piece, position};
        }
        std::cout << "Wrong input!!! - I'm crashing now....";
        // int f = (void) non_exempt_bad_input();
        std::exit(non_exempt_bad_input());
        return Result{};

    }
};

auto main() -> int {
    BoardState board{};
    // board.grid[0][1] = SquareState::Circle;
    // board.grid[1][1] = SquareState::Circle;
    // board.grid[2][1] = SquareState::Circle;
    // board.grid[1][2] = SquareState::Cross;
    // board.grid[1][0] = SquareState::Cross;
    bool did_circle_win = board.exhaustive_check_for_winner(SquareState::Circle);
    bool did_cross_win = board.exhaustive_check_for_winner(SquareState::Cross);
    while(!did_circle_win && !did_cross_win){
        board.print_board_state();
        Result res = board.get_user_input();
        board.grid[res.position.x][res.position.y] = res.piece;
        did_circle_win = board.exhaustive_check_for_winner(SquareState::Circle);
        did_cross_win = board.exhaustive_check_for_winner(SquareState::Cross);
    }

    board.print_board_state();
    did_circle_win? std::cout << "Circle is the winner!" << std::endl : std::cout << "Cross is the winner!" << std::endl;
    return 0;
}
