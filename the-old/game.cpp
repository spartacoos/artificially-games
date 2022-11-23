#include "game.h"
#include <tuple>
#include <vector>
#define DOCTEST_CONFIG_IMPLEMENT
// #define ENABLE_DEBUG // comment out to disable debugging
#include <bitset>
#include <cstdlib>
#include <iostream>
#include <regex>
#include "doctest.h"
// typedef unsigned long score;
typedef int score;

int Game::BadInput::non_exempt(){
    // for testing during development
    std::cout << "You gave me bad input to something that should not require an exception! bad programmer, go to sleep!"<<std::endl;
    return EXIT_FAILURE;
}


Game::WinningStates Game::WinStates[8] = {  //there are  8 different conditions that the board can be in where somebody has won
    {std::bitset<BOARD_SIZE>(std::string("100010001")), std::string("diagonal (left-to-right) #1")},
    {std::bitset<BOARD_SIZE>(std::string("100100100")), std::string("vertical col #1")},
    {std::bitset<BOARD_SIZE>(std::string("010010010")), std::string("vertical col #2")},
    {std::bitset<BOARD_SIZE>(std::string("001001001")), std::string("vertical col #3")},
    {std::bitset<BOARD_SIZE>(std::string("111000000")), std::string("horizontal row #1")},
    {std::bitset<BOARD_SIZE>(std::string("000111000")), std::string("horizontal row #2")},
    {std::bitset<BOARD_SIZE>(std::string("000000111")), std::string("horizontal row #3")},
    {std::bitset<BOARD_SIZE>(std::string("001010100")), std::string("diagonal (right-to-left) #2")}
};

Game::SquareState inverse_player(Game::SquareState player){
    return (player == Game::SquareState::Circle)? Game::SquareState::Cross  :
           (player == Game::SquareState::Cross) ? Game::SquareState::Circle :
                                            Game::SquareState::Empty;
}


struct Game::Position {
    unsigned long x{};
    unsigned long y{};
};

struct Game::Result {
    Game::SquareState piece{};
    Game::Position position{};
};

struct Game::Move {
    score value{};
    Game::Position pos{};
};


Game::BoardState::BoardState(){
    winner = false;
}

bool Game::BoardState::is_full(){
    if(empty_slots().size()>0){
        return false;
    } else {
        return true;
    }
}

std::string Game::BoardState::read_state_at_location(std::size_t x, std::size_t y){
    switch (grid[x][y]) {
        case Game::SquareState::Empty:
        {
                return "Empty";
        } break;

        case Game::SquareState::Circle:
        {
                return "Circle";
        } break;


        case Game::SquareState::Cross:
        {
            return "Cross";
        } break;
    }
}

void Game::BoardState::print_state_at_location(std::size_t x, std::size_t y){
    switch (grid[x][y]) {
        case Game::SquareState::Empty:
        {
            std::cout<< "Empty" << std::endl;
        } break;

        case Game::SquareState::Circle:
        {
            std::cout<< "Circle" << std::endl;
        } break;


        case Game::SquareState::Cross:
        {
            std::cout<< "Cross" << std::endl;
        } break;
    }
}

void Game::BoardState::print_board_state(){
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


auto Game::BoardState::serialize_board_state_for_player(Game::SquareState player)-> std::bitset<BOARD_SIZE> {
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

auto Game::BoardState::get_position_for_bitset_bit(std::size_t index){
    // Given a bitset representing the serialized state of the board and a given index in the bitset
    // return the (x,y) coordinates corresponding to that bit in the board's grid
    return (index == 0)?Game::Position{.x=0,.y=0}:
            (index == 1)?Game::Position{.x=0, .y=1}:
            (index == 2)?Game::Position{.x=0, .y=2}:
            (index == 3)?Game::Position{.x=1, .y=0}:
            (index == 4)?Game::Position{.x=1, .y=1}:
            (index == 5)?Game::Position{.x=1, .y=2}:
            (index == 6)?Game::Position{.x=2, .y=0}:
            (index == 7)?Game::Position{.x=2, .y=1}:
            Game::Position{.x=2, .y=2};
}

auto Game::BoardState::exhaustive_check_for_winner(Game::SquareState player) -> bool{
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
    for(auto [state,name]:Game::WinStates){
        if(state==(serialized & state)) {
            std::cout<< "rule: " << name << std::endl;
            return true;
        }
    }
    return false;
    }

auto Game::BoardState::string_to_piece_type_enum(std::string const& piece_str){
    if(piece_str == "Circle" || piece_str == "o") return Game::SquareState::Circle;
    if(piece_str == "Cross" || piece_str == "x")  return Game::SquareState::Cross;
    std::exit(Game::BadInput::non_exempt());
    return Game::SquareState::Empty;
}

auto Game::BoardState::get_user_input(){
    std::string s;
    std::cout << "Enter move e.g x(a,b) or o(c,d): ";
    std::cin >>  s;
    std::regex e{"(o|x).(\\d+),(\\d+)."};
    std::smatch m;
    if(std::regex_match(s, m, e))
    {
        auto piece = string_to_piece_type_enum(m.str(1));
        auto position = Game::Position{.x = std::stoul(m.str(2)), .y = std::stoul(m.str(3))};
        return Game::Result{piece, position};
    }
    std::cout << "Wrong input!!! - I'm crashing now....";
    // int f = (void) Game::BadInput::non_exempt();
    std::exit(Game::BadInput::non_exempt());
    return Game::Result{};

}

std::vector<Game::Position> Game::BoardState::empty_slots(){
    std::vector<Game::Position> positions;
    for(auto i = 0UL; i < X_DIM; i++){
        for(auto j = 0UL; j < Y_DIM; j++){
            if(grid[i][j] == Game::SquareState::Empty){
                positions.push_back({.x=i, .y=j});
            }
        }
    }
    return positions;
}

bool Game::BoardState::is_draw(){
    if(is_full()){
        if(!(exhaustive_check_for_winner(Game::SquareState::Circle) || exhaustive_check_for_winner(Game::SquareState::Cross))){
            return true;
        } else {
            return false;
        }
    } else{
        return false;
    }
}

void Game::BoardState::make_winner_true(){
    winner = true;
}

Game::AI::AI(SquareState maximizing_player):maximizing_player{maximizing_player}{}
Game::AI::~AI()= default;
// virtual Game::Position Game::AI::next_move(BoardState board)=0;

struct RandomMover: Game::AI {
        RandomMover(Game::SquareState maximizing_player) : AI(maximizing_player){}
        Game::Position next_move(Game::BoardState board){
            //check if there are available squares on the board, if so, record their indices
        //     std::map<int,bool> available = {
        //     {},
        // };
            // Looking at which locations on the board are empty i.e available to make a move
            std::bitset<BOARD_SIZE> current_board_state = board.serialize_board_state_for_player(Game::SquareState::Empty);
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
            //toggle that corresponding bit on current_board_state and return it?? or should I translate serialized -> Game::Game::Position??
    }
};

struct MiniMax: Game::AI {
    MiniMax(Game::SquareState maximizing_player) : AI(maximizing_player){}
    Game::Position next_move(Game::BoardState board){
        // might want to do a check to ensure there are possible moves left (i.e empty slots) otherwise best_move might contain garbage vallues, worry abt that later
        int depth = 0;//BOARD_SIZE-1;//(X_DIM * Y_DIM); //* 5;
        score best_score{INT_MIN};
        //Possible moves
        std::vector<Game::Position> possible_moves = board.empty_slots();
        // return minimax(board, depth, true).pos;
        Game::Position best_move{possible_moves[0].x,possible_moves[0].y};
        //iterate over possible moves given board state (i.e playing as maximizing_player) then pick the one with higest score
        for(auto move: possible_moves){
            auto [x,y] = move;
            Game::BoardState child_board = board;
            child_board.grid[x][y] = maximizing_player;
            // std::cout << "Addr original: " << &board << "\n";
            // std::cout << "Addr copy: " << &child_board<< "\n";
            // exit(0);
            // score curr_value = minimax(child_board, possible_moves.size() ,true);
            score curr_value = minimax(child_board, depth,false);
            if(curr_value > best_score){
                best_score = curr_value;
                best_move = {x,y};
            }
        }

        // std::cout << "HERREEE: \n" << "score: " << best_score << "  |  move: {" << best_move.x << ", " << best_move.y << "} \n";
        // exit(0);
        // if(possible_moves.size()==8){
        //     std::cout << best_move.x;
        //     std::cout << "HERE\n";
        //     exit(0);
        // }
        return best_move;
    }
    score minimax(Game::BoardState board, unsigned int depth, bool playing_as_maximizer){
        // if(depth==0 || board.is_full() || board.exhaustive_check_for_winner(Game::SquareState::Circle) || board.exhaustive_check_for_winner(Game::SquareState::Cross)){
        //     // return heuristic(board,playing_as_maximizer);
        //     return heuristic(board);
        // }
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
               Game::BoardState child = board;
               child.grid[x][y] = maximizing_player;
               value = std::max(value, minimax(child, depth+1, !playing_as_maximizer));
           }
           return value;
        } else {
           // score value = INT_MAX;
           score value = 1000;
           for(auto move: board.empty_slots()){
               auto [x,y] = move;
               Game::BoardState child = board;
               child.grid[x][y] = inverse_player(maximizing_player);
               value = std::min(value, minimax(child, depth+1, !playing_as_maximizer));
           }
           return value;
        }
    }

    // score heuristic(BoardState &board, bool playing_as_maximizer){
    score heuristic(Game::BoardState &board){
        // score value{0};
        // if(playing_as_maximizer){
           std::bitset<BOARD_SIZE> serialized_board_for_maxim_player = board.serialize_board_state_for_player(maximizing_player);
           std::bitset<BOARD_SIZE> serialized_board_for_minim_player = board.serialize_board_state_for_player(inverse_player(maximizing_player));
           for(auto rule:Game::WinStates){
               if((rule.bits & serialized_board_for_maxim_player) == rule.bits){
                  return 10;
               }
               if((rule.bits & serialized_board_for_minim_player) == rule.bits){
                   return -10;
               }
           }
           return 0;
        // }else{
        //    std::bitset<BOARD_SIZE> serialized_board_for_minim_player = board.serialize_board_state_for_player(inverse_player(maximizing_player));
        //    for(auto rule:Game::WinStates){
        //        if((rule.bits & serialized_board_for_minim_player) == rule.bits){
        //           return -10;
        //        }
        //    }
        //    return 0;
        // }
    }
    // Move minimax(BoardState &board, int depth, bool playing_as_maximizer){
    //     //ISSUES:
    //     // 1. Both best/worst (i.e first/last) give me the same value
    //     // 2. the heuristic isn't good (should instead get points if beat opponent, no points if draw, negative points if opponent wins or similar scheme)
    //     // std::cout << "HEREEEEEE MA FRIEND : fullnesss::>> " << board.is_full();
    //     if (depth ==0 || board.is_full() || board.exhaustive_check_for_winner(Game::SquareState::Circle)) {
    //         return {10, {0,0}};
    //     }
    //     if (depth ==0 || board.is_full() || board.exhaustive_check_for_winner(Game::SquareState::Cross)) {
    //         return {0,{0,0}};
    //     }
    //     // if(depth==0 || board.is_full() || board.exhaustive_check_for_winner(Game::SquareState::Circle) || board.exhaustive_check_for_winner(Game::SquareState::Cross)){
    //     // // if(depth==0 || board.exhaustive_check_for_winner(Game::SquareState::Circle) || board.exhaustive_check_for_winner(Game::SquareState::Cross)){
    //     //     if(playing_as_maximizer){
    //     //         // return heuristic_score2(board, maximizing_player);
    //     //         return heuristic_score2(board, !playing_as_maximizer);
    //     //         // return (playing_as_maximizer)?heuristic_score2(board, playing_as_maximizer):heuristic_score2(board, !playing_as_maximizer);
    //     //     } else {
    //     //         return heuristic_score2(board, playing_as_maximizer);
    //     //     }
    //     // }

    //     if(playing_as_maximizer){
    //         std::vector<std::tuple<score,unsigned long,unsigned long>> possible_positions{0};
    //         score value = INT_MIN;
    //         for(auto empty_square: board.empty_slots()){
    //             auto [x,y] = empty_square;
    //             BoardState child_board = board; //need to make it deep copy semantics, currently only copies ref
    //             child_board.grid[x][y] = Game::SquareState::Circle;
    //             value = std::max(value,minimax(child_board, depth-1, false).value);
    //             possible_positions.push_back(std::make_tuple(value,x,y));
    //         }
    //         // Move m = {.value=value, .pos={.x=0, .y=0}};
    //         // Find best position (i.e the x&y pair that yields highest value)
    //         sort(possible_positions.begin(), possible_positions.end());
    //         int idx = possible_positions.size()-1;
    //         // int idx = 0;
    //         Game::Game::Position best = {std::get<1>(possible_positions[idx]), std::get<2>(possible_positions[idx])};
    //         std::cout << "Possible moves Circle:  "<< possible_positions.size() << "\n";
    //         std::cout << "LAST MOVE IN VECS OF MOVES \n";
    //         std::cout << std::get<0>(possible_positions[possible_positions.size()-1])
    //                   << ", " << std::get<1>(possible_positions[possible_positions.size()-1])
    //                   << ", " << std::get<2>(possible_positions[possible_positions.size()-1]) << "\n";


    //         std::cout << "FIRST MOVE IN VEC \n";
    //         std::cout << std::get<0>(possible_positions[0])
    //                   << ", " << std::get<1>(possible_positions[0])
    //                   << ", " << std::get<2>(possible_positions[0]) << "\n";

    //         // return {value, {.x=0, .y=0}};
    //         return {value, best};
    //     }else{
    //         std::vector<std::tuple<score,unsigned long,unsigned long>> possible_positions{0};
    //         score value = INT_MAX;
    //         for(auto empty_square: board.empty_slots()){
    //             auto [x,y] = empty_square;
    //             BoardState child_board = board; //need to make it deep copy semantics, currently only copies ref
    //             child_board.grid[x][y] = Game::SquareState::Cross;
    //             value = std::min(value,minimax(child_board, depth-1, true).value);
    //             possible_positions.push_back(std::make_tuple(value,x,y));
    //         }
    //         sort(possible_positions.begin(), possible_positions.end());
    //         // int idx = possible_positions.size()-1;
    //         int idx = 0;
    //         Game::Game::Position best = {std::get<1>(possible_positions[idx]), std::get<2>(possible_positions[idx])};

    //         std::cout << "Possible moves Cross:  "<< possible_positions.size() << "\n";
    //         std::cout << "LAST MOVE IN VECS OF MOVES \n";
    //         std::cout << std::get<0>(possible_positions[possible_positions.size()-1])
    //                   << ", " << std::get<1>(possible_positions[possible_positions.size()-1])
    //                   << ", " << std::get<2>(possible_positions[possible_positions.size()-1]) << "\n";


    //         std::cout << "FIRST MOVE IN VEC \n";
    //         std::cout << std::get<0>(possible_positions[0])
    //                   << ", " << std::get<1>(possible_positions[0])
    //                   << ", " << std::get<2>(possible_positions[0]) << "\n";
    //         return {value, best};
    //     }
    // }


    // Move heuristic_score2(BoardState &board, bool playing_as_maximizer){
    //     // auto tally_rules_hit{0UL};
    //     score tally_rules_hit{};
    //     std::bitset<9> serialized_board_for_maximizing_player{};
    //     if(playing_as_maximizer){
    //     serialized_board_for_maximizing_player = board.serialize_board_state_for_player(Game::SquareState::Circle);
    //     } else{
    //     serialized_board_for_maximizing_player = board.serialize_board_state_for_player(Game::SquareState::Cross);
    //     }

    //     for(auto rule: Game::WinStates){
    //         std::bitset<BOARD_SIZE> bit_overlap = (serialized_board_for_maximizing_player & rule.bits);
    //         // std::cout << "COUNT OF BITS: " << bit_overlap.count() << " \n";
    //         tally_rules_hit = (tally_rules_hit>bit_overlap.count())? tally_rules_hit:bit_overlap.count();  //std::max(tally_rules_hit,bit_overlap.count());
    //         // if(bit_overlap.all()){
    //         // if((bit_overlap&rule.bits) == rule.bits){
    //         // if(bit_overlap.any()){
    //             // tally_rules_hit++;
    //             // tally_mplayer_pieces += bit_overlap.count();
    //         // }
    //     }
    //     // Move heuristic_move{.value = tally_mplayer_pieces + tally_rules_hit,

    //     // std::cout << "TALLY: " << tally_rules_hit << " \n";
    //     Move heuristic_move = {.value = tally_rules_hit, .pos = {.x = 2, .y = 2}};
    //     return heuristic_move;
    // }

    // Move heuristic_score(BoardState &board, Game::SquareState maximizing_player){
    //     // For all rules (Game::WinStates) tally how many our maximizing_player overlaps
    //     // also tally how many of maximizing_player pieces are in each rule.
    //     // return the product of these two numbers.
    //     // Game::Game::Position heuristic_position{};
    //     // auto tally_rules_hit{0UL};
    //     // auto tally_mplayer_pieces{0UL};
    //     score tally_rules_hit{};
    //     score tally_mplayer_pieces{};
    //     for(auto rule: Game::WinStates){
    //         auto serialized_board_for_maximizing_player = board.serialize_board_state_for_player(maximizing_player);
    //         auto bit_overlap = (serialized_board_for_maximizing_player & rule.bits);
    //         if(bit_overlap.any()){ //overlaps with rule (i.e at least one piece of MPlayer is hitting this rule)
    //             tally_rules_hit++;
    //             // count how many of those bits were hit (i.e how many pieces MPlayer has in that rule)
    //             // because having two pieces in a line already is much better than only having one.
    //             tally_mplayer_pieces += bit_overlap.count();
    //         }
    //     }

    //     // For whatever the matched/hit rule with highest value (tallies), I want an available
    //     // position in that line.
    //     // HANG ON A SECOND, we don't actually care about the move for a heuristic function
    //     // since the board is already FULL, we just want the value.

    //     #ifdef ENABLE_DEBUG
    //     std::cout << "Mplayer pieces: " << tally_mplayer_pieces << "\n";
    //     std::cout << "Rules hit: " << tally_rules_hit << "\n";
    //     std::cout << "-------------------------------------- \n";
    //     #endif

    //     Move heuristic_move{.value = tally_mplayer_pieces * tally_rules_hit,
    //     // Move heuristic_move{.value = tally_mplayer_pieces ,
    //     // Move heuristic_move{.value = tally_rules_hit,
    //         .pos = {.x = 2, .y = 2}
    //     };
    //     return heuristic_move;
    // }
};

// auto main(int argc, const char** argv) -> int {
//     //Setting up the testing environment
//     doctest::Context ctx;
//     ctx.setOption("abort-after", 2);  // stop testing after 2 failed test
//     ctx.applyCommandLine(argc, argv); // pass cli args to test framework
//     ctx.setOption("no-breaks", true); // don't break when jumping into the debugger
//     int test_results = ctx.run();     // run test cases unless passed "--no-run" in which case only the game code runs
//     if(ctx.shouldExit())              // framework relies on this stopping mechanism (i.e when we run only the tests)
//         return test_results;

//     int successful_game_flag = EXIT_SUCCESS;
//     BoardState board{};
//     // RandomMover ai{Game::SquareState::Circle};
//     MiniMax ai{Game::SquareState::Circle};
//     // std::cout << ai.next_move(BoardState &board)
//     // for(int i = 0; i < X_DIM; i++){
//     //     for(int j=0; j < Y_DIM; j++){
//     //         board.grid[i][j] = Game::SquareState::Circle;
//     //     }
//     // }

//     // std::cout << "HEREEEEEE MA FRIEND : fullnesss::>> " << board.is_full();
//     // TEST1
//     // board.grid[0][0] = Game::SquareState::Circle;
//     // board.grid[0][1] = Game::SquareState::Empty;
//     // board.grid[0][2] = Game::SquareState::Cross;
//     // board.grid[1][0] = Game::SquareState::Empty;
//     // board.grid[1][1] = Game::SquareState::Circle;
//     // board.grid[1][2] = Game::SquareState::Cross;
//     // board.grid[2][0] = Game::SquareState::Empty;
//     // board.grid[2][1] = Game::SquareState::Empty;
//     // board.grid[2][2] = Game::SquareState::Circle;

//     // TEST2
//     // board.grid[0][0] = Game::SquareState::Cross;
//     // board.grid[0][1] = Game::SquareState::Empty;
//     // board.grid[0][2] = Game::SquareState::Circle;
//     // board.grid[1][0] = Game::SquareState::Empty;
//     // board.grid[1][1] = Game::SquareState::Cross;
//     // board.grid[1][2] = Game::SquareState::Circle;
//     // board.grid[2][0] = Game::SquareState::Empty;
//     // board.grid[2][1] = Game::SquareState::Empty;
//     // board.grid[2][2] = Game::SquareState::Cross;


//     // TEST3
//     // board.grid[0][0] = Game::SquareState::Cross;
//     // board.grid[0][1] = Game::SquareState::Empty;
//     // board.grid[0][2] = Game::SquareState::Circle;
//     // board.grid[1][0] = Game::SquareState::Cross;
//     // board.grid[1][1] = Game::SquareState::Circle;
//     // board.grid[1][2] = Game::SquareState::Circle;
//     // board.grid[2][0] = Game::SquareState::Empty;
//     // board.grid[2][1] = Game::SquareState::Empty;
//     // board.grid[2][2] = Game::SquareState::Cross;


//     // board.grid[0][0] = Game::SquareState::Empty;
//     // board.grid[0][1] = Game::SquareState::Empty;
//     // board.grid[0][2] = Game::SquareState::Empty;
//     // board.grid[1][0] = Game::SquareState::Empty;
//     // board.grid[1][1] = Game::SquareState::Empty;
//     // board.grid[1][2] = Game::SquareState::Empty;
//     // board.grid[2][0] = Game::SquareState::Cross;
//     // board.grid[2][1] = Game::SquareState::Empty;
//     // board.grid[2][2] = Game::SquareState::Empty;
//     // board.print_board_state();
//     // std::cout << ai.heuristic(board) << "\n";
//     // exit(0);
//     // exit(0);
//     // std::cout << "HEREEEEEE MA FRIEND : fullnesss::>> " << board.is_full();
//     // board.print_board_state();

//     // score val = ai.heuristic_score(board, Game::SquareState::Circle);
//     // std::cout << "Value: " << val << "\n";
//     // std::cout << "Fullness: " << board.is_full() << "\n";
//     // exit(0);
//     bool did_circle_win = board.exhaustive_check_for_winner(Game::SquareState::Circle);
//     bool did_cross_win = board.exhaustive_check_for_winner(Game::SquareState::Cross);
//     bool draw_condition = board.is_draw();

//     while(!did_circle_win && !did_cross_win && !draw_condition){
//         board.print_board_state();
//         Game::Result res = board.get_user_input();
//         board.grid[res.position.x][res.position.y] = res.piece;
//         did_cross_win = board.exhaustive_check_for_winner(Game::SquareState::Cross);

//         if(!board.is_draw() && !did_cross_win){
//         Game::Game::Position ai_pos = ai.next_move(board);
//         board.grid[ai_pos.x][ai_pos.y] = ai.maximizing_player;//Game::SquareState::Circle;
//         }
//         // std::cout << "HEREEEE";
//         // Game::Game::Position ai_pos = ai.next_move(board);
//         // board.grid[ai_pos.x][ai_pos.y] = Game::SquareState::Circle;
//         did_circle_win = board.exhaustive_check_for_winner(Game::SquareState::Circle);
//         draw_condition = board.is_draw();
//     }

//     board.print_board_state();
//     did_circle_win? std::cout << "Circle is the winner!" << std::endl : std::cout << "Cross is the winner!" << std::endl;
//     return test_results + successful_game_flag;
// }
