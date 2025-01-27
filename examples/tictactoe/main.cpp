#include "../../include/mcts/agent.hpp"
#include "tic_tac_toe.hpp"

#include <chrono>
#include <cstdio>

void print_board(const TicTacToeState &state) {
  const std::array<TicTacToeState::BitBoard, 2> &bitboard = state.get_info();
  std::string grid(9, ' ');
  for (int i = 0; i < 9; i++) {
    if (bitboard[0].test(i)) {
      grid[i] = 'X';
    } else if (bitboard[1].test(i)) {
      grid[i] = 'O';
    }
  }
  for (int i = 0; i < 3; i++) {
    if (i > 0) {
      printf("---|---|---\n");
    }
    for (int j = 0; j < 3; j++) {
      if (j > 0) {
        printf("|");
      }
      printf(" %c ", grid[i * 3 + j]);
    }
    printf("\n");
  }
  printf("\n");
}

void mcts_vs_human() {
  mcts::Agent<TicTacToeState> agent((TicTacToeState()), true,
                                    std::chrono::steady_clock::now().time_since_epoch().count());
  while (true) {
    int agent_move = agent.search_move(std::chrono::seconds(1));
    agent.apply_move(agent_move);
    if (agent.state().terminal()) {
      break;
    }
    print_board(agent.state());
    int human_move;
    scanf("%d", &human_move);
    agent.apply_move(human_move);
    if (agent.state().terminal()) {
      break;
    }
  }
  printf("Final Board:\n");
  print_board(agent.state());
}

void human_vs_mcts() {
  mcts::Agent<TicTacToeState> mcts((TicTacToeState()));
  while (true) {
    print_board(mcts.state());
    int human_move;
    scanf("%d", &human_move);
    mcts.apply_move(human_move);
    if (mcts.state().terminal()) {
      break;
    }
    int mcts_move = mcts.search_move(std::chrono::seconds(1));
    mcts.apply_move(mcts_move);
    if (mcts.state().terminal()) {
      break;
    }
  }
  printf("Final Board:\n");
  print_board(mcts.state());
}

int main() {
  mcts_vs_human();
  // human_vs_mcts();

  return 0;
}
