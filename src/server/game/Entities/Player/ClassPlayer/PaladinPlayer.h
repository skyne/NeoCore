#ifndef __NeoEMU_PALADINPLAYER_H
#define __NeoEMU_PALADINPLAYER_H

#include "Player.h"

class PaladinPlayer: public Player
{
public:
    PaladinPlayer(WorldSession * session): Player(session) {}
    virtual uint8 getClass() const { return CLASS_PALADIN; }
};

#endif