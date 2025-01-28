#include "../../include/mcts/agent.hpp"
#include "tic_tac_toe.hpp"

#include <chrono>
#include <cstdio>
#include <string>

void print_board(std::string grid) {
  assert(grid.length() == 9);
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

void print_state(const TicTacToeState& state) {
  const std::array<TicTacToeState::BitBoard, 2>& bitboard = state.get_info();
  std::string grid(9, ' ');
  for (int i = 0; i < 9; i++) {
    if (bitboard[0].test(i)) {
      grid[i] = 'X';
    } else if (bitboard[1].test(i)) {
      grid[i] = 'O';
    }
  }
  print_board(grid);
}

void mcts_vs_human() {
  printf("Enter the position to play your move:\n");
  print_board("123456789");

  uint64_t seed = std::chrono::steady_clock::now().time_since_epoch().count();
  mcts::Agent<TicTacToeState> agent((TicTacToeState()), true, seed);
  while (true) {
    int agent_move = agent.search_move(std::chrono::seconds(1));
    agent.apply_move(agent_move);
    if (agent.state().terminal()) {
      break;
    }
    TicTacToeState state = agent.state();
    print_state(state);
    int human_move;
    printf("Move: ");
    scanf("%d", &human_move);
    --human_move;
    while (!state.is_valid_move(human_move)) {
      printf("Invalid Move.\n\n");
      printf("Move: ");
      scanf("%d", &human_move);
      --human_move;
    }
    agent.apply_move(human_move);
    if (agent.state().terminal()) {
      break;
    }
  }
  printf("Final Board:\n");
  print_state(agent.state());
}

void human_vs_mcts() {
  printf("Enter the position to play your move:\n");
  print_board("123456789");

  uint64_t seed = std::chrono::steady_clock::now().time_since_epoch().count();
  mcts::Agent<TicTacToeState> agent((TicTacToeState()), true, seed);
  while (true) {
    TicTacToeState state = agent.state();
    print_state(state);
    int human_move;
    printf("Move: ");
    scanf("%d", &human_move);
    --human_move;
    while (!state.is_valid_move(human_move)) {
      printf("Invalid Move.\n\n");
      printf("Move: ");
      scanf("%d", &human_move);
      --human_move;
    }
    agent.apply_move(human_move);
    if (agent.state().terminal()) {
      break;
    }
    int agent_move = agent.search_move(std::chrono::seconds(1));
    agent.apply_move(agent_move);
    if (agent.state().terminal()) {
      break;
    }
  }
  printf("Final Board:\n");
  print_state(agent.state());
}

void mcts_vs_mcts() {
  uint64_t seed = std::chrono::steady_clock::now().time_since_epoch().count();
  mcts::Agent<TicTacToeState> agent((TicTacToeState()), true, seed);
  while (true) {
    {
      int agent_move = agent.search_move(std::chrono::seconds(1));
      agent.apply_move(agent_move);
      if (agent.state().terminal()) {
        break;
      }
      print_state(agent.state());
    }
    {
      int agent_move = agent.search_move(std::chrono::seconds(1));
      agent.apply_move(agent_move);
      if (agent.state().terminal()) {
        break;
      }
      print_state(agent.state());
    }
  }
  printf("Final Board:\n");
  print_state(agent.state());
}

int main() {
  // mcts_vs_human();
  // human_vs_mcts();
  mcts_vs_mcts();

  return 0;
}
