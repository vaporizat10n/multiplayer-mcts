#include <array>
#include <bitset>
#include <cassert>
#include <vector>

class TicTacToeState {
 public:
  using Move = int;
  using BitBoard = std::bitset<9>;

  TicTacToeState()
      : terminal_(false), winner_(-1), current_player_(0) {
    occupied.fill(0);
  }

  bool terminal() const {
    return terminal_;
  }

  bool is_valid_move(Move move) const {
    return !occupied[0].test(move) && !occupied[1].test(move);
  }

  std::vector<Move> possible_moves() const {
    if (terminal_) {
      return std::vector<Move>();
    }
    std::vector<Move> moves;
    std::bitset squares = get_empty_squares();
    for (int square = squares._Find_first(); square != 9; square = squares._Find_next(square)) {
      moves.push_back(square);
    }
    return moves;
  }

  void apply_move(Move move) {
    assert(!terminal());
    assert(is_valid_move(move));
    occupied[current_player_].set(move);
    check_terminal(current_player_);
    current_player_ ^= 1;
  }

  TicTacToeState make_move(Move move) const {
    TicTacToeState state(*this);
    state.apply_move(move);
    return state;
  }

  int current_player() const {
    return current_player_;
  }

  int winner() const {
    assert(terminal());
    return winner_;
  }

  double get_result(int player) const {
    assert(terminal());
    if (winner_ == -1) {
      return 0.5;
    }
    return player == winner_ ? 1.0 : 0.0;
  }

  std::array<BitBoard, 2> get_info() const {
    return occupied;
  }

 private:
  static constexpr std::array winning{
      BitBoard(0b000'000'111), BitBoard(0b000'111'000), BitBoard(0b111'000'000),
      BitBoard(0b001'001'001), BitBoard(0b010'010'010), BitBoard(0b100'100'100),
      BitBoard(0b100'010'001), BitBoard(0b001'010'100),
  };

  std::array<BitBoard, 2> occupied;
  bool terminal_;
  int winner_;
  int current_player_;

  std::bitset<9> get_empty_squares() const {
    return ~(occupied[0] | occupied[1]);
  }

  bool check_terminal(int player = -1) {
    assert(!terminal());
    auto check_win = [this](int player) -> bool {
      const BitBoard &board = occupied[player];
      for (const BitBoard &config : winning) {
        if ((board & config) == config) {
          winner_ = player;
          return true;
        }
      }
      return false;
    };
    auto check_draw = [this]() -> bool {
      return (occupied[0] | occupied[1]).all();
    };
    if (player == -1) {
      terminal_ = check_win(0) || check_win(1) || check_draw();
    } else {
      terminal_ = check_win(player) || check_draw();
    }
    return terminal_;
  }
};
