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
#include "g_local.h"
#include "../qcommon/qfiles.h"

#include <btBulletDynamicsCommon.h>
#include <LinearMath/btGeometryUtil.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

/*#pragma comment( lib, "BulletCollision_debug.lib" )
#pragma comment( lib, "BulletDynamics_debug.lib" )
#pragma comment( lib, "LinearMath_debug.lib" )*/

btBroadphaseInterface* broadphase = 0;
btDefaultCollisionConfiguration* collisionConfiguration = 0;
btCollisionDispatcher* dispatcher = 0;
btSequentialImpulseConstraintSolver* solver = 0;
btDiscreteDynamicsWorld* dynamicsWorld = 0;

static int last_time = 0;

//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
#define USE_MOTIONSTATE 1

void G_InitBullet() 
{
	// Build the broadphase
	broadphase = new btDbvtBroadphase();

	// Set up the collision configuration and dispatcher
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);

	// The actual physics solver
	solver = new btSequentialImpulseConstraintSolver;

	// The world.
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0, -10, 0));
}

void G_ShudownBullet() 
{
	// Clean up behind ourselves like good little programmers
	delete dynamicsWorld;
	dynamicsWorld = 0;
	delete solver;
	solver = 0;
	delete dispatcher;
	dispatcher = 0;
	delete collisionConfiguration;
	collisionConfiguration = 0;
	delete broadphase;
	broadphase = 0;
	last_time = 0;
}

void G_RunPhysics() 
{
	float time = level.time - last_time;
	dynamicsWorld->stepSimulation(time / 1000.0f, 10);
	last_time = level.time;
}

void BT_CreateWorldBrush(btAlignedObjectArray<btVector3> &vertices) 
{
	float mass = 0.f;
	btTransform startTransform;
	//can use a shift
	startTransform.setIdentity();
	//this create an internal copy of the vertices
	btCollisionShape* shape = new btConvexHullShape(&(vertices[0].getX()), vertices.size());
	//m_demoApp->m_collisionShapes.push_back(shape);

	//m_demoApp->localCreateRigidBody(mass, startTransform,shape);

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);

	if (isDynamic)
	{
		shape->calculateLocalInertia(mass, localInertia);
	}

#ifdef USE_MOTIONSTATE
	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);

	btRigidBody* body = new btRigidBody(cInfo);
	//body->setContactProcessingThreshold(m_defaultContactProcessingThreshold);
#else
	btRigidBody* body = new btRigidBody(mass, 0, shape, localInertia);
	body->setWorldTransform(startTransform);
#endif

	dynamicsWorld->addRigidBody(body);
}

void G_RunCharacterController(vec3_t dir, btKinematicCharacterController *ch, vec3_t newPos) 
{
	// set the forward direction of the character controller
	btVector3 walkDir(dir[0], dir[1], dir[2]);
	ch->setWalkDirection(walkDir);

	btVector3 c = ch->getGhostObject()->getWorldTransform().getOrigin();
	newPos[0] = c.x();
	newPos[1] = c.y();
	newPos[2] = c.z();
}

btKinematicCharacterController* BT_CreateCharacter(float stepHeight,
	vec3_t pos, float characterHeight, float characterWidth)
{
	btPairCachingGhostObject* ghostObject = new btPairCachingGhostObject();
	btConvexShape* characterShape = new btCapsuleShape(characterWidth, characterHeight);
	btTransform trans;
	trans.setIdentity();
	btVector3 vPos(pos[0], pos[1], pos[2]);
	trans.setOrigin(vPos);
	ghostObject->setWorldTransform(trans);
	ghostObject->setCollisionShape(characterShape);
	btKinematicCharacterController *character = new btKinematicCharacterController(ghostObject,
		characterShape, stepHeight, 1);
	character->setUpAxis(2);

	dynamicsWorld->addCollisionObject(ghostObject, btBroadphaseProxy::CharacterFilter,
		btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);

	dynamicsWorld->addCharacter(character);
	dynamicsWorld->getPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());

	return character;
};

void G_LoadMap(const char *mapName) 
{
#if 0
	FILE *f = fopen(mapName, "rb");
	if (f == 0) {
		char buf[256];
		strcpy(buf, "baseqio/");
		strcat(buf, mapName);
		strcat(buf, ".bsp");
		f = fopen(buf, "rb");
	}
	if (f == 0)
		return 0;
	fseek(f, 0, SEEK_END);
	int len = ftell(f);
	rewind(f);
	byte *data = (byte*)malloc(len);
	fread(data, 1, len, f);
	fclose(f);
#else
	char buf[256];
	strcpy(buf, "maps/");
	strcat(buf, mapName);
	strcat(buf, ".bsp");
	fileHandle_t f;
	int len = gi.FS_FOpenFile(buf, &f, fsMode_t::FS_READ);
	byte *data = (byte*)malloc(len);
	gi.FS_Read(data, len, f);
	gi.FS_FCloseFile(f);
#endif

	dheader_t *h = (dheader_t*)data;
	dbrush_t *b = (dbrush_t*)(data + h->lumps[LUMP_BRUSHES].fileofs);
	dbrushside_t *sides = (dbrushside_t*)(data + h->lumps[LUMP_BRUSHSIDES].fileofs);
	dshader_t *mats = (dshader_t*)(data + h->lumps[LUMP_SHADERS].fileofs);
	dplane_t *planes = (dplane_t*)(data + h->lumps[LUMP_PLANES].fileofs);
	int numBrushes = h->lumps[LUMP_BRUSHES].filelen / sizeof(dbrush_t);

	for (int i = 0; i < numBrushes; i++, b++) 
	{
		dshader_t &m = mats[b->shaderNum];

		if ((m.contentFlags & 1) == false)
		{
			continue;
		}

		btAlignedObjectArray<btVector3> planeEquations;
		dbrushside_t *s = sides + b->firstSide;

		for (int j = 0; j < b->numSides; j++, s++) 
		{
			dplane_t &plane = planes[s->planeNum];
			btVector3 planeEq;
			planeEq.setValue(plane.normal[0], plane.normal[1], plane.normal[2]);
			planeEq[3] = -plane.dist;
			planeEquations.push_back(planeEq);
		}

		btAlignedObjectArray<btVector3>	vertices;
		btGeometryUtil::getVerticesFromPlaneEquations(planeEquations, vertices);
		BT_CreateWorldBrush(vertices);
	}

	free(data);
}

#if 0
static char const * testmodels [] = 
{
	"models/testbox.obj",
	"models/testbox2.obj"
};

static size_t const testmodels_num = sizeof(testmodels) / sizeof(char const *);

void G_TEST_PhysTestEnt(vec3_t pos) 
{
	gentity_t * physent = G_Spawn();
	physent->s.eType = ET_PROP;
	physent->contents = MASK_SOLID;
	//physent->r.svFlags |= SVF_BROADCAST;
	
	VectorCopy(pos, trans.origin);
	VectorClear(trans.angles);
	
	size_t testm_i = rand() % testmodels_num;
	
	props.mass = -1;
	props.friction = 0.5;
	props.restitution = 0.125;
	props.dampening = 0.05;
	props.actor = qfalse;
	props.kinematic = qfalse;
	props.disabled = qfalse;
	props.contents = CONTENTS_SOLID;
	props.token = physent;
	
	physent->phys = gi.Phys_Object_Create_From_Obj(gworld, testmodels[testm_i], &trans, &props, 1);
	physent->s.modelindex = G_ModelIndex(testmodels[testm_i]);

	phys_properties_t * nprops = gi.Phys_Object_Get_Properties(physent->phys);
	VectorCopy(nprops->mins, physent->mins);
	VectorCopy(nprops->maxs, physent->maxs);
	
	G_SetOrigin(physent, pos);
	gi.linkentity(physent);
}

/*QUAKED misc_model_phys (1 0 0) (-16 -16 0) (16 16 16)
"model"		.obj file to load
*/
void SP_misc_model_phys(gentity_t *ent)
{
	if (!ent || !ent->model || !ent->model[0])
	{
		Com_Error(ERR_DROP, "misc_model_phys with no model.");
		return;
	}

	const size_t len = strlen(ent->model);
	if (len < 4 || Q_stricmp(&ent->model[len - 4], ".obj") != 0)
	{
		Com_Error(ERR_DROP, "misc_model_phys model(%s) is not an obj.", ent->model);
		return;
	}

	ent->s.eType = ET_PROP;
	ent->contents = MASK_SOLID;

	VectorCopy(ent->currentOrigin, trans.origin);
	VectorClear(trans.angles);

	props.mass = -1;
	props.friction = 0.5;
	props.restitution = 0.125;
	props.dampening = 0.05;
	props.actor = qfalse;
	props.kinematic = qfalse;
	props.disabled = qfalse;
	props.contents = CONTENTS_SOLID;
	props.token = ent;

	ent->phys = gi.Phys_Object_Create_From_Obj(gworld, ent->model, &trans, &props, 1);
	ent->s.modelindex = G_ModelIndex(ent->model);

	phys_properties_t * nprops = gi.Phys_Object_Get_Properties(ent->phys);
	VectorCopy(nprops->mins, ent->mins);
	VectorCopy(nprops->maxs, ent->maxs);

	G_SetOrigin(ent, ent->currentOrigin);
	gi.linkentity(ent);
}
#endif
