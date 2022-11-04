#include <iostream>
#define X_DIM 3
#define Y_DIM 3

std::bitset<9> WinStates[8] = {  //there are  8 different conditions that the board can be in where somebody has won
  std::bitset<9>(std::string("100010001")), // diagonal #1
  std::bitset<9>(std::string("100010001")), //
  std::bitset<9>(std::string("100010001")),
  std::bitset<9>(std::string("100010001")),
  std::bitset<9>(std::string("100010001")),
  std::bitset<9>(std::string("100010001")),
  std::bitset<9>(std::string("100010001")),
  std::bitset<9>(std::string("100010001")),

};

enum class SquareState {
  Empty,
  Circle,
  Cross
};

struct BoardState {
    bool winner = false;
    SquareState grid[3][3];

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
        // this is not smart but it will do for now as long as we keep the grid small i.e 3x3 but if we want the AI to play on, say, 100x100 grids, this will be utter O(n^2) shite
        // SquareState circle = SquareState::Circle;
        // SquareState cross = SquareState::Cross;
        //horizontal =
        //grid[0][0] grid[0][1] grid[0][2]  ==> boolean
        //grid[1][0] grid[1][1] grid[1][2]  ==> boolean
        //grid[2][0] grid[2][1] grid[2][2]  ==> boolean
        //
        //I could encode it in binary as such
        // 1              1          1 //target bit pattern for rule (i.e horizontal)
        // 1              1          1 //constructed binary pattern after scanning line
        // ----------------------------
        // How to construct the binary patterns for h/v lines?
        // Could iterate and then bit shift? set a specific location in the binary number as either 1 or 0, how to do with binary operators? XOR with a shifted byte
        // holder = 0 0 0 0 0 0 0 0
        //
        // REALIZATION: the entireeee exhaustive rule check can be done via a single binary operation where we check the constructed binary or 1s and 0s each representing the state of (Circle | NotCircle)
        // and then XOR-ing that with another binary number that contains the XOR sum of the rules??
            //vertical check
        std::bitset<9> serialized = serialize_board_state_for_player(player);
        bool winner =
            (serialized == WinStates[0])? true:
            (serialized == WinStates[1])? true:
            (serialized == WinStates[2])? true:
            (serialized == WinStates[3])? true:
            (serialized == WinStates[4])? true:
            (serialized == WinStates[5])? true:
            (serialized == WinStates[6])? true:
            (serialized == WinStates[7])? true:
            (serialized == WinStates[8])? true: false;
        std::cout << serialized << std::endl;
        return winner;
        }

};

auto main() -> int {
    BoardState board{};
    board.grid[0][0] = SquareState::Circle;
    board.grid[1][1] = SquareState::Circle;
    board.grid[2][2] = SquareState::Circle;
    board.grid[1][2] = SquareState::Cross;
    board.grid[1][0] = SquareState::Cross;
    board.print_board_state();
    bool did_circle_win = board.exhaustive_check_for_winner(SquareState::Circle);
    did_circle_win? std::cout << "Yes, circle is the winner" << std::endl : std::cout << "No, circle hasn't won" << std::endl;
        // sort out emacs: autocomplete not working ... is it due to how I initialized the project?
    return 0;
}
