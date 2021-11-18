#ifndef BOMB_H
#define BOMB_H

struct Bomb {
    bool active;
    byte row;
    byte col;
    long expiration;
};

#endif