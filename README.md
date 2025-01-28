# Monte-Carlo Tree Search for Turn-Based Multiplayer Games

An implementation of the
[Monte Carlo tree search](https://en.wikipedia.org/wiki/Monte_Carlo_tree_search) algorithm in C++.
It provides an agent which can play turn-based multiplayer games.

A game state type needs to be defined in order to use this library.
See the source code documentation for more details.

## Details

This is a relatively simple, single-threaded implementation. It can play as multiple players in the
same game and reuses the tree at every turn.

## Usage

This is a header only library. To use it, include the file at `include/mcts/agent.hpp`.
Atleast C++20 is required.

For example usage, see `examples/tictactoe/`.

## References

1. Świechowski, M., Godlewski, K., Sawicki, B. *et al.* Monte Carlo Tree Search: a review of recent
modifications and applications. *Artif Intell Rev* **56**, 2497–2562 (2023).
https://arxiv.org/abs/2103.04931v4
