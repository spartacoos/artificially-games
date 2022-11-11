#include <tuple>
#include <vector>
#define DOCTEST_CONFIG_IMPLEMENT
#define X_DIM 3
#define Y_DIM 3
#define BOARD_SIZE (X_DIM)*(Y_DIM)
// #define ENABLE_DEBUG // comment out to disable debugging
#include <bitset>
#include <cstdlib>
#include <iostream>
#include <regex>
#include "doctest.h"
typedef int score;

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

struct BoardState {
    bool winner;
    SquareState grid[X_DIM][Y_DIM]{};

    BoardState(){
        winner = false;
    }



    bool is_full(){
        // bool board_full = true;
        // for(auto square: grid){
        //     if (*square == SquareState::Empty) {
        //         board_full = false;
        //         return board_full;
        //     }
        // }
        if(empty_slots().size()>0){
            //not empty
            return false;
        } else {
            return true;
        }
        // return board_full;
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
                // print_state_at_location(i,j);
                place_holder = read_state_at_location(i,j);
                std::cout << place_holder << " |";
            }
        std::cout << "\n-------------------------" << std::endl;
        }
    }


    auto serialize_board_state_for_player(SquareState player)-> std::bitset<BOARD_SIZE> {
        std::bitset<BOARD_SIZE> bits{};
        for(auto i = 0; i < X_DIM; i++) {
            for(auto j = 0; j < Y_DIM; j++) {
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
                std::cout<< "rule: " << name << std::endl;
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
        std::cout << "Enter move e.g x(a,b) or o(c,d): ";
        std::cin >>  s;
        std::regex e{"(o|x).(\\d+),(\\d+)."};
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

    std::vector<Position> empty_slots(){
        std::vector<Position> positions;
        for(auto i = 0UL; i < X_DIM; i++){
            for(auto j = 0UL; j < Y_DIM; j++){
                if(grid[i][j] == SquareState::Empty){
                    positions.push_back({.x=i, .y=j});
                }
            }
        }
        return positions;
    }
};


struct AI {
    AI(SquareState maximizing_player):maximizing_player{maximizing_player}{}
    ~AI()= default;
    virtual Position next_move(BoardState &board)=0;
    SquareState maximizing_player; //cross or circle
};

struct RandomMover: AI {
        RandomMover(SquareState maximizing_player) : AI(maximizing_player){}
        Position next_move(BoardState &board){
            //check if there are available squares on the board, if so, record their indices
        //     std::map<int,bool> available = {
        //     {},
        // };
            // Looking at which locations on the board are empty i.e available to make a move
            std::bitset<BOARD_SIZE> current_board_state = board.serialize_board_state_for_player(SquareState::Empty);
            // int count = current_board_state.count();
            std::vector<int> indices;
            for(std::size_t i=0; i < BOARD_SIZE; i++){
                auto bit = current_board_state[i];
                if(bit == 1) {
                    indices.push_back(i); //this i represents the index on the board for an empty spot
                }
            }
            // std::cout << "HEREEEE";
            // std::cout << "number of empty spots on board:: " << indices.size() << "\n";
            // std::cout << indices[0] << " : 0 \n";
            // std::cout << indices[1]<< " : 1 \n";
            // std::cout << indices[2]<< " : 2 \n";
            //pick random usign count
            std::size_t random_number{rand()%(indices.size()-1)};
            int random_bit_to_flip = indices[random_number];
            // board
            // need to have a way to go from serialized board -> grid board
            // noooo, wait, think about it, you just need to go from:
            // index in serialized_board -> index(x,y)@board_grid

            return board.get_position_for_bitset_bit(random_bit_to_flip);//current_board_state.flip(random_bit_to_flip);

            // int count = current_board_state.
            //pick a random index from list above
            //toggle that corresponding bit on current_board_state and return it?? or should I translate serialized -> Position??
    }
};

struct MiniMax: AI {
    MiniMax(SquareState maximizing_player) : AI(maximizing_player){}
    Position next_move(BoardState &board){
        int depth = (X_DIM * Y_DIM); //* 5;
        return minimax(board, depth, true).pos;
    }
    Move minimax(BoardState &board, int depth, bool playing_as_maximizer){

        // std::cout << "HEREEEEEE MA FRIEND : fullnesss::>> " << board.is_full();
        if(depth==0 || board.is_full()){
            return heuristic_score(board, maximizing_player);
        }

        if(playing_as_maximizer){
            std::vector<std::tuple<score,unsigned long,unsigned long>> possible_positions{};
            score value = INT_MIN;
            for(auto empty_square: board.empty_slots()){
                auto [x,y] = empty_square;
                BoardState child_board = board; //need to make it deep copy semantics, currently only copies ref
                child_board.grid[x][y] = SquareState::Circle;
                value = std::max(value,minimax(child_board, depth-1, false).value);
                possible_positions.push_back(std::make_tuple(value,x,y));
            }
            // Move m = {.value=value, .pos={.x=0, .y=0}};
            // Find best position (i.e the x&y pair that yields highest value)
            sort(possible_positions.begin(), possible_positions.end());
            // int idx = possible_positions.size()-1;
            int idx = 0;
            Position best = {std::get<1>(possible_positions[idx]), std::get<2>(possible_positions[idx])};
            std::cout << "Possible moves:  "<< possible_positions.size() << "\n";
            std::cout << "LAST MOVE IN VECS OF MOVES \n";
            std::cout << std::get<0>(possible_positions[possible_positions.size()-1])
                      << ", " << std::get<1>(possible_positions[possible_positions.size()-1])
                      << ", " << std::get<2>(possible_positions[possible_positions.size()-1]) << "\n";


            std::cout << "FIRST MOVE IN VEC \n";
            std::cout << std::get<0>(possible_positions[0])
                      << ", " << std::get<1>(possible_positions[0])
                      << ", " << std::get<2>(possible_positions[0]) << "\n";

            // return {value, {.x=0, .y=0}};
            return {value, best};
        }else{
            std::vector<std::tuple<score,unsigned long,unsigned long>> possible_positions{};
            score value = INT_MAX;
            for(auto empty_square: board.empty_slots()){
                auto [x,y] = empty_square;
                BoardState child_board = board; //need to make it deep copy semantics, currently only copies ref
                child_board.grid[x][y] = SquareState::Cross;
                value = std::min(value,minimax(child_board, depth-1, true).value);
                possible_positions.push_back(std::make_tuple(value,x,y));
            }
            sort(possible_positions.begin(), possible_positions.end());
            int idx = possible_positions.size()-1;
            // int idx = 0;
            Position best = {std::get<1>(possible_positions[idx]), std::get<2>(possible_positions[idx])};
            return {value, best};
        }
    }
    Move heuristic_score(BoardState &board, SquareState maximizing_player){
        // For all rules (WinStates) tally how many our maximizing_player overlaps
        // also tally how many of maximizing_player pieces are in each rule.
        // return the product of these two numbers.
        // Position heuristic_position{};
        auto tally_rules_hit{0};
        auto tally_mplayer_pieces{0};
        for(auto rule: WinStates){
            auto serialized_board_for_maximizing_player = board.serialize_board_state_for_player(maximizing_player);
            auto bit_overlap = (serialized_board_for_maximizing_player & rule.bits);
            if(bit_overlap.any()){ //overlaps with rule (i.e at least one piece of MPlayer is hitting this rule)
                tally_rules_hit++;
                // count how many of those bits were hit (i.e how many pieces MPlayer has in that rule)
                // because having two pieces in a line already is much better than only having one.
                tally_mplayer_pieces += bit_overlap.count();
            }
        }

        // For whatever the matched/hit rule with highest value (tallies), I want an available
        // position in that line.
        // HANG ON A SECOND, we don't actually care about the move for a heuristic function
        // since the board is already FULL, we just want the value.

        #ifdef ENABLE_DEBUG
        std::cout << "Mplayer pieces: " << tally_mplayer_pieces << "\n";
        std::cout << "Rules hit: " << tally_rules_hit << "\n";
        std::cout << "-------------------------------------- \n";
        #endif

        Move heuristic_move{.value = tally_mplayer_pieces * tally_rules_hit,
        // Move heuristic_move{.value = tally_mplayer_pieces ,
        // Move heuristic_move{.value = tally_rules_hit,
            .pos = {.x = 2, .y = 2}
        };
        return heuristic_move;
    }
};

auto main(int argc, const char** argv) -> int {
    //Setting up the testing environment
    doctest::Context ctx;
    ctx.setOption("abort-after", 2);  // stop testing after 2 failed test
    ctx.applyCommandLine(argc, argv); // pass cli args to test framework
    ctx.setOption("no-breaks", true); // don't break when jumping into the debugger
    int test_results = ctx.run();     // run test cases unless passed "--no-run" in which case only the game code runs
    if(ctx.shouldExit())              // framework relies on this stopping mechanism (i.e when we run only the tests)
        return test_results;                   

    int successful_game_flag = EXIT_SUCCESS;
    BoardState board{};
    // RandomMover ai{SquareState::Circle};
    MiniMax ai{SquareState::Circle};
    // std::cout << ai.next_move(BoardState &board)
    // for(int i = 0; i < X_DIM; i++){
    //     for(int j=0; j < Y_DIM; j++){
    //         board.grid[i][j] = SquareState::Circle;
    //     }
    // }

    // std::cout << "HEREEEEEE MA FRIEND : fullnesss::>> " << board.is_full();
    board.grid[0][0] = SquareState::Circle;
    board.grid[0][1] = SquareState::Cross;
    board.grid[0][2] = SquareState::Circle;
    board.grid[1][0] = SquareState::Cross;
    board.grid[1][1] = SquareState::Empty;
    board.grid[1][2] = SquareState::Cross;
    board.grid[2][0] = SquareState::Empty;
    board.grid[2][1] = SquareState::Empty;
    board.grid[2][2] = SquareState::Circle;

    // std::cout << "HEREEEEEE MA FRIEND : fullnesss::>> " << board.is_full();
    // board.print_board_state();

    // score val = ai.heuristic_score(board, SquareState::Circle);
    // std::cout << "Value: " << val << "\n";
    // std::cout << "Fullness: " << board.is_full() << "\n";
    // exi(0);
    bool did_circle_win = board.exhaustive_check_for_winner(SquareState::Circle);
    bool did_cross_win = board.exhaustive_check_for_winner(SquareState::Cross);

    while(!did_circle_win && !did_cross_win){

        Position ai_pos = ai.next_move(board);
        board.grid[ai_pos.x][ai_pos.y] = SquareState::Circle;

        board.print_board_state();
        Result res = board.get_user_input();
        board.grid[res.position.x][res.position.y] = res.piece;

        // std::cout << "HEREEEE";
        // Position ai_pos = ai.next_move(board);
        // board.grid[ai_pos.x][ai_pos.y] = SquareState::Circle;
        did_circle_win = board.exhaustive_check_for_winner(SquareState::Circle);
        did_cross_win = board.exhaustive_check_for_winner(SquareState::Cross);
    }

    board.print_board_state();
    did_circle_win? std::cout << "Circle is the winner!" << std::endl : std::cout << "Cross is the winner!" << std::endl;
    return test_results + successful_game_flag;
}


// Test cases (OPTIONAL)

TEST_CASE("testing the heuristic_score function") {
    // Base test conditions, objects and functions to be tested:
    BoardState board{};
    SquareState maximizing_player = SquareState::Circle;
    MiniMax ai{maximizing_player};
    REQUIRE(board.winner == false);
    board.grid[0][0] = maximizing_player;
    board.grid[1][0] = maximizing_player;
    board.grid[1][1] = inverse_player(maximizing_player);

    SUBCASE("Base: two circles in vertical column, one cross in centre"){
        score val = ai.heuristic_score(board, maximizing_player).value;
        CHECK(val == (4*5));
    }

    SUBCASE("two circles in vertical column, one circle in middle of last column, one cross in centre"){
        board.grid[1][2] = maximizing_player;
        score val = ai.heuristic_score(board, maximizing_player).value;
        CHECK(val == (5*7));
    }

    SUBCASE("two circles in vertical column, one cross in the centre, one cross in bottom right corner"){
        board.grid[2][2] = inverse_player(maximizing_player);
        score val = ai.heuristic_score(board, maximizing_player).value;
        CHECK(val == (4*5));
    }
}


TEST_CASE("testing board.is_full() method when board is FULL"){
    BoardState board{};
    for(int i = 0; i < X_DIM; i++){
        for(int j=0; j < Y_DIM; j++){
            board.grid[i][j] = SquareState::Circle;
        }
    }

    #ifdef ENABLE_DEBUG
    board.print_board_state();
    std::cout << "Fullness: " << board.is_full() << "\n";
    #endif

    CHECK(board.is_full() == true);
}

TEST_CASE("testing board.is_full() method when board is EMPTY"){
    BoardState board{};
    for(int i = 0; i < X_DIM; i++){
        for(int j=0; j < Y_DIM; j++){
            board.grid[i][j] = SquareState::Empty;
        }
    }

    #ifdef ENABLE_DEBUG
    board.print_board_state();
    std::cout << "Fullness: " << board.is_full() << "\n";
    #endif

    CHECK(board.is_full() == false);
}
