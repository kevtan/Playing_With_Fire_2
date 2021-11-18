// TODO: When a player wins, clear the screen using the classic clearing mechanism and announce the winner.
// TODO: Have a game startup sequence to introduce the game
// TODO: Have a game startup menu to select whether you are playing single player or double player.
// TODO: Give players "health" by dimming the player LED each time they get hit.
// Testing - dylan
#include "math.h"
#include "actions.h"
#include "players.h"
#include "directions.h"

#define N 8                 // The dimensions of the LED array are N x N.
#define T 50                // How long (in µs) each row of the LED array is lit every cycle.
#define REFRESH_PERIOD 10   // How often the scene should be refreshed (in µs).
#define BOMB_LIFE 4000      // How many frames a bomb with stick around.
#define BOMB_INTERVAL 100   // How many frames a bomb will stay on or off.
#define BOMB_BLAST_LIFE 500 // How many frames a bomb's plast animation will play.
#define ACTION_INTERVAL 250
#define NULL_REGION_RADIUS 256

#define VRX A5
#define VRY A4
#define SW 1

const byte ROWS[N] = {13, 12, 11, 10, 9, 8, 7, 6};
const byte COLS[N] = {A3, A2, A1, A0, 5, 4, 3, 2};

static byte frame_buffer[N][N];

byte human_row, human_col;
byte computer_row, computer_col;

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

void setup()
{
  // Configure all row and column pins as output pins.
  for (byte i = 0; i < N; i++)
  {
    pinMode(ROWS[i], OUTPUT);
    pinMode(COLS[i], OUTPUT);
  }
  pinMode(SW, INPUT_PULLUP);
  pinMode(VRX, INPUT);
  pinMode(VRY, INPUT);
}

void display(byte pattern[N][N])
{
  for (byte r = 0; r < N; r++)
  {
    // Light up row "r" for T µs.
    for (byte c = 0; c < N; c++)
      digitalWrite(COLS[c], pattern[r][c] ? LOW : HIGH);
    digitalWrite(ROWS[r], LOW);
    delayMicroseconds(T);
    digitalWrite(ROWS[r], HIGH);
  }
}

void composite_level()
{
  for (byte r = 0; r < N; r += 2)
  {
    for (byte c = 0; c < N; c += 2)
      frame_buffer[r][c] = 1;
  }
}

void composite_players()
{
  frame_buffer[human_row][human_col] = 1;
  frame_buffer[computer_row][computer_col] = 1;
}

void composite_bombs()
{
  if (exists_player_bomb)
  {
    frame_buffer[br1][bc1] = 1;
  }
}

void clear_frame_buffer()
{
  for (byte r = 0; r < N; r++)
  {
    for (byte c = 0; c < N; c++)
      frame_buffer[r][c] = 0;
  }
}

bool position_legal(Player player, const byte row, const byte col)
{
  // The position is illegal if it's out of bounds.
  if (row >= N or col >= N)
    return false;
  // The position is illegal if there's a wall there.
  if (row % 2 == 0 and col % 2 == 0)
    return false;
  // The position is illegal if the other player is there.
  if (player == Player::HUMAN and row == computer_row and col == computer_col)
    return false;
  if (player == Player::COMPUTER and row == human_row and col == human_col)
    return false;
  // Check for collision with bombs.
  bool collides_with_bomb_1 = exists_player_bomb ? (row == br1 && col == bc1) : false;
  //  bool collides_with_bomb_2 = ttl2 ? (r == br2 && c == bc2) : false;
  bool collides_with_bombs = collides_with_bomb_1 /* || collides_with_bomb_2 */;
  // If the position is not illegal, then it's legal.
  return true;
}

// NOTE: does not return!
void print_L()
{
  clear_frame_buffer();
  for (int r = 1; r <= 5; r++)
  {
    frame_buffer[r][2] = 1;
  }
  for (int c = 2; c <= 5; c++)
  {
    frame_buffer[5][c] = 1;
  }
  while (true)
  {
    display(frame_buffer);
  }
}

// NOTE: does not return!
void print_W()
{
  clear_frame_buffer();
  for (int r = 1; r <= 5; r++)
  {
    frame_buffer[r][1] = 1;
    frame_buffer[r][6] = 1;
  }
  for (int c = 1; c <= 6; c++)
  {
    frame_buffer[5][c] = 1;
  }
  for (int r = 3; r <= 4; r++)
  {
    frame_buffer[r][3] = 1;
    frame_buffer[r][4] = 1;
  }
  while (true)
  {
    display(frame_buffer);
  }
}

// NOTE: does not return!
void print_D()
{
  clear_frame_buffer();
  for (int r = 1; r <= 5; r++)
  {
    frame_buffer[r][2] = 1;
  }
  frame_buffer[1][3] = 1;
  frame_buffer[2][4] = 1;
  frame_buffer[3][5] = 1;
  frame_buffer[4][4] = 1;
  frame_buffer[5][3] = 1;
  while (true)
  {
    display(frame_buffer);
  }
}

void callback_computer_action()
{
  Action action = static_cast<Action>(random(static_cast<long>(Action::ACTION_END_MARKER)));
  if (action == Action::MOVE_UP and position_legal(Player::COMPUTER, computer_row - 1, computer_col))
  {
    computer_row--;
  }
  else if (action == Action::MOVE_DOWN and position_legal(Player::COMPUTER, computer_row + 1, computer_col))
  {
    computer_row++;
  }
  else if (action == Action::MOVE_LEFT and position_legal(Player::COMPUTER, computer_row, computer_col - 1))
  {
    computer_col--;
  }
  else if (action == Action::MOVE_RIGHT and position_legal(Player::COMPUTER, computer_row, computer_col + 1))
  {
    computer_col++;
  }
  else if (action == Action::DROP_BOMB)
  {
    // Drop Bomb
  }
}

void callback_human_action(long current_time)
{
  // Process a user action (i.e. drop bomb or move).
  bool sw = digitalRead(SW);
  if (sw == 0)
  {
    // Player action: drop bomb
    if (not exists_player_bomb)
    {
      exists_player_bomb = true;
      player_bomb_explode_time = current_time + 5000;
      br1 = human_row;
      bc1 = human_col;
    }
  }
  else
  {
    // Player action: move
    int y = analogRead(VRY) - 512;
    int x = analogRead(VRX) - 512;
    // Determine whether or not we're in the null region.
    double distance = sqrt(square(y) + square(x));
    Direction d = Direction::Move;
    if (distance < NULL_REGION_RADIUS)
      d = Direction::NoMove;
    if (d == Direction::Move)
    {
      // Determine the movement direction.
      if (y > x and y < -x)
      {
        d = Direction::Down;
      }
      else if (y < x and y < -x)
      {
        d = Direction::Right;
      }
      else if (y < x and y > -x)
      {
        d = Direction::Up;
      }
      else
      {
        d = Direction::Left;
      }
      // Move the player.
      if (d == Direction::Left and position_legal(Player::HUMAN, human_row, human_col - 1))
      {
        human_col--;
      }
      else if (d == Direction::Right and position_legal(Player::HUMAN, human_row, human_col + 1))
      {
        human_col++;
      }
      else if (d == Direction::Up and position_legal(Player::HUMAN, human_row + 1, human_col))
      {
        human_row++;
      }
      else if (d == Direction::Down and position_legal(Player::HUMAN, human_row - 1, human_col))
      {
        human_row--;
      }
    }
  }
}

void loop()
{
  static long next_computer_action = millis();
  static long next_human_action = millis();

  long current_time = millis();

  if (current_time - next_computer_action > 0)
  {
    callback_computer_action();
    next_computer_action = current_time + ACTION_INTERVAL;
  }

  if (current_time - next_human_action > 0)
  {
    callback_human_action(current_time);
    next_human_action = current_time + ACTION_INTERVAL;
  }
  if (exists_player_bomb)
  {
    if (current_time - player_bomb_explode_time > 0)
    {
      exists_player_bomb = false;
      exists_player_bomb_explosion = true;
      player_bomb_explosion_time = current_time + 1000;
    }
  }
  clear_frame_buffer();
  if (exists_player_bomb_explosion)
  {
    if (current_time - player_bomb_explosion_time > 0)
    {
      exists_player_bomb_explosion = false;
    }
    else
    {
      // Check if anybody died.
      if (br1 % 2 == 1)
      {
        for (int i = 0; i < N; i++)
        {
          frame_buffer[br1][i] = 1;
        }
      }
      if (bc1 % 2 == 1)
      {
        for (int i = 0; i < N; i++)
        {
          frame_buffer[i][bc1] = 1;
        }
      }
      bool player_died = false;
      bool computer_died = false;
      if (br1 % 2 == 1)
      {
        for (int i = 0; i < N; i++)
        {
          frame_buffer[br1][i] = 1;
          if (br1 == human_row and i == human_col)
            player_died = true;
          if (br1 == computer_row and i == computer_col)
            computer_died = true;
        }
      }
      if (bc1 % 2 == 1)
      {
        for (int i = 0; i < N; i++)
        {
          if (i == human_row and bc1 == human_col)
            player_died = true;
          if (i == computer_row and bc1 == computer_col)
            computer_died = true;
        }
      }
      if (player_died and not computer_died)
      {
        // Print L
        print_L();
      }
      else if (not player_died and computer_died)
      {
        // Print W
        print_W();
      }
      else if (player_died and computer_died)
      {
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
