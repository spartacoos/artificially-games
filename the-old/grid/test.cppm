module;
#include <tuple>
#include <vector>
#define DOCTEST_CONFIG_IMPLEMENT
// #define ENABLE_DEBUG // comment out to disable debugging
#include <bitset>
#include <cstdlib>
#include <iostream>
#include <regex>
// #include "doctest.h"
export module test;
typedef int score;

export
const std::uint8_t DIM = 3;
export
const std::uint8_t BOARD_SIZE = DIM * DIM;

int non_exempt_bad_input(){
    // for testing during development
    std::cout << "You gave me bad input to something that should not require an exemption! bad programmer, go to sleep!"<<std::endl;
    return EXIT_FAILURE;
}

struct WinningStates {
    std::bitset<BOARD_SIZE> bits{};
    std::string name;
};

WinningStates WinStates[8] = {  //there are  8 different conditions that the board can be in where somebody has won
    {std::bitset<BOARD_SIZE>(std::string("100010001")), std::string("diagonal (left-to-right) #1")},
    {std::bitset<BOARD_SIZE>(std::string("100100100")), std::string("vertical col #1")},
    {std::bitset<BOARD_SIZE>(std::string("010010010")), std::string("vertical col #2")},
    {std::bitset<BOARD_SIZE>(std::string("001001001")), std::string("vertical col #3")},
    {std::bitset<BOARD_SIZE>(std::string("111000000")), std::string("horizontal row #1")},
    {std::bitset<BOARD_SIZE>(std::string("000111000")), std::string("horizontal row #2")},
    {std::bitset<BOARD_SIZE>(std::string("000000111")), std::string("horizontal row #3")},
    {std::bitset<BOARD_SIZE>(std::string("001010100")), std::string("diagonal (right-to-left) #2")}
};

export
enum class SquareState {
  Empty,
  Circle,
  Cross
};

SquareState inverse_player(SquareState player){
    return (player == SquareState::Circle)? SquareState::Cross  :
           (player == SquareState::Cross) ? SquareState::Circle :
                                            SquareState::Empty;
}


export
struct Position {
    unsigned long x{};
    unsigned long y{};
};

struct Result {
    SquareState piece{};
    Position position{};
};

struct Move {
    score value{};
    Position pos{};
};

export
enum class game{started, finished, playing, popup};

export
struct BoardState {
    bool winner{};
    bool did_circle_win{};
    bool did_cross_win{};
    bool draw_condition{};
    game state{}; //= game::started;
    SquareState grid[DIM][DIM]{};
    bool player_made_move{};

    // BoardState(){
    //     winner = false;
    // }
    void reset(){
        winner = false;
        state = game::started;
        player_made_move = false;
        did_circle_win = false;
        did_cross_win = false;
        draw_condition = false;
        for(std::size_t x = 0; x < DIM; x++){
            for(std::size_t y =0; y < DIM; y++){
                grid[x][y] = SquareState::Empty;
            }
        }
    }

    bool is_full(){
        if(empty_slots().size()>0){
            return false;
        } else {
            return true;
        }
    }

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
                print_state_at_location(i,j);
                place_holder = read_state_at_location(i,j);
                std::cout << place_holder << " |";
            }
        std::cout << "\n-------------------------" << std::endl;
        }
    }


    auto serialize_board_state_for_player(SquareState player)-> std::bitset<BOARD_SIZE> {
        std::bitset<BOARD_SIZE> bits{};
        for(auto i = 0; i < DIM; i++) {
            for(auto j = 0; j < DIM; j++) {
                if(grid[i][j] == player){
                    bits.set((3*i)+j);
                }
            }
        }
        return bits;
    }

    auto get_position_for_bitset_bit(std::size_t index){
        // Given a bitset representing the serialized state of the board and a given index in the bitset
        // return the (x,y) coordinates corresponding to that bit in the board's grid
        return (index == 0)?Position{.x=0,.y=0}:
               (index == 1)?Position{.x=0, .y=1}:
               (index == 2)?Position{.x=0, .y=2}:
               (index == 3)?Position{.x=1, .y=0}:
               (index == 4)?Position{.x=1, .y=1}:
               (index == 5)?Position{.x=1, .y=2}:
               (index == 6)?Position{.x=2, .y=0}:
               (index == 7)?Position{.x=2, .y=1}:
               Position{.x=2, .y=2};
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
        std::bitset<BOARD_SIZE> serialized = serialize_board_state_for_player(player);
        for(auto [state,name]:WinStates){
            if(state==(serialized & state)) {
                //std::cout<< "rule: " << name << std::endl;
                return true;
            }
        }
        return false;
        }

    auto string_to_piece_type_enum(std::string const& piece_str){
        if(piece_str == "Circle" || piece_str == "o") return SquareState::Circle;
        if(piece_str == "Cross" || piece_str == "x")  return SquareState::Cross;
        std::exit(non_exempt_bad_input());
        return SquareState::Empty;
    }

    auto get_user_input(){
        std::string s;
        //std::cout << "Enter move e.g x(a,b) or o(c,d): ";
        std::cin >>  s;
        std::regex e{"(o|x).(\\d+),(\\d+)."};
        std::smatch m;
        if(std::regex_match(s, m, e))
        {
            auto piece = string_to_piece_type_enum(m.str(1));
            auto position = Position{.x = std::stoul(m.str(2)), .y = std::stoul(m.str(3))};
            return Result{piece, position};
        }
        //std::cout << "Wrong input!!! - I'm crashing now....";
        std::exit(non_exempt_bad_input());
        return Result{};

    }

    std::vector<Position> empty_slots(){
        std::vector<Position> positions;
        for(auto i = 0UL; i < DIM; i++){
            for(auto j = 0UL; j < DIM; j++){
                if(grid[i][j] == SquareState::Empty){
                    positions.push_back({.x=i, .y=j});
                }
            }
        }
        return positions;
    }

    bool is_draw(){
        if(is_full()){
            if(!(exhaustive_check_for_winner(SquareState::Circle) || exhaustive_check_for_winner(SquareState::Cross))){
                return true;
            } else {
                return false;
            }
        } else{
            return false;
        }
    }
};


struct AI {
    AI(SquareState maximizing_player):maximizing_player{maximizing_player}{}
    ~AI()= default;
    virtual Position next_move(BoardState board)=0;
    SquareState maximizing_player; //cross or circle
};

struct RandomMover: AI {
        RandomMover(SquareState maximizing_player) : AI(maximizing_player){}
        Position next_move(BoardState board){
            //check if there are available squares on the board, if so, record their indices
            // Looking at which locations on the board are empty i.e available to make a move
            std::bitset<BOARD_SIZE> current_board_state = board.serialize_board_state_for_player(SquareState::Empty);
            std::vector<int> indices;
            for(std::size_t i=0; i < BOARD_SIZE; i++){
                auto bit = current_board_state[i];
                if(bit == 1) {
                    indices.push_back(i); //this i represents the index on the board for an empty spot
                }
            }
            std::size_t random_number{rand()%(indices.size()-1)};
            int random_bit_to_flip = indices[random_number];
            return board.get_position_for_bitset_bit(random_bit_to_flip);//current_board_state.flip(random_bit_to_flip);
    }
};

export
struct MiniMax: AI {
    MiniMax(SquareState maximizing_player) : AI(maximizing_player){}
    Position next_move(BoardState board){
        // might want to do a check to ensure there are possible moves left (i.e empty slots) otherwise best_move might contain garbage vallues, worry abt that later
        int depth = 0;//BOARD_SIZE-1;//(DIM * DIM); //* 5;
        score best_score{INT_MIN};
        //Possible moves
        std::vector<Position> possible_moves = board.empty_slots();
        // return minimax(board, depth, true).pos;
        Position best_move{possible_moves[0].x,possible_moves[0].y};
        //iterate over possible moves given board state (i.e playing as maximizing_player) then pick the one with higest score
        for(auto move: possible_moves){
            auto [x,y] = move;
            BoardState child_board = board;
            child_board.grid[x][y] = maximizing_player;
            score curr_value = minimax(child_board, depth,false);
            if(curr_value > best_score){
                best_score = curr_value;
                best_move = {x,y};
            }
        }
        return best_move;
    }
    score minimax(BoardState board, unsigned int depth, bool playing_as_maximizer){
        score val = heuristic(board);
        if(val == 10){
            return 10-depth;
        }
        if(val == -10){
            return -10+depth;
        }

        if(board.is_full()){
            return 0;
        }

        if(playing_as_maximizer){
           // score value = INT_MIN;
           score value = -1000;
           for(auto move: board.empty_slots()){
               auto [x,y] = move;
               BoardState child = board;
               child.grid[x][y] = maximizing_player;
               value = std::max(value, minimax(child, depth+1, !playing_as_maximizer));
           }
           return value;
        } else {
           score value = 1000;
           for(auto move: board.empty_slots()){
               auto [x,y] = move;
               BoardState child = board;
               child.grid[x][y] = inverse_player(maximizing_player);
               value = std::min(value, minimax(child, depth+1, !playing_as_maximizer));
           }
           return value;
        }
    }

    score heuristic(BoardState &board){
           std::bitset<BOARD_SIZE> serialized_board_for_maxim_player = board.serialize_board_state_for_player(maximizing_player);
           std::bitset<BOARD_SIZE> serialized_board_for_minim_player = board.serialize_board_state_for_player(inverse_player(maximizing_player));
           for(auto rule:WinStates){
               if((rule.bits & serialized_board_for_maxim_player) == rule.bits){
                  return 10;
               }
               if((rule.bits & serialized_board_for_minim_player) == rule.bits){
                   return -10;
               }
           }
           return 0;
    }
};
