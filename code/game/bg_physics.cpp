/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
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
#include <btBulletDynamicsCommon.h>

#include "qcommon/cm_public.h"
#include "bg_local.h"
#include "bg_physics.h"

struct physics_world_t::impl_t 
{
	
	btBroadphaseInterface * broadphase = nullptr;
	btDefaultCollisionConfiguration * config = nullptr;
	btCollisionDispatcher * dispatch = nullptr;
	btSequentialImpulseConstraintSolver * solver = nullptr;
	btDiscreteDynamicsWorld * world = nullptr;
	
	impl_t() 
	{		
		broadphase = new btDbvtBroadphase;
		config = new btDefaultCollisionConfiguration;
		dispatch = new btCollisionDispatcher {config};
		solver = new btSequentialImpulseConstraintSolver;
		world = new btDiscreteDynamicsWorld {dispatch, broadphase, solver, config};
	}
	
	~impl_t() 
	{		
		if (world) delete world;
		if (solver) delete solver;
		if (dispatch) delete dispatch;
		if (config) delete config;
		if (broadphase) delete broadphase;
	}
	
};

physics_world_t::physics_world_t() : impl { std::make_unique<impl_t>() } {
	
}

physics_world_t::~physics_world_t() {
	
}
