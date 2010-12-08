#ifndef __NeoEMU_PRIESTPLAYER_H
#define __NeoEMU_PRIESTPLAYER_H

#include "Player.h"

class PriestPlayer: public Player
{
public:
    PriestPlayer(WorldSession * session): Player(session) {}
    virtual uint8 getClass() const { return CLASS_PRIEST; }
};

#endif