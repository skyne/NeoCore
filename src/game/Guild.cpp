/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * Copyright (C) 2008-2009 Neo <http://www.neocore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "MapManager.h"
#include "Player.h"
#include "Opcodes.h"
#include "ObjectMgr.h"
#include "Guild.h"
#include "Chat.h"
#include "SocialMgr.h"
#include "Util.h"

Guild::Guild()
{
    Id = 0;
    name = "";
    leaderGuid = 0;
    GINFO = MOTD = "";
    EmblemStyle = 0;
    EmblemColor = 0;
    BorderStyle = 0;
    BorderColor = 0;
    BackgroundColor = 0;

    CreatedYear = 0;
    CreatedMonth = 0;
    CreatedDay = 0;
}

Guild::~Guild()
{

}

bool Guild::create(uint64 lGuid, std::string gname)
{
    std::string rname;
    std::string lName;

    if(!objmgr.GetPlayerNameByGUID(lGuid, lName))
        return false;
    if(objmgr.GetGuildByName(gname))
        return false;

    sLog.outDebug("GUILD: creating guild %s to leader: %u", gname.c_str(), GUID_LOPART(lGuid));

    leaderGuid = lGuid;
    name = gname;
    GINFO = "";
    MOTD = "No message set.";
    purchased_tabs = 0;

    Id = objmgr.GenerateGuildId();

    // gname already assigned to Guild::name, use it to encode string for DB
    CharacterDatabase.escape_string(gname);

    std::string dbGINFO = GINFO;
    std::string dbMOTD = MOTD;
    CharacterDatabase.escape_string(dbGINFO);
    CharacterDatabase.escape_string(dbMOTD);

    CharacterDatabase.BeginTransaction();
    // CharacterDatabase.PExecute("DELETE FROM guild WHERE guildid='%u'", Id); - MAX(guildid)+1 not exist
    CharacterDatabase.PExecute("DELETE FROM guild_rank WHERE guildid='%u'", Id);
    CharacterDatabase.PExecute("DELETE FROM guild_member WHERE guildid='%u'", Id);
    CharacterDatabase.PExecute("INSERT INTO guild (guildid,name,leaderguid,info,motd,createdate,EmblemStyle,EmblemColor,BorderStyle,BorderColor,BackgroundColor) " 
        "VALUES('%u','%s','%u', '%s', '%s', NOW(),'%u','%u','%u','%u','%u')", 
        Id, gname.c_str(), GUID_LOPART(leaderGuid), dbGINFO.c_str(), dbMOTD.c_str(), EmblemStyle, EmblemColor, BorderStyle, BorderColor, BackgroundColor); 
    CharacterDatabase.CommitTransaction(); 

    rname = "Guild Master";
    CreateRank(rname,GR_RIGHT_ALL);
    rname = "Officer";
    CreateRank(rname,GR_RIGHT_ALL);
    rname = "Veteran";
    CreateRank(rname,GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);
    rname = "Member";
    CreateRank(rname,GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);
    rname = "Initiate";
    CreateRank(rname,GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);

    return AddMember(lGuid, (uint32)GR_GUILDMASTER);
}

bool Guild::AddMember(uint64 plGuid, uint32 plRank)
{
    Player* pl = objmgr.GetPlayer(plGuid);
    if(pl)
    {
        if(pl->GetGuildId() != 0)
            return false;
    }
    else
    {
        if(Player::GetGuildIdFromDB(plGuid) != 0)           // player already in guild
            return false;
    }

    // remove all player signs from another petitions
    // this will be prevent attempt joining player to many guilds and corrupt guild data integrity
    Player::RemovePetitionsAndSigns(plGuid, 9);

    // fill player data
    MemberSlot newmember;

    if(!FillPlayerData(plGuid, &newmember))                 // problems with player data collection
        return false;

    newmember.RankId = plRank;
    newmember.OFFnote = (std::string)"";
    newmember.Pnote = (std::string)"";
    newmember.logout_time = time(NULL);
    members[GUID_LOPART(plGuid)] = newmember;

    std::string dbPnote = newmember.Pnote;
    std::string dbOFFnote = newmember.OFFnote;
    CharacterDatabase.escape_string(dbPnote);
    CharacterDatabase.escape_string(dbOFFnote);

    CharacterDatabase.PExecute("INSERT INTO guild_member (guildid,guid,rank,pnote,offnote) VALUES ('%u', '%u', '%u','%s','%s')",
        Id, GUID_LOPART(plGuid), newmember.RankId, dbPnote.c_str(), dbOFFnote.c_str());

    // If player not in game data in data field will be loaded from guild tables, no need to update it!!
    if(pl)
    {
        pl->SetInGuild(Id);
        pl->SetRank(newmember.RankId);
        pl->SetGuildIdInvited(0);
    }
    return true;
}

void Guild::SetMOTD(std::string motd)
{
    MOTD = motd;

    // motd now can be used for encoding to DB
    CharacterDatabase.escape_string(motd);
    CharacterDatabase.PExecute("UPDATE guild SET motd='%s' WHERE guildid='%u'", motd.c_str(), Id);
}

void Guild::SetGINFO(std::string ginfo)
{
    GINFO = ginfo;

    // ginfo now can be used for encoding to DB
    CharacterDatabase.escape_string(ginfo);
    CharacterDatabase.PExecute("UPDATE guild SET info='%s' WHERE guildid='%u'", ginfo.c_str(), Id);
}

bool Guild::LoadGuildFromDB(uint32 GuildId)
{
    if(!LoadRanksFromDB(GuildId))
        return false;

    if(!LoadMembersFromDB(GuildId))
        return false;

    //                                        0        1     2           3            4            5           6
    QueryResult_AutoPtr result = CharacterDatabase.PQuery("SELECT guildid, name, leaderguid, EmblemStyle, EmblemColor, BorderStyle, BorderColor,"
    //   7                8     9     10
        "BackgroundColor, info, motd, createdate FROM guild WHERE guildid = '%u'", GuildId);

    if(!result)
        return false;

    Field *fields = result->Fetch();

    Id = fields[0].GetUInt32();
    name = fields[1].GetCppString();
    leaderGuid  = MAKE_NEW_GUID(fields[2].GetUInt32(), 0, HIGHGUID_PLAYER);

    EmblemStyle = fields[3].GetUInt32();
    EmblemColor = fields[4].GetUInt32();
    BorderStyle = fields[5].GetUInt32();
    BorderColor = fields[6].GetUInt32();
    BackgroundColor = fields[7].GetUInt32();
    GINFO = fields[8].GetCppString();
    MOTD = fields[9].GetCppString();
    uint64 time = fields[10].GetUInt64();                   //datetime is uint64 type ... YYYYmmdd:hh:mm:ss

    

    uint64 dTime = time /1000000;
    CreatedDay   = dTime%100;
    CreatedMonth = (dTime/100)%100;
    CreatedYear  = (dTime/10000)%10000;

    // If the leader does not exist attempt to promote another member
    if(!objmgr.GetPlayerAccountIdByGUID(leaderGuid ))
    {
        DelMember(leaderGuid);

        // check no members case (disbanded)
        if(members.empty())
            return false;
    }

    sLog.outDebug("Guild %u Creation time Loaded day: %u, month: %u, year: %u", GuildId, CreatedDay, CreatedMonth, CreatedYear);
    m_eventlogloaded = false;
    m_onlinemembers = 0;
    RenumGuildEventlog();
    return true;
}

bool Guild::LoadRanksFromDB(uint32 GuildId)
{
    Field *fields;
    QueryResult_AutoPtr result = CharacterDatabase.PQuery("SELECT rname,rights,rid FROM guild_rank WHERE guildid = '%u' ORDER BY rid ASC", GuildId);

    if(!result)
        return false;

    bool broken_ranks = false;

    do
    {
        fields = result->Fetch();

        std::string rankName = fields[0].GetCppString();
        uint32 rankRights    = fields[1].GetUInt32();
        uint32 rankRID       = fields[2].GetUInt32();

        if(rankRID != m_ranks.size()+1)                     // guild_rank.rid always store rank+1
            broken_ranks =  true;

        if(m_ranks.size()==GR_GUILDMASTER)                  // prevent loss leader rights
            rankRights |= GR_RIGHT_ALL;

        AddRank(rankName,rankRights);
    }while( result->NextRow() );
    

    if(m_ranks.size()==0)                                   // empty rank table?
    {
        AddRank("Guild Master",GR_RIGHT_ALL);
        broken_ranks = true;
    }

    // guild_rank have wrong numbered ranks, repair
    if(broken_ranks)
    {
        sLog.outError("Guild %u have broken `guild_rank` data, repairing...",GuildId);
        CharacterDatabase.BeginTransaction();
        CharacterDatabase.PExecute("DELETE FROM guild_rank WHERE guildid='%u'", GuildId);
        for(size_t i =0; i < m_ranks.size(); ++i)
        {
            // guild_rank.rid always store rank+1
            std::string name = m_ranks[i].name;
            uint32 rights = m_ranks[i].rights;
            CharacterDatabase.escape_string(name);
            CharacterDatabase.PExecute( "INSERT INTO guild_rank (guildid,rid,rname,rights) VALUES ('%u', '%u', '%s', '%u')", GuildId, i+1, name.c_str(), rights);
        }
        CharacterDatabase.CommitTransaction();
    }

    return true;
}

bool Guild::LoadMembersFromDB(uint32 GuildId)
{
    //                                                     0                 1     2      3        4
    QueryResult_AutoPtr result = CharacterDatabase.PQuery("SELECT guild_member.guid,rank, pnote, offnote,logout_time FROM guild_member LEFT JOIN characters ON characters.guid = guild_member.guid WHERE guildid = '%u'", GuildId);

    if(!result)
        return false;

    do
    {
        Field *fields = result->Fetch();
        MemberSlot newmember;
        newmember.RankId = fields[1].GetUInt32();
        uint64 guid = MAKE_NEW_GUID(fields[0].GetUInt32(), 0, HIGHGUID_PLAYER);

        // Player does not exist
        if(!FillPlayerData(guid, &newmember))
            continue;

        newmember.Pnote                 = fields[2].GetCppString();
        newmember.OFFnote               = fields[3].GetCppString();
        newmember.logout_time           = fields[4].GetUInt64();
        members[GUID_LOPART(guid)]      = newmember;

    }while( result->NextRow() );
    

    if(members.empty())
        return false;

    return true;
}

bool Guild::FillPlayerData(uint64 guid, MemberSlot* memslot)
{
    std::string plName;
    uint32 plLevel;
    uint32 plClass;
    uint32 plZone;

    Player* pl = objmgr.GetPlayer(guid);
    if(pl)
    {
        plName  = pl->GetName();
        plLevel = pl->getLevel();
        plClass = pl->getClass();
        plZone  = pl->GetZoneId();
    }
    else
    {
        PCachePlayerInfo pInfo = objmgr.GetPlayerInfoFromCache(GUID_LOPART(guid));
        if(pInfo)
        {
            plName = pInfo->sPlayerName;
            plClass = pInfo->unClass;
            if(plClass<CLASS_WARRIOR||plClass>=MAX_CLASSES)     // can be at broken `class` field
            {
                sLog.outError("Player (GUID: %u) has a broken data in field `characters`.`class`.",GUID_LOPART(guid));
                return false;
            }
            plLevel = pInfo->unLevel;
            plZone = Player::GetZoneIdFromDB(guid);
        }
        else
        {
            QueryResult_AutoPtr result = CharacterDatabase.PQuery("SELECT name,data,zone,class FROM characters WHERE guid = '%u'", GUID_LOPART(guid));
            if(!result)
                return false;                                   // player doesn't exist

            Field *fields = result->Fetch();

            plName = fields[0].GetCppString();

            Tokens data = StrSplit(fields[1].GetCppString(), " ");
            plLevel = Player::GetUInt32ValueFromArray(data,UNIT_FIELD_LEVEL);

            plZone = fields[2].GetUInt32();
            plClass = fields[3].GetUInt32();
            

        if(plLevel<1||plLevel>STRONG_MAX_LEVEL)             // can be at broken `data` field
        {
            sLog.outError("Player (GUID: %u) has a broken data in field `characters`.`data`.",GUID_LOPART(guid));
            return false;
        }

        if(!plZone)
        {
            sLog.outError("Player (GUID: %u) has broken zone-data",GUID_LOPART(guid));
            //here it will also try the same, to get the zone from characters-table, but additional it tries to find
            plZone = Player::GetZoneIdFromDB(guid);
            //the zone through xy coords.. this is a bit redundant, but
            //shouldn't be called often
        }

        if(plClass<CLASS_WARRIOR||plClass>=MAX_CLASSES)     // can be at broken `class` field
        {
            sLog.outError("Player (GUID: %u) has a broken data in field `characters`.`class`.",GUID_LOPART(guid));
            return false;
        }
    }
    }

    memslot->name = plName;
    memslot->level = plLevel;
    memslot->Class = plClass;
    memslot->zoneId = plZone;

    return(true);
}

void Guild::LoadPlayerStatsByGuid(uint64 guid)
{
    MemberList::iterator itr = members.find(GUID_LOPART(guid));
    if (itr == members.end() )
        return;

    Player *pl = ObjectAccessor::FindPlayer(guid);
    if(!pl)
        return;
    itr->second.name  = pl->GetName();
    itr->second.level = pl->getLevel();
    itr->second.Class = pl->getClass();
}

void Guild::SetLeader(uint64 guid)
{
    leaderGuid = guid;
    ChangeRank(guid, GR_GUILDMASTER);

    CharacterDatabase.PExecute("UPDATE guild SET leaderguid='%u' WHERE guildid='%u'", GUID_LOPART(guid), Id);
}

void Guild::DelMember(uint64 guid, bool isDisbanding)
{
    if(leaderGuid == guid && !isDisbanding)
    {
        MemberSlot* oldLeader = NULL;
        MemberSlot* best = NULL;
        uint64 newLeaderGUID = 0;
        for(Guild::MemberList::iterator i = members.begin(); i != members.end(); ++i)
        {
            if(i->first == GUID_LOPART(guid))
            {
                oldLeader = &(i->second);
                continue;
            }

            if(!best || best->RankId > i->second.RankId)
            {
                best = &(i->second);
                newLeaderGUID = i->first;
            }
        }
        if(!best)
        {
            Disband();
            return;
        }

        SetLeader(newLeaderGUID);

        // If player not online data in data field will be loaded from guild tabs no need to update it !!
        if(Player *newLeader = objmgr.GetPlayer(newLeaderGUID))
            newLeader->SetRank(GR_GUILDMASTER);

        // when leader non-exist (at guild load with deleted leader only) not send broadcasts
        if(oldLeader)
        {
            WorldPacket data(SMSG_GUILD_EVENT, (1+1+(oldLeader->name).size()+1+(best->name).size()+1));
            data << (uint8)GE_LEADER_CHANGED;
            data << (uint8)2;
            data << oldLeader->name;
            data << best->name;
            BroadcastPacket(&data);

            data.Initialize(SMSG_GUILD_EVENT, (1+1+(oldLeader->name).size()+1));
            data << (uint8)GE_LEFT;
            data << (uint8)1;
            data << oldLeader->name;
            BroadcastPacket(&data);
        }

        sLog.outDebug( "WORLD: Sent (SMSG_GUILD_EVENT)" );
    }

    members.erase(GUID_LOPART(guid));

    Player *player = objmgr.GetPlayer(guid);
    // If player not online data in data field will be loaded from guild tabs no need to update it !!
    if(player)
    {
        player->SetInGuild(0);
        player->SetRank(0);
    }

    CharacterDatabase.PExecute("DELETE FROM guild_member WHERE guid = '%u'", GUID_LOPART(guid));
}

void Guild::ChangeRank(uint64 guid, uint32 newRank)
{
    MemberList::iterator itr = members.find(GUID_LOPART(guid));
    if( itr != members.end() )
        itr->second.RankId = newRank;

    Player *player = objmgr.GetPlayer(guid);
    // If player not online data in data field will be loaded from guild tabs no need to update it !!
    if(player)
        player->SetRank(newRank);

    CharacterDatabase.PExecute( "UPDATE guild_member SET rank='%u' WHERE guid='%u'", newRank, GUID_LOPART(guid) );
}

void Guild::SetPNOTE(uint64 guid,std::string pnote)
{
    MemberList::iterator itr = members.find(GUID_LOPART(guid));
    if( itr == members.end() )
        return;

    itr->second.Pnote = pnote;

    // pnote now can be used for encoding to DB
    CharacterDatabase.escape_string(pnote);
    CharacterDatabase.PExecute("UPDATE guild_member SET pnote = '%s' WHERE guid = '%u'", pnote.c_str(), itr->first);
}

void Guild::SetOFFNOTE(uint64 guid,std::string offnote)
{
    MemberList::iterator itr = members.find(GUID_LOPART(guid));
    if( itr == members.end() )
        return;
    itr->second.OFFnote = offnote;
    // offnote now can be used for encoding to DB
    CharacterDatabase.escape_string(offnote);
    CharacterDatabase.PExecute("UPDATE guild_member SET offnote = '%s' WHERE guid = '%u'", offnote.c_str(), itr->first);
}

void Guild::BroadcastToGuild(WorldSession *session, const std::string& msg, uint32 language)
{
    if (session && session->GetPlayer() && HasRankRight(session->GetPlayer()->GetRank(),GR_RIGHT_GCHATSPEAK))
    {
        WorldPacket data;
        ChatHandler(session).FillMessageData(&data, CHAT_MSG_GUILD, language, 0, msg.c_str());

        for (MemberList::const_iterator itr = members.begin(); itr != members.end(); ++itr)
        {
            Player *pl = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(itr->first, 0, HIGHGUID_PLAYER));

            if (pl && pl->GetSession() && HasRankRight(pl->GetRank(),GR_RIGHT_GCHATLISTEN) && !pl->GetSocial()->HasIgnore(session->GetPlayer()->GetGUIDLow()) )
                pl->GetSession()->SendPacket(&data);
        }
    }
}

void Guild::BroadcastToOfficers(WorldSession *session, const std::string& msg, uint32 language)
{
    if (session && session->GetPlayer() && HasRankRight(session->GetPlayer()->GetRank(),GR_RIGHT_OFFCHATSPEAK))
    {
        for(MemberList::iterator itr = members.begin(); itr != members.end(); ++itr)
        {
            WorldPacket data;
            ChatHandler::FillMessageData(&data, session, CHAT_MSG_OFFICER, language, NULL, 0, msg.c_str(),NULL);

            Player *pl = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(itr->first, 0, HIGHGUID_PLAYER));

            if (pl && pl->GetSession() && HasRankRight(pl->GetRank(),GR_RIGHT_OFFCHATLISTEN) && !pl->GetSocial()->HasIgnore(session->GetPlayer()->GetGUIDLow()))
                pl->GetSession()->SendPacket(&data);
        }
    }
}

void Guild::BroadcastPacket(WorldPacket *packet)
{
    for(MemberList::iterator itr = members.begin(); itr != members.end(); ++itr)
    {
        Player *player = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(itr->first, 0, HIGHGUID_PLAYER));
        if(player)
            player->GetSession()->SendPacket(packet);
    }
}

void Guild::BroadcastPacketToRank(WorldPacket *packet, uint32 rankId)
{
    for(MemberList::iterator itr = members.begin(); itr != members.end(); ++itr)
    {
        if (itr->second.RankId == rankId)
        {
            Player *player = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(itr->first, 0, HIGHGUID_PLAYER));
            if(player)
                player->GetSession()->SendPacket(packet);
        }
    }
}

void Guild::CreateRank(std::string name_,uint32 rights)
{
    if(m_ranks.size() >= GUILD_MAX_RANKS)
        return;

    AddRank(name_,rights);

    // guild_rank.rid always store rank+1 value

    // name now can be used for encoding to DB
    CharacterDatabase.escape_string(name_);
    CharacterDatabase.PExecute( "INSERT INTO guild_rank (guildid,rid,rname,rights) VALUES ('%u', '%u', '%s', '%u')", Id, m_ranks.size(), name_.c_str(), rights );
}

void Guild::AddRank(const std::string& name_,uint32 rights)
{
    m_ranks.push_back(RankInfo(name_,rights));
}

void Guild::DelRank()
{
    if(m_ranks.empty())
        return;

    // guild_rank.rid always store rank+1 value
    uint32 rank = m_ranks.size()-1;
    CharacterDatabase.PExecute("DELETE FROM guild_rank WHERE rid>='%u' AND guildid='%u'", (rank+1), Id);

    m_ranks.pop_back();
}

std::string Guild::GetRankName(uint32 rankId)
{
    if(rankId >= m_ranks.size())
        return "<unknown>";

    return m_ranks[rankId].name;
}

uint32 Guild::GetRankRights(uint32 rankId)
{
    if(rankId >= m_ranks.size())
        return 0;

    return m_ranks[rankId].rights;
}

void Guild::SetRankName(uint32 rankId, std::string name_)
{
    if(rankId >= m_ranks.size())
        return;

    m_ranks[rankId].name = name_;

    // name now can be used for encoding to DB
    CharacterDatabase.escape_string(name_);
    CharacterDatabase.PExecute("UPDATE guild_rank SET rname='%s' WHERE rid='%u' AND guildid='%u'", name_.c_str(), (rankId+1), Id);
}

void Guild::SetRankRights(uint32 rankId, uint32 rights)
{
    if(rankId >= m_ranks.size())
        return;

    m_ranks[rankId].rights = rights;

    CharacterDatabase.PExecute("UPDATE guild_rank SET rights='%u' WHERE rid='%u' AND guildid='%u'", rights, (rankId+1), Id);
}

int32 Guild::GetRank(uint32 LowGuid)
{
    MemberList::iterator itr = members.find(LowGuid);
    if (itr==members.end())
        return -1;

    return itr->second.RankId;
}

void Guild::Disband()
{
    WorldPacket data(SMSG_GUILD_EVENT, 1);
    data << (uint8)GE_DISBANDED;
    BroadcastPacket(&data);

    while (!members.empty())
    {
        MemberList::iterator itr = members.begin();
        DelMember(MAKE_NEW_GUID(itr->first, 0, HIGHGUID_PLAYER), true);
    }

    CharacterDatabase.BeginTransaction();
    CharacterDatabase.PExecute("DELETE FROM guild WHERE guildid = '%u'",Id);
    CharacterDatabase.PExecute("DELETE FROM guild_rank WHERE guildid = '%u'",Id);
    CharacterDatabase.PExecute("DELETE FROM guild_eventlog WHERE guildid = '%u'",Id);
    CharacterDatabase.CommitTransaction();
    objmgr.RemoveGuild(Id);
}

void Guild::Roster(WorldSession *session)
{
                                                            // we can only guess size
    WorldPacket data(SMSG_GUILD_ROSTER, (4+MOTD.length()+1+GINFO.length()+1+4+m_ranks.size()*4+members.size()*50));
    data << (uint32)members.size();
    data << MOTD;
    data << GINFO;

    data << (uint32)m_ranks.size();
    for (RankList::const_iterator ritr = m_ranks.begin(); ritr != m_ranks.end(); ++ritr)
        data << (uint32)ritr->rights;

    for (MemberList::const_iterator itr = members.begin(); itr != members.end(); ++itr)
    {
        if (Player *pl = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(itr->first, 0, HIGHGUID_PLAYER)))
        {
            data << (uint64)pl->GetGUID();
            data << (uint8)1;
            data << (std::string)pl->GetName();
            data << (uint32)itr->second.RankId;
            data << (uint8)pl->getLevel();
            data << (uint8)pl->getClass();
            data << (uint32)pl->GetZoneId();
            data << itr->second.Pnote;
            data << itr->second.OFFnote;
        }
        else
        {
            data << uint64(MAKE_NEW_GUID(itr->first, 0, HIGHGUID_PLAYER));
            data << (uint8)0;
            data << itr->second.name;
            data << (uint32)itr->second.RankId;
            data << (uint8)itr->second.level;
            data << (uint8)itr->second.Class;
            data << (uint32)itr->second.zoneId;
            data << (float(time(NULL)-itr->second.logout_time) / DAY);
            data << itr->second.Pnote;
            data << itr->second.OFFnote;
        }
    }
    session->SendPacket(&data);;
    sLog.outDebug( "WORLD: Sent (SMSG_GUILD_ROSTER)" );
}

void Guild::Query(WorldSession *session)
{
    WorldPacket data(SMSG_GUILD_QUERY_RESPONSE, (8*32+200));// we can only guess size

    data << Id;
    data << name;

    for (size_t i = 0 ; i < 10; ++i)                        // show always 10 ranks
    {
        if(i < m_ranks.size())
            data << m_ranks[i].name;
        else
            data << (uint8)0;                               // null string
    }

    data << uint32(EmblemStyle);
    data << uint32(EmblemColor);
    data << uint32(BorderStyle);
    data << uint32(BorderColor);
    data << uint32(BackgroundColor);

    session->SendPacket( &data );
    sLog.outDebug( "WORLD: Sent (SMSG_GUILD_QUERY_RESPONSE)" );
}

void Guild::SetEmblem(uint32 emblemStyle, uint32 emblemColor, uint32 borderStyle, uint32 borderColor, uint32 backgroundColor)
{
    EmblemStyle = emblemStyle;
    EmblemColor = emblemColor;
    BorderStyle = borderStyle;
    BorderColor = borderColor;
    BackgroundColor = backgroundColor;

    CharacterDatabase.PExecute("UPDATE guild SET EmblemStyle=%u, EmblemColor=%u, BorderStyle=%u, BorderColor=%u, BackgroundColor=%u WHERE guildid = %u", EmblemStyle, EmblemColor, BorderStyle, BorderColor, BackgroundColor, Id);
}

void Guild::UpdateLogoutTime(uint64 guid)
{
    MemberList::iterator itr = members.find(GUID_LOPART(guid));
    if (itr == members.end() )
        return;

    itr->second.logout_time = time(NULL);

    if (m_onlinemembers > 0)
        --m_onlinemembers;
    else
        UnloadGuildEventlog();
}

// *************************************************
// Guild Eventlog part
// *************************************************
// Display guild eventlog
void Guild::DisplayGuildEventlog(WorldSession *session)
{
    // Load guild eventlog, if not already done
    if (!m_eventlogloaded)
        LoadGuildEventLogFromDB();

    // Sending result
    WorldPacket data(MSG_GUILD_EVENT_LOG_QUERY, 0);
    // count, max count == 100
    data << uint8(m_GuildEventlog.size());
    for (GuildEventlog::const_iterator itr = m_GuildEventlog.begin(); itr != m_GuildEventlog.end(); ++itr)
    {
        // Event type
        data << uint8((*itr)->EventType);
        // Player 1
        data << uint64((*itr)->PlayerGuid1);
        // Player 2 not for left/join guild events
        if( (*itr)->EventType != GUILD_EVENT_LOG_JOIN_GUILD && (*itr)->EventType != GUILD_EVENT_LOG_LEAVE_GUILD )
            data << uint64((*itr)->PlayerGuid2);
        // New Rank - only for promote/demote guild events
        if( (*itr)->EventType == GUILD_EVENT_LOG_PROMOTE_PLAYER || (*itr)->EventType == GUILD_EVENT_LOG_DEMOTE_PLAYER )
            data << uint8((*itr)->NewRank);
        // Event timestamp
        data << uint32(time(NULL)-(*itr)->TimeStamp);
    }
    session->SendPacket(&data);
    sLog.outDebug("WORLD: Sent (MSG_GUILD_EVENT_LOG_QUERY)");
}

// Load guild eventlog from DB
void Guild::LoadGuildEventLogFromDB()
{
    // Return if already loaded
    if (m_eventlogloaded)
        return;

    QueryResult_AutoPtr result = CharacterDatabase.PQuery("SELECT LogGuid, EventType, PlayerGuid1, PlayerGuid2, NewRank, TimeStamp FROM guild_eventlog WHERE guildid=%u ORDER BY LogGuid DESC LIMIT %u", Id, GUILD_EVENTLOG_MAX_ENTRIES);
    if(!result)
        return;
    do
    {
        Field *fields = result->Fetch();
        GuildEventlogEntry *NewEvent = new GuildEventlogEntry;
        // Fill entry
        NewEvent->LogGuid = fields[0].GetUInt32();
        NewEvent->EventType = fields[1].GetUInt8();
        NewEvent->PlayerGuid1 = fields[2].GetUInt32();
        NewEvent->PlayerGuid2 = fields[3].GetUInt32();
        NewEvent->NewRank = fields[4].GetUInt8();
        NewEvent->TimeStamp = fields[5].GetUInt64();
        // Add entry to map
        m_GuildEventlog.push_front(NewEvent);

    } while( result->NextRow() );
    

    // Check lists size in case to many event entries in db
    // This cases can happen only if a crash occured somewhere and table has too many log entries
    if (!m_GuildEventlog.empty())
    {
        CharacterDatabase.PExecute("DELETE FROM guild_eventlog WHERE guildid=%u AND LogGuid < %u", Id, m_GuildEventlog.front()->LogGuid);
    }
    m_eventlogloaded = true;
}

// Unload guild eventlog
void Guild::UnloadGuildEventlog()
{
    if (!m_eventlogloaded)
        return;
    GuildEventlogEntry *EventLogEntry;
    if( !m_GuildEventlog.empty() )
    {
        do
        {
            EventLogEntry = *(m_GuildEventlog.begin());
            m_GuildEventlog.pop_front();
            delete EventLogEntry;
        }while( !m_GuildEventlog.empty() );
    }
    m_eventlogloaded = false;
}

// This will renum guids used at load to prevent always going up until infinit
void Guild::RenumGuildEventlog()
{
    QueryResult_AutoPtr result = CharacterDatabase.PQuery("SELECT Min(LogGuid), Max(LogGuid) FROM guild_eventlog WHERE guildid = %u", Id);
    if(!result)
        return;

    Field *fields = result->Fetch();
    CharacterDatabase.PExecute("UPDATE guild_eventlog SET LogGuid=LogGuid-%u+1 WHERE guildid=%u ORDER BY LogGuid %s",fields[0].GetUInt32(), Id, fields[0].GetUInt32()?"ASC":"DESC");
    GuildEventlogMaxGuid = fields[1].GetUInt32()+1;
    
}

// Add entry to guild eventlog
void Guild::LogGuildEvent(uint8 EventType, uint32 PlayerGuid1, uint32 PlayerGuid2, uint8 NewRank)
{
    GuildEventlogEntry *NewEvent = new GuildEventlogEntry;
    // Fill entry
    NewEvent->LogGuid = GuildEventlogMaxGuid++;
    NewEvent->EventType = EventType;
    NewEvent->PlayerGuid1 = PlayerGuid1;
    NewEvent->PlayerGuid2 = PlayerGuid2;
    NewEvent->NewRank = NewRank;
    NewEvent->TimeStamp = uint32(time(NULL));
    // Check max entry limit and delete from db if needed
    if (m_GuildEventlog.size() > GUILD_EVENTLOG_MAX_ENTRIES)
    {
        GuildEventlogEntry *OldEvent = *(m_GuildEventlog.begin());
        m_GuildEventlog.pop_front();
        CharacterDatabase.PExecute("DELETE FROM guild_eventlog WHERE guildid='%u' AND LogGuid='%u'", Id, OldEvent->LogGuid);
        delete OldEvent;
    }
    // Add entry to map
    m_GuildEventlog.push_back(NewEvent);
    // Add new eventlog entry into DB
    CharacterDatabase.PExecute("INSERT INTO guild_eventlog (guildid, LogGuid, EventType, PlayerGuid1, PlayerGuid2, NewRank, TimeStamp) VALUES ('%u','%u','%u','%u','%u','%u','" UI64FMTD "')",
        Id, NewEvent->LogGuid, uint32(NewEvent->EventType), NewEvent->PlayerGuid1, NewEvent->PlayerGuid2, uint32(NewEvent->NewRank), NewEvent->TimeStamp);
}
