/*
===========================================================================
Copyright (C) 1999 - 2005, Id Software, Inc.
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2005 - 2015, ioquake3 contributors
Copyright (C) 2013 - 2015, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/
#include "g_local.h"
#include "bg_physics.h"

std::unique_ptr<physics_world_t> g_phys;

void G_Phys_Init() 
{
	Com_Printf("================================\n");
	Com_Printf("Initializing Bullet Physics\n");
	Com_Printf("================================\n");

	g_phys = Physics_Create();
	g_phys->set_gravity(g_gravity->value);
	clipMap_t const * cm = reinterpret_cast<clipMap_t const *>(CM_Get());
	g_phys->add_world(cm);

	Com_Printf("DONE\n================================================\n");
}

void G_Phys_Shutdown() 
{
	g_phys.reset();
}

void G_Phys_Frame() 
{
	if (!g_phys)
		return;

	time_t raw_time;

	g_phys->advance(time( &raw_time ) / 1000.0f);
}

void G_RunPhysicsProp(gentity_t * ent) 
{
	auto physics = ent->get_component<GEntPhysics>();

	if (ent->s.eFlags2 & EF2_PHYSICS && physics) 
	{
		qm::vec3_t new_origin = physics->object->get_origin();
		new_origin.assign_to(ent->s.origin);
		new_origin.assign_to(ent->currentOrigin);
		new_origin.assign_to(ent->s.pos.trBase);

		qm::vec3_t new_angles = physics->object->get_angles();
		new_angles.assign_to(ent->s.angles);
		new_angles.assign_to(ent->currentAngles);
		new_angles.assign_to(ent->s.apos.trBase);
	}

	gi.linkentity(ent);
}
