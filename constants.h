#ifndef CONSTANTS_H
#define CONSTANTS_H

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

#endif