#include "stdafx.h"
#include "xrServer.h"
#include "xrServer_Objects.h"

void xrServer::Process_update(NET_Packet& P)
{
	if (!SV_Client) return;

	// while has information
	while (!P.r_eof())
	{
		// find entity
		u16				ID;
		u8				size;

		P.r_u16			(ID);
		P.r_u8			(size);
		u32	_pos		= P.r_tell();
		CSE_Abstract	*E	= game->get_entity_from_eid(ID);
		
		if (E) 
		{
			E->net_Ready	= TRUE;
			E->UPDATE_Read	(P);
			
			if ((P.r_tell() - _pos) != size) 
			{
				string16 tmp;
				CLSID2TEXT(E->m_tClassID, tmp);
				Debug.fatal(DEBUG_INFO, "Beer from the creator of '%s'; initiator: 0x%08x, r_tell() = %d, pos = %d, objectID = %d",
					tmp, SV_Client->ID.value(), P.r_tell(), _pos, E->ID);
			}
		}
		else P.r_advance	(size);
	}
}

void xrServer::Process_save(NET_Packet& P)
{
	R_ASSERT2(SV_Client, "Process_save client not found");

	// while has information
	while (!P.r_eof())
	{
		// find entity
		u16				ID;
		u16				size;

		P.r_u16(ID);
		P.r_u16(size);
		s32 _pos_start = P.r_tell();
		CSE_Abstract *E = game->get_entity_from_eid(ID);

		if (E) {
			E->net_Ready = TRUE;
			E->load(P);
		}
		else
			P.r_advance(size);
		s32				_pos_end = P.r_tell();
		s32				_size = size;
		if (_size != (_pos_end - _pos_start)) {
			Msg("! load/save mismatch, object: '%s'", E ? E->name_replace() : "unknown");
			s32			_rollback = _pos_start + _size;
			P.r_seek(_rollback);
		}
	}
}
