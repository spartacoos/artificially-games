#include <bitset>
#include <cstdlib>
#include <iostream>
#include <regex>
#define X_DIM 3
#define Y_DIM 3
#define BOARD_SIZE (X_DIM)*(Y_DIM)

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
};


struct AI {
    AI(SquareState piece_type):piece_type{piece_type}{}
    ~AI()= default;
    virtual Position next_move(BoardState &board)=0;
    SquareState piece_type; //cross or circle
};

struct RandomMover: AI {
        RandomMover(SquareState piece_type) : AI(piece_type){}
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
        return minimax(board);
    }
    Position minimax(BoardState &board){
        return board.get_position_for_bitset_bit(0);
    }
    score heuristic_score(BoardState &board, SquareState maximizing_player){
        // For all rules (WinStates) tally how many our maximizing_player overlaps
        // also tally how many of maximizing_player pieces are in each rule.
        // return the product of these two numbers.
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
        std::cout << "Mplayer pieces: " << tally_mplayer_pieces << "\n";
        std::cout << "Rules hit: " << tally_rules_hit << "\n";
        return tally_mplayer_pieces * tally_rules_hit;
    }
};

auto main() -> int {
    BoardState board{};
    // RandomMover ai{SquareState::Circle};
    MiniMax ai{SquareState::Circle};
    // std::cout << ai.next_move(BoardState &board)
    board.grid[0][0] = SquareState::Circle;
    board.grid[1][0] = SquareState::Circle;
    // board.grid[1][1] = SquareState::Circle;
    // board.grid[2][1] = SquareState::Circle;
    // board.grid[1][2] = SquareState::Cross;
    board.grid[1][1] = SquareState::Cross;
    board.print_board_state();
    score val = ai.heuristic_score(board, SquareState::Circle);
    std::cout << "Value: " << val << "\n";
    exit(0);
    bool did_circle_win = board.exhaustive_check_for_winner(SquareState::Circle);
    bool did_cross_win = board.exhaustive_check_for_winner(SquareState::Cross);

    while(!did_circle_win && !did_cross_win){
        board.print_board_state();
        Result res = board.get_user_input();
        board.grid[res.position.x][res.position.y] = res.piece;

        // std::cout << "HEREEEE";
        Position ai_pos = ai.next_move(board);
        board.grid[ai_pos.x][ai_pos.y] = SquareState::Circle;
        did_circle_win = board.exhaustive_check_for_winner(SquareState::Circle);
        did_cross_win = board.exhaustive_check_for_winner(SquareState::Cross);
    }

    board.print_board_state();
    did_circle_win? std::cout << "Circle is the winner!" << std::endl : std::cout << "Cross is the winner!" << std::endl;
    return 0;
}
