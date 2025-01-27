#pragma once

#include <algorithm>
#include <chrono>
#include <memory>
#include <numbers>
#include <random>
#include <vector>

namespace mcts {

template <typename GameState, typename RNG = std::mt19937_64>
class Agent {
 public:
  using Move = GameState::Move;

  // Initializes an agent with the `state`.
  // Set `shuffle_moves` to true if the order of moves returned by `GameState::get_possible_moves()`
  // should be randomized. This only matters when choosing between multiple moves with the same
  // evaluation.
  Agent(GameState state = GameState(), bool shuffle_moves = true, uint64_t seed = 0)
      : rng_(seed),
        shuffle_moves_(shuffle_moves),
        root_(make_node(nullptr, -1, std::move(state), shuffle_moves ? &rng_ : nullptr)) {}

  const GameState &state() const {
    return root_->state;
  }

  // Plays `move` as the current player.
  void apply_move(Move move) {
    auto &state = root_->state;
    auto &possible_moves = root_->possible_moves;
    int move_index =
        std::ranges::distance(possible_moves.begin(), std::ranges::find(possible_moves, move));
    if (move_index < root_->children.size()) {
      root_ = std::move(root_->children[move_index]);
      // Reset statistics for the root_.
      root_->parent = nullptr;
      root_->player = -1;
      root_->score = 0.0;
    } else {
      root_ =
          make_node(nullptr, -1, state.make_move(possible_moves[move_index]), root_->shuffle_rng);
    }
  }

  // Searches for a move in `duration` using the MCTS algorithm. To apply this move, use the
  // `apply_move()` function.
  // Exhibits undefined behaviour if the game has already ended.
  Move search_move(std::chrono::steady_clock::duration duration) {
    auto get_time = []() -> std::chrono::steady_clock::time_point {
      return std::chrono::steady_clock::now();
    };
    auto begin_time = std::chrono::steady_clock::now();
    while (get_time() - begin_time < duration) {
      execute_one_iteration();
    }
    int move_index = -1;
    int most_visits = 0;
    int child_count = root_->children.size();
    for (int i = 0; i < child_count; i++) {
      int visits = root_->children[i]->visits;
      if (visits > most_visits) {
        move_index = i;
        most_visits = visits;
      }
    }
    return root_->possible_moves[move_index];
  }

 private:
  struct Node;

  template <typename... Args>
  static std::unique_ptr<Node> make_node(Args &&...args) {
    return std::make_unique<Node>(std::forward<Args>(args)...);
  }

  RNG rng_;
  bool shuffle_moves_;

  std::unique_ptr<Node> root_;

  void execute_one_iteration() {
    auto select_node = [this]() -> Node * {
      Node *node = root_.get();
      while (!node->is_terminal() && node->is_fully_expanded()) {
        node = node->select_child();
      }
      return node;
    };
    Node *node = select_node();
    if (!node->is_terminal()) {
      node = node->expand();
    }
    GameState terminal_state = rollout(node->state);
    node->backpropagate(terminal_state);
  }

  GameState rollout(GameState state) {
    while (!state.terminal()) {
      std::vector moves = state.possible_moves();
      int choice = std::uniform_int_distribution<>(0, moves.size() - 1)(rng_);
      state = state.make_move(moves[choice]);
    }
    return state;
  }
};

// Statistics for each move in the search tree.
template <typename GameState, typename RNG>
struct Agent<GameState, RNG>::Node {
  Node(Node *_parent, int _player, GameState _state, RNG *_shuffle_rng)
      : parent(_parent),
        visits(0),
        player(_player),
        score(0.0),
        state(std::move(_state)),
        shuffle_rng(_shuffle_rng),
        possible_moves(state.possible_moves()) {
    if (shuffle_rng) {
      std::ranges::shuffle(possible_moves, *shuffle_rng);
    }
    next_untried_move = possible_moves.begin();
  }

  Node *parent;
  int visits;

  // The player who made the last move.
  int player;
  double score;

  // Points to Agent::RNG if moves returned by `state.possible_moves()` should be
  // shuffled. Holds a `nullptr` otherwise.
  RNG *shuffle_rng;

  GameState state;
  std::vector<Move> possible_moves;

  std::vector<std::unique_ptr<Node>> children;
  std::vector<Move>::iterator next_untried_move;

  bool is_terminal() const {
    return possible_moves.empty();
  }

  // https://www.chessprogramming.org/UCT#Selection
  Node *select_child(double c = std::numbers::sqrt2) const {
    Node *best_child = nullptr;
    double best_uct = std::numeric_limits<double>::min();
    for (const std::unique_ptr<Node> &child : children) {
      double uct = child->score / child->visits + c * sqrt(log(visits) / child->visits);
      if (uct > best_uct) {
        best_child = child.get();
        best_uct = uct;
      }
    }
    return best_child;
  }

  bool is_fully_expanded() const {
    return next_untried_move == possible_moves.end();
  }

  Node *expand() {
    // assert(!is_fully_expanded());
    Move move = *next_untried_move++;
    return children
        .emplace_back(make_node(this, state.current_player(), state.make_move(move), shuffle_rng))
        .get();
  }

  void backpropagate(const GameState &terminal_state) {
    for (Node *node = this; node; node = node->parent) {
      ++node->visits;
      // 'score' of the root state isn't useful anyway
      if (node->player != -1) {
        node->score += terminal_state.get_result(node->player);
      }
    }
  }
};

}  // namespace mcts
