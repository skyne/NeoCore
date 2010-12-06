/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "Log.h"
#include "Opcodes.h"
#include "UpdateData.h"
#include "MapManager.h"
#include "Player.h"

void WorldSession::HandleDuelAcceptedOpcode(WorldPacket& recvPacket)
{
    sLog.outDebug("WORLD: received CMSG_DUEL_ACCEPTED");

    uint64 guid;
    Player* player;
    Player* target;

    if (!GetPlayer()->duel)
        return;

    recvPacket >> guid;

    player = GetPlayer();
    target = player->duel->opponent;

    if (player == player->duel->initiator || !target || player == target || player->duel->startTime != 0 || target->duel->startTime != 0)
        return;

    sLog.outDebug("Player 1 is: %u (%s)", player->GetGUIDLow(), player->GetName());
    sLog.outDebug("Player 2 is: %u (%s)", target->GetGUIDLow(), target->GetName());

    time_t now = time(NULL);
    player->duel->startTimer = now;
    target->duel->startTimer = now;

    WorldPacket data(SMSG_DUEL_COUNTDOWN, 4);
    data << (uint32)3000;
    player->GetSession()->SendPacket(&data);
    target->GetSession()->SendPacket(&data);

    //Duel System
    if (sWorld.getConfig(CONFIG_DUEL_SYSTEM))
    {
        player->RemoveAllSpellCooldown();
        target->RemoveAllSpellCooldown();
        player->SetHealth(player->GetMaxHealth());
        target->SetHealth(target->GetMaxHealth());

        if(player->getPowerType() == POWER_MANA)
            player->SetPower(POWER_MANA, player->GetMaxPower(POWER_MANA));
        
        if(target->getPowerType() == POWER_MANA)
            target->SetPower(POWER_MANA, target->GetMaxPower(POWER_MANA));
    }
}

void WorldSession::HandleDuelCancelledOpcode(WorldPacket& recvPacket)
{
    sLog.outDebug("WORLD: received CMSG_DUEL_CANCELLED");

    Player* player = GetPlayer();

    if(!player)
        return;

    if(!player->GetSession())
        return;

    if (!player->duel)
        return;

    if (player->duel->startTime != 0)
    {
        player->CombatStopWithPets(true);

        if (player->duel->opponent)
            player->duel->opponent->CombatStopWithPets(true);

        player->CastSpell(player, 7267, true);
        player->DuelComplete(DUEL_WON);
        return;
    }

    // player either discarded the duel using the "discard button"
    // or used "/forfeit" before countdown reached 0

    uint64 guid;
    recvPacket >> guid;

    player->DuelComplete(DUEL_INTERUPTED);
}

