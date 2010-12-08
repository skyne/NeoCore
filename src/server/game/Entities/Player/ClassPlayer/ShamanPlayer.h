#ifndef __NeoEMU_SHAMANPLAYER_H
#define __NeoEMU_SHAMANPLAYER_H

#include "Player.h"

class ShamanPlayer: public Player
{
public:
    ShamanPlayer(WorldSession * session): Player(session) {}
    virtual uint8 getClass() const { return CLASS_SHAMAN; }
};

#endif