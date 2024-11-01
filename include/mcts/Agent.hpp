#pragma once

#include <algorithm>
#include <chrono>
#include <memory>
#include <numbers>
#include <random>
#include <vector>

template <typename GameState, typename RNG = std::mt19937_64>
class Agent {
 public:
  using Move = GameState::Move;

  Agent(GameState state = GameState(), bool _shuffle_moves = true, uint64_t seed = 0)
      : rng(seed),
        shuffle_moves(_shuffle_moves),
        root(make_node(nullptr, -1, std::move(state), shuffle_moves ? &rng : nullptr)) {}

  const GameState &state() const {
    return root->state;
  }

  void apply_move(Move move) {
    auto &state = root->state;
    auto &possible_moves = root->possible_moves;
    int move_index =
        std::ranges::distance(possible_moves.begin(), std::ranges::find(possible_moves, move));
    if (move_index < root->children.size()) {
      root = std::move(root->children[move_index]);
      root->parent = nullptr;
      root->player = -1;
      root->score = 0.0;
    } else {
      root = make_node(nullptr, -1, state.make_move(possible_moves[move_index]), root->shuffle_rng);
    }
  }

  Move compute_move(std::chrono::steady_clock::duration duration) {
    auto get_time = []() -> std::chrono::steady_clock::time_point {
      return std::chrono::steady_clock::now();
    };
    auto begin_time = std::chrono::steady_clock::now();
    while (get_time() - begin_time < duration) {
      execute_one_iteration();
    }
    int move_index = -1;
    int most_visits = 0;
    int child_count = root->children.size();
    for (int i = 0; i < child_count; i++) {
      int visits = root->children[i]->visits;
      if (visits > most_visits) {
        move_index = i;
        most_visits = visits;
      }
    }
    return root->possible_moves[move_index];
  }

 private:
  RNG rng;
  bool shuffle_moves;

  struct Node {
    Node(Node *_parent, int _player, GameState _state, RNG *_shuffle_rng)
        : parent(_parent),
          visits(0),
          player(_player),
          score(0.0),
          state(std::move(_state)),
          shuffle_rng(_shuffle_rng),
          possible_moves(state.get_possible_moves()) {
      if (shuffle_rng) {
        std::ranges::shuffle(possible_moves, *shuffle_rng);
      }
      next_untried_move = possible_moves.begin();
    }

    Node *parent;
    int visits;
    int player;
    double score;

    RNG *shuffle_rng;

    GameState state;
    std::vector<Move> possible_moves;

    std::vector<std::unique_ptr<Node>> children;
    std::vector<Move>::iterator next_untried_move;

    bool is_terminal() const {
      return possible_moves.empty();
    }

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
      Move move = *next_untried_move++;
      return children
          .emplace_back(make_node(this, state.current_player(), state.make_move(move), shuffle_rng))
          .get();
    }

    void backpropagate(const GameState &terminal_state) {
      for (Node *node = this; node; node = node->parent) {
        ++node->visits;
        if (node->player != -1) {
          node->score += terminal_state.get_result(node->player);
        }
      }
    }
  };

  template <typename... Args>
  static std::unique_ptr<Node> make_node(Args &&...args) {
    return std::make_unique<Node>(std::forward<Args>(args)...);
  }

  std::unique_ptr<Node> root;

  GameState rollout(GameState state) {
    while (!state.terminal()) {
      std::vector moves = state.get_possible_moves();
      int choice = std::uniform_int_distribution<>(0, moves.size() - 1)(rng);
      state = state.make_move(moves[choice]);
    }
    return state;
  }

  void execute_one_iteration() {
    auto select_node = [this]() -> Node * {
      Node *node = root.get();
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
};
