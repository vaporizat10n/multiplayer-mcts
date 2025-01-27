# Monte-Carlo Tree Search for Turn-Based Multiplayer Games

## Usage

The class `mcts::Agent` requires a `GameState` class and an RNG (`mt19937_64` by default).

**Description of `GameState`:**

Nested Classes:
`GameState::Move`: Represents a move.

Functions:
| Prototype                                                                      | Description                                                                         |
| ------------------------------------------------------------------------------ | ----------------------------------------------------------------------------------- |
| `GameState GameState::make_move(GameState::Move move)`                         | Returns a new game state with `move` applied. The original state remains unchanged. |
| `std::vector<GameState::Move> GameState::possible_moves()`                     | Retrieves the valid moves for the current player in the game state.                 |
| `int GameState::current_player()`                                              | Retrieves the integer id of the player who is next to move.                         |
| `double GameState::get_result(int player)` (required for terminal states only) | Returns a score for the player in the range [0, 1].                                 |
