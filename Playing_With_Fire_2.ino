// TODO: When a player wins, clear the screen using the classic clearing mechanism and announce the winner.
// TODO: Have a game startup sequence to introduce the game
// TODO: Have a game startup menu to select whether you are playing single player or double player.
// TODO: Give players "health" by dimming the player LED each time they get hit.
// Testing - dylan
#include "math.h"

#define N 8   // The dimensions of the LED array are N x N.
#define T 50  // How long (in µs) each row of the LED array is lit every cycle.
#define REFRESH_PERIOD 10 // How often the scene should be refreshed (in µs).
#define BOMB_LIFE 4000  // How many frames a bomb with stick around.
#define BOMB_INTERVAL 100  // How many frames a bomb will stay on or off.
#define BOMB_BLAST_LIFE 500 // How many frames a bomb's plast animation will play.
#define ACTION_INTERVAL 250
#define NULL_REGION_RADIUS 256

#define VRX A5
#define VRY A4
#define SW 1

const byte ROWS[N] = {13, 12, 11, 10, 9, 8, 7, 6};
const byte COLS[N] = {A3, A2, A1, A0, 5, 4, 3, 2};

static byte frame_buffer[N][N];

// The position of player 1.
static byte r1 = 7, c1 = 0;

// The position of player 2.
static byte r2 = 3, c2 = 3;

// Variables describing player's bomb.
bool exists_player_bomb = false;
long player_bomb_explode_time;
bool exists_player_bomb_explosion = false;
long player_bomb_explosion_time;
byte br1, bc1;

// Variables describing computer's bomb.
bool exists_computer_bomb = false;
long computer_bomb_explode_time;
byte br2, bc2;

void setup() {
  // Configure all row and column pins as output pins.
  for (byte i = 0; i < N; i++) {
    pinMode(ROWS[i], OUTPUT);
    pinMode(COLS[i], OUTPUT);
  }
  pinMode(SW, INPUT_PULLUP);
  pinMode(VRX, INPUT);
  pinMode(VRY, INPUT);
}

void display(byte pattern[N][N]) {
  for (byte r = 0; r < N; r++) {
    // Light up row "r" for T µs.
    for (byte c = 0; c < N; c++)
      digitalWrite(COLS[c], pattern[r][c] ? LOW : HIGH);
    digitalWrite(ROWS[r], LOW);
    delayMicroseconds(T);
    digitalWrite(ROWS[r], HIGH);
  }
}

void composite_level() {
  for (byte r = 0; r < N; r += 2) {
    for (byte c = 0; c < N; c += 2)
      frame_buffer[r][c] = 1;
  }
}

void composite_players() {
  frame_buffer[r1][c1] = 1;
  frame_buffer[r2][c2] = 1;
}

void composite_bombs() {
  if (exists_player_bomb) {
    frame_buffer[br1][bc1] = 1;
  }
}

void clear_frame_buffer() {
  for (byte r = 0; r < N; r++) {
    for (byte c = 0; c < N; c++)
      frame_buffer[r][c] = 0;
  }
}

bool player_position_valid(const byte player, const byte r, const byte c) {
  // Check for collision with walls.
  bool collides_with_walls = r % 2 == 0 && c % 2 == 0;
  // Check for collision with the other player.
  bool collides_with_other_player = false;
  if (player == 1) {
    if (r == r2 && c == c2) collides_with_other_player = true;
  } else {
    if (r == r1 && c == c1) collides_with_other_player = true;
  }
  // Check for collision with bombs.
  bool collides_with_bomb_1 = exists_player_bomb ? (r == br1 && c == bc1) : false;
  //  bool collides_with_bomb_2 = ttl2 ? (r == br2 && c == bc2) : false;
  bool collides_with_bombs = collides_with_bomb_1/* || collides_with_bomb_2 */;
  // Check for out of bounds.
  bool out_of_bounds = r > 7 || c > 7;
  // Collate tests.
  return !collides_with_walls && !collides_with_other_player && !out_of_bounds
         && !collides_with_bombs;
}

enum Direction {
  Left,
  Right,
  Up,
  Down,
  NoMove,
  Move
};

// NOTE: does not return!
void print_L() {
  clear_frame_buffer();
  for (int r = 1; r <= 5; r++) {
    frame_buffer[r][2] = 1;
  }
  for (int c = 2; c <= 5; c++) {
    frame_buffer[5][c] = 1;
  }
  while (true) {
    display(frame_buffer);
  }
}

// NOTE: does not return!
void print_W() {
  clear_frame_buffer();
  for (int r = 1; r <= 5; r++) {
    frame_buffer[r][1] = 1;
    frame_buffer[r][6] = 1;
  }
  for (int c = 1; c <= 6; c++) {
    frame_buffer[5][c] = 1;
  }
  for (int r = 3; r <= 4; r++) {
    frame_buffer[r][3] = 1;
    frame_buffer[r][4] = 1;
  }
  while (true) {
    display(frame_buffer);
  }
}

// NOTE: does not return!
void print_D() {
  clear_frame_buffer();
  for (int r = 1; r <= 5; r++) {
    frame_buffer[r][2] = 1;
  }
  frame_buffer[1][3] = 1;
  frame_buffer[2][4] = 1;
  frame_buffer[3][5] = 1;
  frame_buffer[4][4] = 1;
  frame_buffer[5][3] = 1;
  while (true) {
    display(frame_buffer);
  }
}

void loop() {
  // Sample the timer.
  long current_time = millis();
  static long next_action_time = millis();
  static long next_computer_action_time = millis();
  if (current_time - next_computer_action_time > 0) {
    // Randomly choose an action (up, left, right, down, drop bomb).
    int n = random(5);
    if (n == 0 and player_position_valid(2, r2 - 1, c2)) {
      // Up
      r2--;
    } else if (n == 1 and player_position_valid(2, r2 + 1, c2)) {
      // Down
      r2++;
    } else if (n == 2 and player_position_valid(2, r2, c2 - 1)) {
      // Left
      c2--;
    } else if (n == 3 and player_position_valid(2, r2, c2 + 1)) {
      // Right
      c2++;
    } else if (n == 4) {
      // Drop Bomb
    }
    next_computer_action_time = current_time + ACTION_INTERVAL;
  }
  if (current_time - next_action_time > 0) {
    // Process a user action (i.e. drop bomb or move).
    bool sw = digitalRead(SW);
    if (sw == 0) {
      // Player action: drop bomb
      if (not exists_player_bomb) {
        exists_player_bomb = true;
        player_bomb_explode_time = current_time + 5000;
        br1 = r1;
        bc1 = c1;
      }
      // Update the next action time.
      next_action_time = current_time + ACTION_INTERVAL;
    } else {
      // Player action: move
      int y = analogRead(VRY) - 512;
      int x = analogRead(VRX) - 512;
      // Determine whether or not we're in the null region.
      double distance = sqrt(square(y) + square(x));
      Direction d = Move;
      if (distance < NULL_REGION_RADIUS) d = NoMove;
      if (d == Move) {
        // Determine the movement direction.
        if (y > x and y < -x) {
          d = Down;
        } else if (y < x and y < -x) {
          d = Right;
        } else if (y < x and y > -x) {
          d = Up;
        } else {
          d = Left;
        }
        // Move the player.
        if (d == Left and player_position_valid(1, r1, c1 - 1)) {
          c1--;
        } else if (d == Right and player_position_valid(1, r1, c1 + 1)) {
          c1++;
        } else if (d == Up and player_position_valid(1, r1 + 1, c1)) {
          r1++;
        } else if (d == Down and player_position_valid(1, r1 - 1, c1)) {
          r1--;
        }
      }
      // Update the next action time.
      next_action_time = current_time + ACTION_INTERVAL;
    }
  }
  if (exists_player_bomb) {
    if (current_time - player_bomb_explode_time > 0) {
      exists_player_bomb = false;
      exists_player_bomb_explosion = true;
      player_bomb_explosion_time = current_time + 1000;
    }
  }
  clear_frame_buffer();
  if (exists_player_bomb_explosion) {
    if (current_time - player_bomb_explosion_time > 0) {
      exists_player_bomb_explosion = false;
    } else {
      // Check if anybody died.
      if (br1 % 2 == 1) {
        for (int i = 0; i < N; i++) {
          frame_buffer[br1][i] = 1;
        }
      }
      if (bc1 % 2 == 1) {
        for (int i = 0; i < N; i++) {
          frame_buffer[i][bc1] = 1;
        }
      }
      bool player_died = false;
      bool computer_died = false;
      if (br1 % 2 == 1) {
        for (int i = 0; i < N; i++) {
          frame_buffer[br1][i] = 1;
          if (br1 == r1 and i == c1) player_died = true;
          if (br1 == r2 and i == c2) computer_died = true;
        }
      }
      if (bc1 % 2 == 1) {
        for (int i = 0; i < N; i++) {
          if (i == r1 and bc1 == c1) player_died = true;
          if (i == r2 and bc1 == c2) computer_died = true;
        }
      }
      if (player_died and not computer_died) {
        // Print L
        print_L();
      } else if (not player_died and computer_died) {
        // Print W
        print_W();
      } else if (player_died and computer_died) {
        // Print D
        print_D();
      }
    }
  }
  composite_level();
  composite_players();
  composite_bombs();
  display(frame_buffer);
}
