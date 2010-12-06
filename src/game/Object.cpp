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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "Common.h"
#include "SharedDefines.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "Object.h"
#include "Creature.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "UpdateData.h"
#include "UpdateMask.h"
#include "Util.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Log.h"
#include "Transports.h"
#include "TargetedMovementGenerator.h"
#include "WaypointMovementGenerator.h"
#include "VMapFactory.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"

#include "TemporarySummon.h"

uint32 GuidHigh2TypeId(uint32 guid_hi)
{
    switch(guid_hi)
    {
        case HIGHGUID_ITEM:         return TYPEID_ITEM;
        //case HIGHGUID_CONTAINER:    return TYPEID_CONTAINER; HIGHGUID_CONTAINER==HIGHGUID_ITEM currently
        case HIGHGUID_UNIT:         return TYPEID_UNIT;
        case HIGHGUID_PET:          return TYPEID_UNIT;
        case HIGHGUID_PLAYER:       return TYPEID_PLAYER;
        case HIGHGUID_GAMEOBJECT:   return TYPEID_GAMEOBJECT;
        case HIGHGUID_DYNAMICOBJECT:return TYPEID_DYNAMICOBJECT;
        case HIGHGUID_CORPSE:       return TYPEID_CORPSE;
        case HIGHGUID_MO_TRANSPORT: return TYPEID_GAMEOBJECT;
    }
    return 10;                                              // unknown
}

Object::Object( )
{
    m_objectTypeId      = TYPEID_OBJECT;
    m_objectType        = TYPEMASK_OBJECT;

    m_uint32Values      = 0;
    m_uint32Values_mirror = 0;
    m_valuesCount       = 0;

    m_inWorld           = false;
    m_objectUpdated     = false;

    m_PackGUID.clear();
    m_PackGUID.appendPackGUID(0);
}

Object::~Object( )
{
    //if(m_objectUpdated)
    //    ObjectAccessor::Instance().RemoveUpdateObject(this);

    if(m_uint32Values)
    {
        if(IsInWorld())
        {
            ///- Do NOT call RemoveFromWorld here, if the object is a player it will crash
            sLog.outError("Object::~Object - guid="UI64FMTD", typeid=%d deleted but still in world!!", GetGUID(), GetTypeId());
            assert(false);
        }

        assert(!m_objectUpdated);

        //DEBUG_LOG("Object desctr 1 check (%p)",(void*)this);
        delete [] m_uint32Values;
        delete [] m_uint32Values_mirror;
        m_uint32Values = NULL;
        m_uint32Values_mirror = NULL;
        //DEBUG_LOG("Object desctr 2 check (%p)",(void*)this);
    }
}

void Object::_InitValues()
{
    m_uint32Values = new uint32[ m_valuesCount ];
    memset(m_uint32Values, 0, m_valuesCount*sizeof(uint32));

    m_uint32Values_mirror = new uint32[ m_valuesCount ];
    memset(m_uint32Values_mirror, 0, m_valuesCount*sizeof(uint32));

    m_objectUpdated = false;
}

void Object::_Create( uint32 guidlow, uint32 entry, HighGuid guidhigh )
{
    if(!m_uint32Values) _InitValues();

    uint64 guid = MAKE_NEW_GUID(guidlow, entry, guidhigh);  // required more changes to make it working
    SetUInt64Value( OBJECT_FIELD_GUID, guid );
    SetUInt32Value( OBJECT_FIELD_TYPE, m_objectType );
    m_PackGUID.clear();
    m_PackGUID.appendPackGUID(GetGUID());
}

void Object::BuildMovementUpdateBlock(UpdateData * data, uint32 flags ) const
{
    ByteBuffer buf(500);

    buf << uint8( UPDATETYPE_MOVEMENT );
    buf << GetGUID();

    _BuildMovementUpdate(&buf, flags, 0x00000000);

    data->AddUpdateBlock(buf);
}

void Object::BuildCreateUpdateBlockForPlayer(UpdateData *data, Player *target) const
{
    if(!target) return;

    uint8  updatetype = UPDATETYPE_CREATE_OBJECT;

    ByteBuffer buf(500);
    buf << (uint8)updatetype;
    buf << (uint8)0xFF << GetGUID() ;
    buf << (uint8)m_objectTypeId;

    switch(m_objectTypeId)
    {
        case TYPEID_OBJECT:                                 //do nothing
            break;
        case TYPEID_ITEM:
        case TYPEID_CONTAINER:
            _BuildMovementUpdate( &buf, 0x10, 0x0 );
            break;
        case TYPEID_UNIT:
            _BuildMovementUpdate( &buf, 0x70, 0x800000 );
            break;
        case TYPEID_PLAYER:
        {
            if( target == this )                            //build for self
            {
                buf.clear();
                buf << uint8( UPDATETYPE_CREATE_OBJECT2 );
                buf << uint8( 0xFF );                       // must be packet GUID ?
                buf << GetGUID() ;
                buf << m_objectTypeId;
                _BuildMovementUpdate( &buf, 0x71, 0x2000 );
            }
            //build for other player
            else
            {
                _BuildMovementUpdate( &buf, 0x70, 0x0 );
            }
        }break;
        case TYPEID_CORPSE:
        case TYPEID_GAMEOBJECT:
        case TYPEID_DYNAMICOBJECT:
        {
            if ((GUID_HIPART(GetGUID())==HIGHGUID_PLAYER_CORPSE) || (GUID_HIPART(GetGUID()) == HIGHGUID_TRANSPORT))
                _BuildMovementUpdate( &buf, 0x52, 0x0 );
            else
                _BuildMovementUpdate( &buf, 0x50, 0x0 );
        }break;
        //case TYPEID_AIGROUP:
        //case TYPEID_AREATRIGGER:
        //break;
        default:                                            //know type
            sLog.outDetail("Unknown Object Type %u Create Update Block.\n", m_objectTypeId);
            break;
    }

    UpdateMask updateMask;
    updateMask.SetCount( m_valuesCount );
    _SetCreateBits( &updateMask, target );
    _BuildValuesUpdate(updatetype, &buf, &updateMask, target );
    data->AddUpdateBlock(buf);
}

void Object::BuildUpdate(UpdateDataMapType &update_players)
{
    ObjectAccessor::_buildUpdateObject(this,update_players);
    ClearUpdateMask(true);
}

void Object::SendUpdateToPlayer(Player* player)
{
    // send update to another players
    SendUpdateObjectToAllExcept(player);

    // send create update to player
    UpdateData upd;
    WorldPacket packet;

    upd.Clear();
    BuildCreateUpdateBlockForPlayer(&upd, player);
    upd.BuildPacket(&packet);
    player->GetSession()->SendPacket(&packet);

    // now object updated/(create updated)
}

void Object::BuildValuesUpdateBlockForPlayer(UpdateData *data, Player *target) const
{
    if(!target)
        return;

    ByteBuffer buf(500);

    buf << (uint8)UPDATETYPE_VALUES;
    buf << (uint8)0xFF << GetGUID();

    UpdateMask updateMask;
    updateMask.SetCount( m_valuesCount );

    _SetUpdateBits( &updateMask, target );
    _BuildValuesUpdate(UPDATETYPE_VALUES, &buf, &updateMask, target);

    data->AddUpdateBlock(buf);
}

void Object::BuildOutOfRangeUpdateBlock(UpdateData * data) const
{
    data->AddOutOfRangeGUID(GetGUID());
}

void Object::DestroyForPlayer(Player *target) const
{
    ASSERT(target);

    WorldPacket data(SMSG_DESTROY_OBJECT, 8);
    data << GetGUID();
    target->GetSession()->SendPacket( &data );
}

void Object::_BuildMovementUpdate(ByteBuffer * data, uint8 flags, uint32 flags2 ) const
{
    *data << (uint8)flags;
    if( m_objectTypeId==TYPEID_PLAYER )
    {
        if(((Player*)this)->GetTransport())
        {
            flags2 |= 0x02000000;
        }
        *data << (uint32)flags2;

        *data << (uint32)getMSTime();

        if (!((Player *)this)->GetTransport())
        {
            *data << ((Player *)this)->GetPositionX();
            *data << ((Player *)this)->GetPositionY();
            *data << ((Player *)this)->GetPositionZ();
            *data << ((Player *)this)->GetOrientation();
        }
        else
        {
            //*data << ((Player *)this)->m_transport->GetPositionX() + (float)((Player *)this)->m_transX;
            //*data << ((Player *)this)->m_transport->GetPositionY() + (float)((Player *)this)->m_transY;
            //*data << ((Player *)this)->m_transport->GetPositionZ() + (float)((Player *)this)->m_transZ;

            *data << ((Player *)this)->GetTransport()->GetPositionX();
            *data << ((Player *)this)->GetTransport()->GetPositionY();
            *data << ((Player *)this)->GetTransport()->GetPositionZ();
            *data << ((Player *)this)->GetTransport()->GetOrientation();

            *data << (uint64)(((Player *)this)->GetTransport()->GetGUID());
            *data << ((Player *)this)->GetTransOffsetX();
            *data << ((Player *)this)->GetTransOffsetY();
            *data << ((Player *)this)->GetTransOffsetZ();
            *data << ((Player *)this)->GetTransOffsetO();
        }

        *data << (float)0;

        if(flags2 & 0x2000)                                 //update self
        {
            *data << (float)0;
            *data << (float)1.0;
            *data << (float)0;
            *data << (float)0;
        }
        *data << ((Player*)this)->GetSpeed( MOVE_WALK );
        *data << ((Player*)this)->GetSpeed( MOVE_RUN );
        *data << ((Player*)this)->GetSpeed( MOVE_SWIM_BACK );
        *data << ((Player*)this)->GetSpeed( MOVE_SWIM );
        *data << ((Player*)this)->GetSpeed( MOVE_RUN_BACK );
        *data << ((Player*)this)->GetSpeed( MOVE_TURN_RATE );
    }
    if( m_objectTypeId==TYPEID_UNIT )
    {
        *data << (uint32)flags2;
        *data << (uint32)0xB5771D7F;
        *data << ((Unit *)this)->GetPositionX();
        *data << ((Unit *)this)->GetPositionY();
        *data << ((Unit *)this)->GetPositionZ();
        *data << ((Unit *)this)->GetOrientation();
        *data << (float)0;
        *data << ((Creature*)this)->GetSpeed( MOVE_WALK );
        *data << ((Creature*)this)->GetSpeed( MOVE_RUN );
        *data << ((Creature*)this)->GetSpeed( MOVE_SWIM_BACK );
        *data << ((Creature*)this)->GetSpeed( MOVE_SWIM );
        *data << ((Creature*)this)->GetSpeed( MOVE_RUN_BACK );
        *data << ((Creature*)this)->GetSpeed( MOVE_TURN_RATE );
        uint8 PosCount=0;
        if(flags2 & 0x400000)
        {
            *data << (uint32)0x0;
            *data << (uint32)0x659;
            *data << (uint32)0xB7B;
            *data << (uint32)0xFDA0B4;
            *data << (uint32)PosCount;
            for(int i=0;i<PosCount+1;i++)
            {
                *data << (float)0;                          //x
                *data << (float)0;                          //y
                *data << (float)0;                          //z
            }
        }
    }
    if( (m_objectTypeId==TYPEID_CORPSE) || (m_objectTypeId==TYPEID_GAMEOBJECT) || (m_objectTypeId==TYPEID_DYNAMICOBJECT))
    {
        if(GUID_HIPART(GetGUID()) != HIGHGUID_TRANSPORT)
        {
            *data << ((WorldObject *)this)->GetPositionX();
            *data << ((WorldObject *)this)->GetPositionY();
            *data << ((WorldObject *)this)->GetPositionZ();
        }
        else
        {
            *data << (uint32)0;
            *data << (uint32)0;
            *data << (uint32)0;
        }
        *data << ((WorldObject *)this)->GetOrientation();
    }

    *data << (uint32)0x1;

    if ((GUID_HIPART(GetGUID()) == HIGHGUID_TRANSPORT))
    {
        uint32 updT = (uint32)getMSTime();
        *data << (uint32)updT;
    }

    if(  GUID_HIPART(GetGUID()) == HIGHGUID_PLAYER_CORPSE)
        *data << (uint32)0xBD38BA14;                        //fix me
}


void Object::_BuildValuesUpdate(uint8 updatetype, ByteBuffer * data, UpdateMask *updateMask, Player *target) const
{
    WPAssert(updateMask && updateMask->GetCount() == m_valuesCount);

    *data << (uint8)updateMask->GetBlockCount();
    data->append(updateMask->GetMask(), updateMask->GetLength());

    bool IsActivateToQuest = false;
    if (updatetype == UPDATETYPE_CREATE_OBJECT || updatetype == UPDATETYPE_CREATE_OBJECT2)
    {
        if (isType(TYPEMASK_GAMEOBJECT) && !((GameObject*)this)->IsTransport())
        {
            if ( ((GameObject*)this)->ActivateToQuest(target) || target->isGameMaster())
            {
                if(GetUInt32Value(GAMEOBJECT_FLAGS))
                {
                    IsActivateToQuest = true;
                    updateMask->SetBit(GAMEOBJECT_FLAGS);
                }
            }
            if (GetUInt32Value(GAMEOBJECT_ARTKIT))
                updateMask->SetBit(GAMEOBJECT_ARTKIT);
        }
    }

    // 3 specialized loops for speed optimization in non-unit/non-go case
    if(isType(TYPEMASK_UNIT))                                   // unit (creature/player) case
    {
        for( uint16 index = 0; index < m_valuesCount; index ++ )
        {
            if( updateMask->GetBit( index ) )
            {
                // Some values at server stored in float format but must be send to client in uint32 format
                if(                                         // unit fields
                    index >= UNIT_FIELD_POWER1         && index <= UNIT_FIELD_MAXPOWER5 ||
                    index >= UNIT_FIELD_BASEATTACKTIME && index <= UNIT_FIELD_RANGEDATTACKTIME ||
                    index >= UNIT_FIELD_STAT0            && index <= UNIT_FIELD_RESISTANCES + 6 ||
                    //player fields (2 comparison preferred instead redundant isType(TYPE_PLAYER) check
                    index >= PLAYER_FIELD_POSSTAT0     && index <= PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE + 6 )
                {
                    // convert from float to uint32 and send
                    *data << uint32(m_floatValues[ index ]);
                }
                // use modelid_a if not gm, _h if gm for CREATURE_FLAG_EXTRA_TRIGGER creatures
                else if(index == UNIT_FIELD_DISPLAYID && GetTypeId() == TYPEID_UNIT)
                {
                    const CreatureInfo* cinfo = ((Creature*)this)->GetCreatureInfo();
                    if(cinfo->flags_extra & CREATURE_FLAG_EXTRA_TRIGGER)
                    {
                        if(target->isGameMaster())
                        {
                            if(cinfo->Modelid_A2)
                                *data << cinfo->Modelid_A1;
                            else
                                *data << 17519; // world invisible trigger's model
                        }
                        else
                        {
                            if(cinfo->Modelid_A2)
                                *data << cinfo->Modelid_A2;
                            else
                                *data << 11686; // world invisible trigger's model
                        }
                    }
                    else
                        *data << m_uint32Values[ index ];
                }
                // hide lootable animation for unallowed players
                else if(index == UNIT_DYNAMIC_FLAGS && GetTypeId() == TYPEID_UNIT)
                {
                    if(!target->isAllowedToLoot((Creature*)this))
                        *data << (m_uint32Values[ index ] & ~UNIT_DYNFLAG_LOOTABLE);
                    else
                        *data << (m_uint32Values[ index ] & ~UNIT_DYNFLAG_OTHER_TAGGER);
                }
                else
                {
                    // send in current format (float as float, uint32 as uint32)
                    *data << m_uint32Values[ index ];
                }
            }
        }
    }
    else if(isType(TYPEMASK_GAMEOBJECT))                    // gameobject case
    {
        for( uint16 index = 0; index < m_valuesCount; index ++ )
        {
            if( updateMask->GetBit( index ) )
            {
                // send in current format (float as float, uint32 as uint32)
                if ( index == GAMEOBJECT_FLAGS )
                {
                    if(IsActivateToQuest)
                        *data << (m_uint32Values[ index ] & ~GO_FLAG_INTERACT_COND);
                    else 
                        *data << m_uint32Values[ index ];
                }
                else
                    *data << m_uint32Values[ index ];       // other cases
            }
        }
    }
    else                                                    // other objects case (no special index checks)
    {
        for( uint16 index = 0; index < m_valuesCount; index ++ )
        {
            if( updateMask->GetBit( index ) )
            {
                // send in current format (float as float, uint32 as uint32)
                *data << m_uint32Values[ index ];
            }
        }
    }
}


void Object::ClearUpdateMask(bool remove)
{
    for( uint16 index = 0; index < m_valuesCount; index ++ )
    {
        if(m_uint32Values_mirror[index]!= m_uint32Values[index])
            m_uint32Values_mirror[index] = m_uint32Values[index];
    }
    if(m_objectUpdated)
    {
        if(remove)
            ObjectAccessor::Instance().RemoveUpdateObject(this);
        m_objectUpdated = false;
    }
}

// Send current value fields changes to all viewers
void Object::SendUpdateObjectToAllExcept(Player* exceptPlayer)
{
    // changes will be send in create packet
    if(!IsInWorld())
        return;

    // nothing do
    if(!m_objectUpdated)
        return;

    ObjectAccessor::UpdateObject(this,exceptPlayer);
}

bool Object::LoadValues(const char* data)
{
    if(!m_uint32Values) _InitValues();

    Tokens tokens = StrSplit(data, " ");

    if(tokens.size() != m_valuesCount)
        return false;

    Tokens::iterator iter;
    int index;
    for (iter = tokens.begin(), index = 0; index < m_valuesCount; ++iter, ++index)
    {
        m_uint32Values[index] = atol((*iter).c_str());
    }

    return true;
}

void Object::_SetUpdateBits(UpdateMask *updateMask, Player* /*target*/) const
{
    for( uint16 index = 0; index < m_valuesCount; index ++ )
    {
        if(m_uint32Values_mirror[index]!= m_uint32Values[index])
            updateMask->SetBit(index);
    }
}

void Object::_SetCreateBits(UpdateMask *updateMask, Player* /*target*/) const
{
    for( uint16 index = 0; index < m_valuesCount; index++ )
    {
        if(GetUInt32Value(index) != 0)
            updateMask->SetBit(index);
    }
}

void Object::SetInt32Value( uint16 index, int32 value )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );

    if(m_int32Values[ index ] != value)
    {
        m_int32Values[ index ] = value;

        if(m_inWorld)
        {
            if(!m_objectUpdated)
            {
                ObjectAccessor::Instance().AddUpdateObject(this);
                m_objectUpdated = true;
            }
        }
    }
}

void Object::SetUInt32Value( uint16 index, uint32 value )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );

    if(m_uint32Values[ index ] != value)
    {
        m_uint32Values[ index ] = value;

        if(m_inWorld)
        {
            if(!m_objectUpdated)
            {
                ObjectAccessor::Instance().AddUpdateObject(this);
                m_objectUpdated = true;
            }
        }
    }
}

void Object::SetUInt64Value( uint16 index, const uint64 &value )
{
    ASSERT( index + 1 < m_valuesCount || PrintIndexError( index , true ) );
    if(*((uint64*)&(m_uint32Values[ index ])) != value)
    {
        m_uint32Values[ index ] = *((uint32*)&value);
        m_uint32Values[ index + 1 ] = *(((uint32*)&value) + 1);

        if(m_inWorld)
        {
            if(!m_objectUpdated)
            {
                ObjectAccessor::Instance().AddUpdateObject(this);
                m_objectUpdated = true;
            }
        }
    }
}

void Object::SetFloatValue( uint16 index, float value )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) ); //[[TZERO] : not sure of it ] ASSERT( index < m_valuesCount || PrintIndexError( index , true ) ); 

    if(m_floatValues[ index ] != value)
    {
        m_floatValues[ index ] = value;

        if(m_inWorld)
        {
            if(!m_objectUpdated)
            {
                ObjectAccessor::Instance().AddUpdateObject(this);
                m_objectUpdated = true;
            }
        }
    }
}

void Object::SetByteValue( uint16 index, uint8 offset, uint8 value )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );

    if(offset > 4)
    {
        sLog.outError("Object::SetByteValue: wrong offset %u", offset);
        return;
    }

    if(uint8(m_uint32Values[ index ] >> (offset * 8)) != value)
    {
        m_uint32Values[ index ] &= ~uint32(uint32(0xFF) << (offset * 8));
        m_uint32Values[ index ] |= uint32(uint32(value) << (offset * 8));

        if(m_inWorld)
        {
            if(!m_objectUpdated)
            {
                ObjectAccessor::Instance().AddUpdateObject(this);
                m_objectUpdated = true;
            }
        }
    }
}

void Object::SetUInt16Value( uint16 index, uint8 offset, uint16 value )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );

    if(offset > 2)
    {
        sLog.outError("Object::SetUInt16Value: wrong offset %u", offset);
        return;
    }

    if(uint8(m_uint32Values[ index ] >> (offset * 16)) != value)
    {
        m_uint32Values[ index ] &= ~uint32(uint32(0xFFFF) << (offset * 16));
        m_uint32Values[ index ] |= uint32(uint32(value) << (offset * 16));

        if(m_inWorld)
        {
            if(!m_objectUpdated)
            {
                ObjectAccessor::Instance().AddUpdateObject(this);
                m_objectUpdated = true;
            }
        }
    }
}

void Object::SetStatFloatValue( uint16 index, float value)
{
    if(value < 0)
        value = 0.0f;

    SetFloatValue(index, value);
}

void Object::SetStatInt32Value( uint16 index, int32 value)
{
    if(value < 0)
        value = 0;

    SetUInt32Value(index, uint32(value));
}

void Object::ApplyModUInt32Value(uint16 index, int32 val, bool apply)
{
    int32 cur = GetUInt32Value(index);
    cur += (apply ? val : -val);
    if(cur < 0)
        cur = 0;
    SetUInt32Value(index,cur);
}

void Object::ApplyModInt32Value(uint16 index, int32 val, bool apply)
{
    int32 cur = GetInt32Value(index);
    cur += (apply ? val : -val);
    SetInt32Value(index,cur);
}

void Object::ApplyModSignedFloatValue(uint16 index, float  val, bool apply)
{
    float cur = GetFloatValue(index);
    cur += (apply ? val : -val);
    SetFloatValue(index,cur);
}

void Object::ApplyModPositiveFloatValue(uint16 index, float  val, bool apply)
{
    float cur = GetFloatValue(index);
    cur += (apply ? val : -val);
    if(cur < 0)
        cur = 0;
    SetFloatValue(index,cur);
}

void Object::SetFlag( uint16 index, uint32 newFlag )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );
    uint32 oldval = m_uint32Values[ index ];
    uint32 newval = oldval | newFlag;

    if(oldval != newval)
    {
        m_uint32Values[ index ] = newval;

        if(m_inWorld)
        {
            if(!m_objectUpdated)
            {
                ObjectAccessor::Instance().AddUpdateObject(this);
                m_objectUpdated = true;
            }
        }
    }
}

void Object::RemoveFlag( uint16 index, uint32 oldFlag )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );
    uint32 oldval = m_uint32Values[ index ];
    uint32 newval = oldval & ~oldFlag;

    if(oldval != newval)
    {
        m_uint32Values[ index ] = newval;

        if(m_inWorld)
        {
            if(!m_objectUpdated)
            {
                ObjectAccessor::Instance().AddUpdateObject(this);
                m_objectUpdated = true;
            }
        }
    }
}

void Object::SetByteFlag( uint16 index, uint8 offset, uint8 newFlag )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );

    if(offset > 4)
    {
        sLog.outError("Object::SetByteFlag: wrong offset %u", offset);
        return;
    }

    if(!(uint8(m_uint32Values[ index ] >> (offset * 8)) & newFlag))
    {
        m_uint32Values[ index ] |= uint32(uint32(newFlag) << (offset * 8));

        if(m_inWorld)
        {
            if(!m_objectUpdated)
            {
                ObjectAccessor::Instance().AddUpdateObject(this);
                m_objectUpdated = true;
            }
        }
    }
}

void Object::RemoveByteFlag( uint16 index, uint8 offset, uint8 oldFlag )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );

    if(offset > 4)
    {
        sLog.outError("Object::RemoveByteFlag: wrong offset %u", offset);
        return;
    }

    if(uint8(m_uint32Values[ index ] >> (offset * 8)) & oldFlag)
    {
        m_uint32Values[ index ] &= ~uint32(uint32(oldFlag) << (offset * 8));

        if(m_inWorld)
        {
            if(!m_objectUpdated)
            {
                ObjectAccessor::Instance().AddUpdateObject(this);
                m_objectUpdated = true;
            }
        }
    }
}

bool Object::PrintIndexError(uint32 index, bool set) const
{
    sLog.outError("Attempt %s non-existed value field: %u (count: %u) for object typeid: %u type mask: %u",(set ? "set value to" : "get value from"),index,m_valuesCount,GetTypeId(),m_objectType);

    // assert must fail after function call
    return false;
}

WorldObject::WorldObject()
{
    m_positionX         = 0.0f;
    m_positionY         = 0.0f;
    m_positionZ         = 0.0f;
    m_orientation       = 0.0f;

    m_mapId             = 0;
    m_InstanceId        = 0;
    m_map               = NULL;

    m_name = "";

    mSemaphoreTeleport  = false;

    m_isActive          = false;
    IsTempWorldObject   = false;
}

void WorldObject::SetWorldObject(bool on)
{
    if(!IsInWorld())
        return;
    
    GetMap()->AddObjectToSwitchList(this, on); 
}

void WorldObject::setActive( bool on )
{
    if(m_isActive == on)
        return;

    if(GetTypeId() == TYPEID_PLAYER)
        return;

    m_isActive = on;

    if(!IsInWorld())
        return;

    Map *map = FindMap();
    if(!map)
        return;

    if(on)
    {
        if(GetTypeId() == TYPEID_UNIT)
            map->AddToActive((Creature*)this);
        else if(GetTypeId() == TYPEID_DYNAMICOBJECT)
            map->AddToActive((DynamicObject*)this);
    }
    else
    {
        if(GetTypeId() == TYPEID_UNIT)
            map->RemoveFromActive((Creature*)this);
        else if(GetTypeId() == TYPEID_DYNAMICOBJECT)
            map->RemoveFromActive((DynamicObject*)this);
    }
}

void WorldObject::_Create( uint32 guidlow, HighGuid guidhigh, uint32 mapid )
{
    Object::_Create(guidlow, 0, guidhigh);

    m_mapId = mapid;
}

uint32 WorldObject::GetZoneId() const
{
    return MapManager::Instance().GetBaseMap(m_mapId)->GetZoneId(m_positionX,m_positionY,m_positionZ);
}

uint32 WorldObject::GetAreaId() const
{
    return MapManager::Instance().GetBaseMap(m_mapId)->GetAreaId(m_positionX,m_positionY,m_positionZ);
}

InstanceData* WorldObject::GetInstanceData()
{
    Map *map = GetMap();
    return map->IsDungeon() ? ((InstanceMap*)map)->GetInstanceData() : NULL;
}

                                                            //slow
float WorldObject::GetDistance(const WorldObject* obj) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float dz = GetPositionZ() - obj->GetPositionZ();
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy) + (dz*dz)) - sizefactor;
    return ( dist > 0 ? dist : 0);
}

float WorldObject::GetDistance2d(float x, float y) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float sizefactor = GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy)) - sizefactor;
    return ( dist > 0 ? dist : 0);
}

float WorldObject::GetExactDistance2d(const float x, const float y) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    return sqrt((dx*dx) + (dy*dy));
}

float WorldObject::GetDistance(const float x, const float y, const float z) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float dz = GetPositionZ() - z;
    float sizefactor = GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy) + (dz*dz)) - sizefactor;
    return ( dist > 0 ? dist : 0);
}

float WorldObject::GetDistanceSq(const float &x, const float &y, const float &z) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float dz = GetPositionZ() - z;
    return dx*dx + dy*dy + dz*dz;
}

float WorldObject::GetDistance2d(const WorldObject* obj) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy)) - sizefactor;
    return ( dist > 0 ? dist : 0);
}

float WorldObject::GetDistanceZ(const WorldObject* obj) const
{
    float dz = fabs(GetPositionZ() - obj->GetPositionZ());
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float dist = dz - sizefactor;
    return ( dist > 0 ? dist : 0);
}

bool WorldObject::_IsWithinDist(WorldObject const* obj, float dist2compare, bool is3D) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float distsq = dx*dx + dy*dy;
    if(is3D)
    {
        float dz = GetPositionZ() - obj->GetPositionZ();
        distsq += dz*dz;
    }
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float maxdist = dist2compare + sizefactor;

    return distsq < maxdist * maxdist;
}

bool WorldObject::IsWithinLOSInMap(const WorldObject* obj) const
{
    if (!IsInMap(obj)) return false;
    float ox,oy,oz;
    obj->GetPosition(ox,oy,oz);
    return(IsWithinLOS(ox, oy, oz ));
}

bool WorldObject::IsWithinLOS(const float ox, const float oy, const float oz ) const
{
    float x,y,z;
    GetPosition(x,y,z);
    VMAP::IVMapManager *vMapManager = VMAP::VMapFactory::createOrGetVMapManager();
    return vMapManager->isInLineOfSight(GetMapId(), x, y, z+2.0f, ox, oy, oz+2.0f);
}

bool WorldObject::GetDistanceOrder(WorldObject const* obj1, WorldObject const* obj2, bool is3D /* = true */) const
{
    float dx1 = GetPositionX() - obj1->GetPositionX();
    float dy1 = GetPositionY() - obj1->GetPositionY();
    float distsq1 = dx1*dx1 + dy1*dy1;
    if(is3D)
    {
        float dz1 = GetPositionZ() - obj1->GetPositionZ();
        distsq1 += dz1*dz1;
    }

    float dx2 = GetPositionX() - obj2->GetPositionX();
    float dy2 = GetPositionY() - obj2->GetPositionY();
    float distsq2 = dx2*dx2 + dy2*dy2;
    if(is3D)
    {
        float dz2 = GetPositionZ() - obj2->GetPositionZ();
        distsq2 += dz2*dz2;
    }

    return distsq1 < distsq2;
}

bool WorldObject::IsInRange(WorldObject const* obj, float minRange, float maxRange, bool is3D /* = true */) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float distsq = dx*dx + dy*dy;
    if(is3D)
    {
        float dz = GetPositionZ() - obj->GetPositionZ();
        distsq += dz*dz;
    }

    float sizefactor = GetObjectSize() + obj->GetObjectSize();

    // check only for real range
    if(minRange > 0.0f)
    {
        float mindist = minRange + sizefactor;
        if(distsq < mindist * mindist)
            return false;
    }

    float maxdist = maxRange + sizefactor;
    return distsq < maxdist * maxdist;
}

bool WorldObject::IsInRange2d(float x, float y, float minRange, float maxRange) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float distsq = dx*dx + dy*dy;

    float sizefactor = GetObjectSize();

    // check only for real range
    if(minRange > 0.0f)
    {
        float mindist = minRange + sizefactor;
        if(distsq < mindist * mindist)
            return false;
    }

    float maxdist = maxRange + sizefactor;
    return distsq < maxdist * maxdist;
}

bool WorldObject::IsInRange3d(float x, float y, float z, float minRange, float maxRange) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float dz = GetPositionZ() - z;
    float distsq = dx*dx + dy*dy + dz*dz;

    float sizefactor = GetObjectSize();

    // check only for real range
    if(minRange > 0.0f)
    {
        float mindist = minRange + sizefactor;
        if(distsq < mindist * mindist)
            return false;
    }

    float maxdist = maxRange + sizefactor;
    return distsq < maxdist * maxdist;
}

float WorldObject::GetAngle(const WorldObject* obj) const
{
    if(!obj) return 0;
    return GetAngle( obj->GetPositionX(), obj->GetPositionY() );
}

// Return angle in range 0..2*pi
float WorldObject::GetAngle( const float x, const float y ) const
{
    float dx = x - GetPositionX();
    float dy = y - GetPositionY();

    float ang = atan2(dy, dx);
    ang = (ang >= 0) ? ang : 2 * M_PI + ang;
    return ang;
}

bool WorldObject::HasInArc(const float arcangle, const WorldObject* obj) const
{
    // always have self in arc
    if(obj == this)
        return true;

    float arc = arcangle;

    // move arc to range 0.. 2*pi
    while( arc >= 2.0f * M_PI )
        arc -=  2.0f * M_PI;
    while( arc < 0 )
        arc +=  2.0f * M_PI;

    float angle = GetAngle( obj );
    angle -= m_orientation;

    // move angle to range -pi ... +pi
    while( angle > M_PI)
        angle -= 2.0f * M_PI;
    while(angle < -M_PI)
        angle += 2.0f * M_PI;

    float lborder =  -1 * (arc/2.0f);                       // in range -pi..0
    float rborder = (arc/2.0f);                             // in range 0..pi
    return (( angle >= lborder ) && ( angle <= rborder ));
}

void WorldObject::GetRandomPoint( float x, float y, float z, float distance, float &rand_x, float &rand_y, float &rand_z) const
{
    if(distance==0)
    {
        rand_x = x;
        rand_y = y;
        rand_z = z;
        return;
    }

    // angle to face `obj` to `this`
    float angle = rand_norm()*2*M_PI;
    float new_dist = rand_norm()*distance;

    rand_x = x + new_dist * cos(angle);
    rand_y = y + new_dist * sin(angle);
    rand_z = z;

    Neo::NormalizeMapCoord(rand_x);
    Neo::NormalizeMapCoord(rand_y);
    UpdateGroundPositionZ(rand_x,rand_y,rand_z);            // update to LOS height if available
}

void WorldObject::UpdateGroundPositionZ(float x, float y, float &z) const
{
    float new_z = MapManager::Instance().GetBaseMap(GetMapId())->GetHeight(x,y,z,true);
    if(new_z > INVALID_HEIGHT)
        z = new_z+ 0.05f;                                   // just to be sure that we are not a few pixel under the surface
}

bool WorldObject::IsPositionValid() const
{
    return Neo::IsValidMapCoord(m_positionX,m_positionY,m_positionZ,m_orientation);
}

void WorldObject::MonsterSay(const char* text, uint32 language, uint64 TargetGuid)
{
    WorldPacket data(SMSG_MESSAGECHAT, 200);
    BuildMonsterChat(&data,CHAT_MSG_MONSTER_SAY,text,language,GetName(),TargetGuid);
    SendMessageToSetInRange(&data,sWorld.getConfig(CONFIG_LISTEN_RANGE_SAY),true);
}

void WorldObject::MonsterYell(const char* text, uint32 language, uint64 TargetGuid)
{
    WorldPacket data(SMSG_MESSAGECHAT, 200);
    BuildMonsterChat(&data,CHAT_MSG_MONSTER_YELL,text,language,GetName(),TargetGuid);
    SendMessageToSetInRange(&data,sWorld.getConfig(CONFIG_LISTEN_RANGE_YELL),true);
}

void WorldObject::MonsterTextEmote(const char* text, uint64 TargetGuid, bool IsBossEmote)
{
    WorldPacket data(SMSG_MESSAGECHAT, 200);
    BuildMonsterChat(&data,IsBossEmote ? CHAT_MSG_RAID_BOSS_EMOTE : CHAT_MSG_MONSTER_EMOTE,text,LANG_UNIVERSAL,GetName(),TargetGuid);
    SendMessageToSetInRange(&data,sWorld.getConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE),true);
}

void WorldObject::MonsterWhisper(const char* text, uint64 receiver, bool IsBossWhisper)
{
    Player *player = objmgr.GetPlayer(receiver);
    if(!player || !player->GetSession())
        return;

    WorldPacket data(SMSG_MESSAGECHAT, 200);
    BuildMonsterChat(&data,IsBossWhisper ? CHAT_MSG_RAID_BOSS_WHISPER : CHAT_MSG_MONSTER_SAY,text,LANG_UNIVERSAL,GetName(),receiver); // CHAT_MSG_MONSTER_SAY instead CHAT_MSG_MONSTER_WHISPER [tbc ? ]

    player->GetSession()->SendPacket(&data);
}

void WorldObject::SendPlaySound(uint32 Sound, bool OnlySelf)
{
    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << Sound;
    if (OnlySelf && GetTypeId() == TYPEID_PLAYER )
        ((Player*)this)->GetSession()->SendPacket( &data );
    else
        SendMessageToSet( &data, true ); // ToSelf ignored in this case
}

void Object::ForceValuesUpdateAtIndex(uint32 i)
{
    m_uint32Values_mirror[i] = GetUInt32Value(i) + 1; // makes server think the field changed
    if(m_inWorld)
    {
        if(!m_objectUpdated)
        {
            ObjectAccessor::Instance().AddUpdateObject(this);
            m_objectUpdated = true;
        }
    }
}

namespace Neo
{
    class MessageChatLocaleCacheDo
    {
        public:
            MessageChatLocaleCacheDo(WorldObject const& obj, ChatMsg msgtype, int32 textId, uint32 language, uint64 targetGUID, float dist)
                : i_object(obj), i_msgtype(msgtype), i_textId(textId), i_language(language),
                i_targetGUID(targetGUID), i_dist(dist)
            {
            }

            ~MessageChatLocaleCacheDo()
            {
                for(int i = 0; i < i_data_cache.size(); ++i)
                    delete i_data_cache[i];
            }

            void operator()(Player* p)
            {
                // skip far away players
                if(p->GetDistance(&i_object) > i_dist)
                    return;

                uint32 loc_idx = p->GetSession()->GetSessionDbLocaleIndex();
                uint32 cache_idx = loc_idx+1;
                WorldPacket* data;

                // create if not cached yet
                if(i_data_cache.size() < cache_idx+1 || !i_data_cache[cache_idx])
                {
                    if(i_data_cache.size() < cache_idx+1)
                        i_data_cache.resize(cache_idx+1);

                    char const* text = objmgr.GetNeoString(i_textId,loc_idx);

                    data = new WorldPacket(SMSG_MESSAGECHAT, 200);

                    // TODO: i_object.GetName() also must be localized?
                    i_object.BuildMonsterChat(data,i_msgtype,text,i_language,i_object.GetNameForLocaleIdx(loc_idx),i_targetGUID);

                    i_data_cache[cache_idx] = data;
                }
                else
                    data = i_data_cache[cache_idx];

                p->SendDirectMessage(data);
            }

        private:
            WorldObject const& i_object;
            ChatMsg i_msgtype;
            int32 i_textId;
            uint32 i_language;
            uint64 i_targetGUID;
            float i_dist;
            std::vector<WorldPacket*> i_data_cache;             // 0 = default, i => i-1 locale index
    };
}                                                           // namespace Neo

void WorldObject::MonsterSay(int32 textId, uint32 language, uint64 TargetGuid)
{
    CellPair p = Neo::ComputeCellPair(GetPositionX(), GetPositionY());

    Cell cell(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    Neo::MessageChatLocaleCacheDo say_do(*this, CHAT_MSG_MONSTER_SAY, textId,language,TargetGuid,sWorld.getConfig(CONFIG_LISTEN_RANGE_SAY));
    Neo::PlayerWorker<Neo::MessageChatLocaleCacheDo> say_worker(say_do);
    TypeContainerVisitor<Neo::PlayerWorker<Neo::MessageChatLocaleCacheDo>, WorldTypeMapContainer > message(say_worker);
    CellLock<GridReadGuard> cell_lock(cell, p);
    cell_lock->Visit(cell_lock, message, *GetMap());
}

void WorldObject::MonsterYell(int32 textId, uint32 language, uint64 TargetGuid)
{
    CellPair p = Neo::ComputeCellPair(GetPositionX(), GetPositionY());

    Cell cell(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    Neo::MessageChatLocaleCacheDo say_do(*this, CHAT_MSG_MONSTER_YELL, textId,language,TargetGuid,sWorld.getConfig(CONFIG_LISTEN_RANGE_YELL));
    Neo::PlayerWorker<Neo::MessageChatLocaleCacheDo> say_worker(say_do);
    TypeContainerVisitor<Neo::PlayerWorker<Neo::MessageChatLocaleCacheDo>, WorldTypeMapContainer > message(say_worker);
    CellLock<GridReadGuard> cell_lock(cell, p);
    cell_lock->Visit(cell_lock, message, *GetMap());
}

void WorldObject::MonsterYellToZone(int32 textId, uint32 language, uint64 TargetGuid)
{
    Neo::MessageChatLocaleCacheDo say_do(*this, CHAT_MSG_MONSTER_YELL, textId,language,TargetGuid,sWorld.getConfig(CONFIG_LISTEN_RANGE_YELL));

    uint32 zoneid = GetZoneId();

    Map::PlayerList const& pList = GetMap()->GetPlayers();
    for(Map::PlayerList::const_iterator itr = pList.begin(); itr != pList.end(); ++itr)
        if(itr->getSource()->GetZoneId()==zoneid)
            say_do(itr->getSource());
}

void WorldObject::MonsterTextEmote(int32 textId, uint64 TargetGuid, bool IsBossEmote)
{
    CellPair p = Neo::ComputeCellPair(GetPositionX(), GetPositionY());

    Cell cell(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    Neo::MessageChatLocaleCacheDo say_do(*this, IsBossEmote ? CHAT_MSG_RAID_BOSS_EMOTE : CHAT_MSG_MONSTER_EMOTE, textId,LANG_UNIVERSAL,TargetGuid,sWorld.getConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE));
    Neo::PlayerWorker<Neo::MessageChatLocaleCacheDo> say_worker(say_do);
    TypeContainerVisitor<Neo::PlayerWorker<Neo::MessageChatLocaleCacheDo>, WorldTypeMapContainer > message(say_worker);
    CellLock<GridReadGuard> cell_lock(cell, p);
    cell_lock->Visit(cell_lock, message, *GetMap());
}

void WorldObject::MonsterWhisper(int32 textId, uint64 receiver, bool IsBossWhisper)
{
    Player *player = objmgr.GetPlayer(receiver);
    if(!player || !player->GetSession())
        return;

    uint32 loc_idx = player->GetSession()->GetSessionDbLocaleIndex();
    char const* text = objmgr.GetNeoString(textId,loc_idx);

    WorldPacket data(SMSG_MESSAGECHAT, 200);
    BuildMonsterChat(&data,IsBossWhisper ? CHAT_MSG_RAID_BOSS_WHISPER : CHAT_MSG_MONSTER_SAY,text,LANG_UNIVERSAL,GetName(),receiver); // CHAT_MSG_MONSTER_SAY instead CHAT_MSG_MONSTER_WHISPER [tbc ? ]

    player->GetSession()->SendPacket(&data);
}

void WorldObject::BuildMonsterChat(WorldPacket *data, uint8 msgtype, char const* text, uint32 language, char const* name, uint64 targetGuid) const
{
    bool pre = (msgtype==CHAT_MSG_MONSTER_EMOTE || msgtype==CHAT_MSG_RAID_BOSS_EMOTE);

    *data << (uint8)msgtype;
    *data << (uint32)language;
    *data << (uint64)GetGUID();
    //[TZERO]*data << (uint32)0;                                     //2.1.0
    *data << (uint32)(strlen(name)+1);
    *data << name;
    *data << (uint64)targetGuid;                            //Unit Target
    if( targetGuid && !IS_PLAYER_GUID(targetGuid) )
    {
        *data << (uint32)1;                                 // target name length
        *data << (uint8)0;                                  // target name
    }
    *data << (uint32)(strlen(text)+1+(pre?3:0));
    if(pre)
        data->append("%s ",3);
    *data << text;
    *data << (uint8)0;                                      // ChatTag
}

void WorldObject::BuildHeartBeatMsg(WorldPacket *data) const
{
    //Heartbeat message cannot be used for non-units
    if (!isType(TYPEMASK_UNIT))
        return;

    data->Initialize(MSG_MOVE_HEARTBEAT, 32);
    *data << GetGUID();
    *data << uint32(((Unit*)this)->GetUnitMovementFlags()); // movement flags
    *data << getMSTime();                                   // time
    *data << m_positionX;
    *data << m_positionY;
    *data << m_positionZ;
    *data << m_orientation;
}

void WorldObject::BuildTeleportAckMsg(WorldPacket *data, float x, float y, float z, float ang) const
{
    //TeleportAck message cannot be used for non-units
    if (!isType(TYPEMASK_UNIT))
        return;

    data->Initialize(MSG_MOVE_TELEPORT_ACK, 41);
    data->append(GetPackGUID());
    *data << uint32(0);                                     // this value increments every time
    *data << uint32(((Unit*)this)->GetUnitMovementFlags()); // movement flags
    *data << getMSTime();                                   // time
    *data << x;
    *data << y;
    *data << z;
    *data << ang;
    *data << uint32(0);
}

void WorldObject::SendMessageToSet(WorldPacket *data, bool /*fake*/, bool bToPossessor)
{
    MapManager::Instance().GetMap(m_mapId, this)->MessageBroadcast(this, data, bToPossessor);
}

void WorldObject::SendMessageToSetInRange(WorldPacket *data, float dist, bool /*bToSelf*/, bool bToPossessor)
{
    MapManager::Instance().GetMap(m_mapId, this)->MessageDistBroadcast(this, data, dist, bToPossessor);
}

void WorldObject::SendObjectDeSpawnAnim(uint64 guid)
{
    WorldPacket data(SMSG_GAMEOBJECT_DESPAWN_ANIM, 8);
    data << guid;
    SendMessageToSet(&data, true);
}

Map* WorldObject::_getMap()
{
    return m_map = MapManager::Instance().GetMap(GetMapId(), this);
}

Map* WorldObject::_findMap()
{
    return m_map = MapManager::Instance().FindMap(GetMapId(), GetInstanceId());
}

Map const* WorldObject::GetBaseMap() const
{
    return MapManager::Instance().GetBaseMap(GetMapId());
}

void WorldObject::AddObjectToRemoveList()
{
    assert(m_uint32Values);

    Map* map = FindMap();
    if(!map)
    {
        sLog.outError("Object (TypeId: %u Entry: %u GUID: %u) at attempt add to move list not have valid map (Id: %u).",GetTypeId(),GetEntry(),GetGUIDLow(),GetMapId());
        return;
    }

    map->AddObjectToRemoveList(this);
}

Creature* WorldObject::SummonCreature(uint32 id, float x, float y, float z, float ang,TempSummonType spwtype,uint32 despwtime)
{
    TemporarySummon* pCreature = new TemporarySummon(GetGUID());

    uint32 team = 0;
    if (GetTypeId()==TYPEID_PLAYER)
        team = ((Player*)this)->GetTeam();

    if (!pCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), GetMap(), id, team))
    {
        delete pCreature;
        return NULL;
    }

    if (x == 0.0f && y == 0.0f && z == 0.0f)
        GetClosePoint(x, y, z, pCreature->GetObjectSize());

    pCreature->Relocate(x, y, z, ang);

    if(!pCreature->IsPositionValid())
    {
        sLog.outError("ERROR: Creature (guidlow %d, entry %d) not summoned. Suggested coordinates isn't valid (X: %f Y: %f)",pCreature->GetGUIDLow(),pCreature->GetEntry(),pCreature->GetPositionX(),pCreature->GetPositionY());
        delete pCreature;
        return NULL;
    }

    pCreature->SetHomePosition(x, y, z, ang);
    pCreature->Summon(spwtype, despwtime);

    if(GetTypeId()==TYPEID_UNIT && ((Creature*)this)->IsAIEnabled)
        ((Creature*)this)->AI()->JustSummoned(pCreature);

    if(pCreature->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_TRIGGER && pCreature->m_spells[0])
    {
        if(GetTypeId() == TYPEID_UNIT || GetTypeId() == TYPEID_PLAYER)
            pCreature->setFaction(((Unit*)this)->getFaction());
        pCreature->CastSpell(pCreature, pCreature->m_spells[0], false, 0, 0, GetGUID());
    }

    //return the creature therewith the summoner has access to it
    return pCreature;
}

Pet* Player::SummonPet(uint32 entry, float x, float y, float z, float ang, PetType petType, uint32 duration)
{
    Pet* pet = new Pet(petType);

    if(petType == SUMMON_PET && pet->LoadPetFromDB(this, entry))
    {
        // Remove Demonic Sacrifice auras (known pet)
        Unit::AuraList const& auraClassScripts = GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
        for(Unit::AuraList::const_iterator itr = auraClassScripts.begin();itr!=auraClassScripts.end();)
        {
            if((*itr)->GetModifier()->m_miscvalue==2228)
            {
                RemoveAurasDueToSpell((*itr)->GetId());
                itr = auraClassScripts.begin();
            }
            else
                ++itr;
        }

        if(duration > 0)
            pet->SetDuration(duration);

        return NULL;
    }

    // petentry==0 for hunter "call pet" (current pet summoned if any)
    if(!entry)
    {
        delete pet;
        return NULL;
    }

    Map *map = GetMap();
    uint32 pet_number = objmgr.GeneratePetNumber();
    if(!pet->Create(objmgr.GenerateLowGuid(HIGHGUID_PET), map, entry, pet_number))
    {
        sLog.outError("no such creature entry %u", entry);
        delete pet;
        return NULL;
    }

    pet->Relocate(x, y, z, ang);

    if(!pet->IsPositionValid())
    {
        sLog.outError("ERROR: Pet (guidlow %d, entry %d) not summoned. Suggested coordinates isn't valid (X: %f Y: %f)",pet->GetGUIDLow(),pet->GetEntry(),pet->GetPositionX(),pet->GetPositionY());
        delete pet;
        return NULL;
    }

    pet->SetOwnerGUID(GetGUID());
    pet->SetCreatorGUID(GetGUID());
    pet->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, getFaction());

    // this enables pet details window (Shift+P)
    pet->GetCharmInfo()->SetPetNumber(pet_number, false);

    pet->AIM_Initialize();

    map->Add((Creature*)pet);

    pet->setPowerType(POWER_MANA);
    pet->SetUInt32Value(UNIT_NPC_FLAGS , 0);
    pet->SetUInt32Value(UNIT_FIELD_BYTES_1,0);
    pet->InitStatsForLevel(getLevel());

    switch(petType)
    {
        case GUARDIAN_PET:
        case POSSESSED_PET:
            pet->SetUInt32Value(UNIT_FIELD_FLAGS,0);
            AddGuardian(pet);
            break;
        case SUMMON_PET:
            pet->SetUInt32Value(UNIT_FIELD_BYTES_0, 2048);
            pet->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, 0);
            pet->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, 1000);
            pet->SetHealth(pet->GetMaxHealth());
            pet->SetPower(POWER_MANA, pet->GetMaxPower(POWER_MANA));
            pet->InitPetCreateSpells();
            pet->SavePetToDB(PET_SAVE_AS_CURRENT);
            SetPet(pet);
            PetSpellInitialize();
            break;
    }

    if(petType == SUMMON_PET)
    {
        // Remove Demonic Sacrifice auras (known pet)
        Unit::AuraList const& auraClassScripts = GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
        for(Unit::AuraList::const_iterator itr = auraClassScripts.begin();itr!=auraClassScripts.end();)
        {
            if((*itr)->GetModifier()->m_miscvalue==2228)
            {
                RemoveAurasDueToSpell((*itr)->GetId());
                itr = auraClassScripts.begin();
            }
            else
                ++itr;
        }
    }

    if(duration > 0)
        pet->SetDuration(duration);

    return pet;
}

GameObject* WorldObject::SummonGameObject(uint32 entry, float x, float y, float z, float ang, float rotation0, float rotation1, float rotation2, float rotation3, uint32 respawnTime)
{
    if(!IsInWorld())
        return NULL;

    GameObjectInfo const* goinfo = objmgr.GetGameObjectInfo(entry);
    if(!goinfo)
    {
        sLog.outErrorDb("Gameobject template %u not found in database!", entry);
        return NULL;
    }
    Map *map = GetMap();
    GameObject *go = new GameObject();
    if(!go->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT),entry,map,x,y,z,ang,rotation0,rotation1,rotation2,rotation3,100,1))
    {
        delete go;
        return NULL;
    }
    go->SetRespawnTime(respawnTime);
    if(GetTypeId()==TYPEID_PLAYER || GetTypeId()==TYPEID_UNIT) //not sure how to handle this
        ((Unit*)this)->AddGameObject(go);
    else
        go->SetSpawnedByDefault(false);
    map->Add(go);

    return go;
}

Creature* WorldObject::SummonTrigger(float x, float y, float z, float ang, uint32 duration, CreatureAI* (*GetAI)(Creature*))
{
    TempSummonType summonType = (duration == 0) ? TEMPSUMMON_DEAD_DESPAWN : TEMPSUMMON_TIMED_DESPAWN;
    Creature* summon = SummonCreature(WORLD_TRIGGER, x, y, z, ang, summonType, duration);
    if(!summon)
        return NULL;

    //summon->SetName(GetName());
    if(GetTypeId()==TYPEID_PLAYER || GetTypeId()==TYPEID_UNIT)
    {
        summon->setFaction(((Unit*)this)->getFaction());
        summon->SetLevel(((Unit*)this)->getLevel());
    }

    if(GetAI)
        summon->AIM_Initialize(GetAI(summon));
    return summon;
}

void WorldObject::GetNearPoint2D(float &x, float &y, float distance2d, float absAngle ) const
{
    x = GetPositionX() + (GetObjectSize() + distance2d) * cos(absAngle);
    y = GetPositionY() + (GetObjectSize() + distance2d) * sin(absAngle);

    Neo::NormalizeMapCoord(x);
    Neo::NormalizeMapCoord(y);
}

void WorldObject::GetNearPoint(WorldObject const* searcher, float &x, float &y, float &z, float searcher_size, float distance2d, float absAngle ) const
{
    GetNearPoint2D(x,y,distance2d+searcher_size,absAngle);

    z = GetPositionZ();

    UpdateGroundPositionZ(x,y,z);
}

void WorldObject::GetGroundPoint(float &x, float &y, float &z, float dist, float angle)
{
    angle += GetOrientation();
    x += dist * cos(angle);
    y += dist * sin(angle);
    Neo::NormalizeMapCoord(x);
    Neo::NormalizeMapCoord(y);
    UpdateGroundPositionZ(x, y, z);
}



