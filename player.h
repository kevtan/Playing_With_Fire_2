#ifndef PLAYER_H
#define PLAYER_H

#include "position.h"

struct Player {
    Position position;
};

//!
//! @brief Identifies the players in the game.
//!
enum class PlayerType {
  HUMAN,
  COMPUTER,
};

#endif