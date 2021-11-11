// TODO: When a player wins, clear the screen using the classic clearing mechanism and announce the winner.
// TODO: Have a game startup sequence to introduce the game
// TODO: Have a game startup menu to select whether you are playing single player or double player.
// TODO: Give players "health" by dimming the player LED each time they get hit.

#define N 8   // The dimensions of the LED array are N x N.
#define T 50  // How long (in µs) each row of the LED array is lit every cycle.
#define REFRESH_PERIOD 10 // How often the scene should be refreshed (in µs).
#define BOMB_LIFE 4000  // How many frames a bomb with stick around.
#define BOMB_INTERVAL 100  // How many frames a bomb will stay on or off.
#define BOMB_BLAST_LIFE 500 // How many frames a bomb's plast animation will play.

const byte ROWS[N] = {13, 12, 11, 10, 9, 8, 7, 6};
const byte COLS[N] = {A3, A2, A1, A0, 5, 4, 3, 2};

static byte frame_buffer[N][N];

// The position of player 1.
static byte r1 = 7, c1 = 0;

// The position of player 2.
static byte r2 = 0, c2 = 7;

// Variables describing player 1's bomb.
static unsigned long long ttl1, bbl1;
static byte br1, bc1;

// Variables describing player 2's bomb.
static unsigned long long ttl2, bbl2;
static byte br2, bc2;

void setup() {
  // Configure all row and column pins as output pins.
  for (byte i = 0; i < N; i++) {
    pinMode(ROWS[i], OUTPUT);
    pinMode(COLS[i], OUTPUT);
  }
  // Configure the serial baud rate.
  Serial.begin(115200);
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
  // Composite player 1's bomb.
  if (ttl1) {
    int frames_since_dropped = BOMB_LIFE - ttl1;
    if ((frames_since_dropped / BOMB_INTERVAL) % 2)
      frame_buffer[br1][bc1] = 1;
    ttl1--;
  }
  if (ttl1 == 0 && bbl1) {
    // Bomb exploded.
    if (br1 % 2 == 1) {
      for (int i = 0; i < N; i++)
        frame_buffer[br1][i] = 1;
    }
    if (bc1 % 2 == 1) {
      for (int i = 0; i < N; i++)
        frame_buffer[i][bc1] = 1;
    }
    bbl1--;
  }
  // Composite player 2's bomb.
  if (ttl2) {
    int frames_since_dropped = BOMB_LIFE - ttl2;
    if ((frames_since_dropped / BOMB_INTERVAL) % 2)
      frame_buffer[br2][bc2] = 1;
    ttl2--;
  }
  if (ttl2 == 0 && bbl2) {
    // Bomb exploded.
    if (br2 % 2 == 1) {
      for (int i = 0; i < N; i++)
        frame_buffer[br2][i] = 1;
    }
    if (bc2 % 2 == 1) {
      for (int i = 0; i < N; i++)
        frame_buffer[i][bc2] = 1;
    }
    bbl2--;
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
  bool collides_with_bomb_1 = ttl1 ? (r == br1 && c == bc1) : false;
  bool collides_with_bomb_2 = ttl2 ? (r == br2 && c == bc2) : false;
  bool collides_with_bombs = collides_with_bomb_1 || collides_with_bomb_2;
  // Check for out of bounds.
  bool out_of_bounds = r > 7 || c > 7;
  // Collate tests.
  return !collides_with_walls && !collides_with_other_player && !out_of_bounds
    && !collides_with_bombs;
}

void loop() {
  if (Serial.available()) {
    byte command = Serial.read();
    if (command == 'w' && player_position_valid(1, r1 - 1, c1)) {
      // Player 1 moves up.
      r1 -= 1;
    } else if (command == 'a' && player_position_valid(1, r1, c1 - 1)) {
      // Player 1 moves left.
      c1 -= 1;
    } else if (command == 's' && player_position_valid(1, r1 + 1, c1)) {
      // Player 1 moves down.
      r1 += 1;
    } else if (command == 'd' && player_position_valid(1, r1, c1 + 1)) {
      // Player 1 moves right.
      c1 += 1;
    } else if (command == 'g') {
      // Player 1 drops a bomb.
      ttl1 = BOMB_LIFE;
      bbl1 = BOMB_BLAST_LIFE;
      br1 = r1;
      bc1 = c1;
    } else if (command == 'i' && player_position_valid(2, r2 - 1, c2)) {
      // Player 2 moves up.
      r2 -= 1;
    } else if (command == 'j' && player_position_valid(2, r2, c2 - 1)) {
      // Player 2 moves left.
      c2 -= 1;
    } else if (command == 'k' && player_position_valid(2, r2, c2)) {
      // Player 2 moves down.
      r2 += 1;
    } else if (command == 'l' && player_position_valid(3, r2, c2 + 1)) {
      // Player 2 moves right.
      c2 += 1;
    } else if (command == 'n') {
      // Player 2 drops a bomb.
      ttl2 = BOMB_LIFE;
      bbl2 = BOMB_BLAST_LIFE;
      br2 = r2;
      bc2 = c2;
    }
  }
  clear_frame_buffer();
  composite_level();
  composite_players();
  composite_bombs();
  display(frame_buffer);
  delayMicroseconds(REFRESH_PERIOD);
}
