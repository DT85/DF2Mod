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

phys_world_t * gworld = NULL;
static int last_time = 0;

static vec3_t nullvec = {0, 0, 0};

#define VELLERP_THRESH 30

static void vellerp(vec3_t v1, vec3_t v2, float lerp, vec3_t out) {
	out[0] = v1[0] * lerp + v2[0] * (1 - lerp);
	out[1] = v1[1] * lerp + v2[1] * (1 - lerp);
	out[2] = v1[2] * lerp + v2[2] * (1 - lerp);
}

static void g_touch_cb_do(phys_world_t * w, phys_collision_t * col, gentity_t * entThis,  gentity_t * entOther) {
	
	if (col->normal[2] < -0.707 && entThis->client->pers.lastCommand.upmove <= 0) { // 45 degrees is maximum walkable surface angle -- TODO: cvar
		
		/*
		// on floor, move velocity closer to floor's
		if (entOther == &g_entities[ENTITYNUM_WORLD]) {
			VectorClear(entThis->phys_post_target_velocity);
		} else {
			trap->Phys_Obj_Get_Linear_Velocity(entOther->phys, entThis->phys_post_target_velocity);
		}
		entThis->phys_post_do_vellerp = qtrue;
		*/
		
		entThis->client->ps.eFlags |= EF_ON_PHYS;
		entThis->client->ps.groundEntityNum = entOther->s.number;
		gi.linkentity(entThis);
		
		//trap->Print("%f\n", col->impulse);
		
		/*
		vec3_t right, apply, curvel;
		trap->Phys_Object_Get_Origin(entThis->phys, btorig);
		trap->Phys_Obj_Get_Linear_Velocity(entThis->phys, curvel);
		AngleVectors(entThis->playerState->viewangles, NULL, right, NULL);
		VectorCopy(btorig, down);
		down[2] -= 240;
		trap->Trace (&tr, btorig, entThis->r.mins, entThis->r.maxs, down, entThis->playerState->clientNum, MASK_PLAYERSOLID, qfalse, 0, 10);
		CrossProduct(tr.plane.normal, right, apply);
		VectorScale(apply, VectorLength(curvel), apply);
		trap->Print("(%f, %f, %f)\n", tr.plane.normal[0], tr.plane.normal[1], tr.plane.normal[2]);
		trap->Phys_Obj_Set_Linear_Velocity(entThis->phys, apply);
		*/
		
		/*
		phys_trace_t tr;
		vec3_t start, end, right, curvel, apply, cvn, an;
		trap->Phys_Object_Get_Origin(entThis->phys, start);
		VectorCopy(start, end);
		end[2] -= 200 + fabs(entThis->r.mins[2]);
		trap->Phys_World_Trace(gworld, start, end, &tr);
		if (tr.hit_object) {
			AngleVectors(entThis->playerState->viewangles, NULL, right, NULL);
			trap->Phys_Obj_Get_Linear_Velocity(entThis->phys, curvel);
			CrossProduct(tr.hit_normal, right, apply);
			VectorScale(apply, VectorLength(curvel), apply);
			VectorNormalize2(curvel, cvn);
			VectorNormalize2(apply, an);
			float v = DotProduct(cvn, an);
			if (v < 0) v = 0;
			vellerp(apply, curvel, v, apply);
			trap->Phys_Obj_Set_Linear_Velocity(entThis->phys, apply);
		}
		*/
	}
	
	/*
	trap->Phys_Object_Get_Origin(entThis->phys, btorig);
	VectorCopy(btorig, up);
	up[2] += STEPSIZE;
	trap->Trace (&tr, btorig, entThis->r.mins, entThis->r.maxs, up, entThis->playerState->clientNum, MASK_PLAYERSOLID, qfalse, 0, 10);
	step_size = tr.endpos[2] - btorig[2];
	VectorCopy(tr.endpos, btorig);
	
	VectorCopy (btorig, down);
	down[2] -= step_size;
	trap->Trace (&tr, btorig, entThis->r.mins, entThis->r.maxs, down, entThis->playerState->clientNum, MASK_PLAYERSOLID, qfalse, 0, 10);
	VectorCopy(tr.endpos, btorig);
	trap->Phys_Object_Set_Origin(entThis->phys, btorig);
	*/
}

static void g_touch_cb(phys_world_t * w, phys_collision_t * col) {
	
	assert(w == gworld);
	
	phys_properties_t *propsA = gi.Phys_Object_Get_Properties(col->A);
	phys_properties_t *propsB = gi.Phys_Object_Get_Properties(col->B);
	
	gentity_t *entA = (gentity_t *)propsA->token;
	gentity_t *entB = (gentity_t *)propsB->token;
	
	if (!entA || !entB) return;

	gentity_t *entClient = NULL;
	gentity_t *entOther = NULL;
	qboolean cli2 = qfalse;
	
	if (entA->s.eType == ET_PLAYER/* || entA->s.eType == ET_NPC*/) {
		entClient = entA;
		entOther = entB;
	}
	if (entB->s.eType == ET_PLAYER/* || entB->s.eType == ET_NPC*/) {
		if (entClient) cli2 = qtrue;
		entClient = entB;
		entOther = entA;
	}
	
	if (!entClient) return; // collisions not involving a client are irrelevant
	
	g_touch_cb_do(w, col, entClient, entOther);
	if (cli2) {
		g_touch_cb_do(w, col, entOther, entClient);
	}
}

void G_Phys_Init() {
	gi.Printf("================================\n");
	Com_Printf("Initializing Bullet Physics\n");
	
	gworld = gi.Phys_World_Create(g_touch_cb);
	G_Phys_Upd_Res();
	G_Phys_Upd_Grav();
	gi.Phys_World_Add_Current_Map(gworld, &g_entities[ENTITYNUM_WORLD]);
	
	gi.Printf("================================\n");
}

void G_Phys_Shutdown() {
	if (gworld) {
		gi.Phys_World_Destroy(gworld);
		gworld = NULL;
	}
	last_time = 0;
}

void G_Phys_Frame() {
	gentity_t *gent = g_entities;
	for (int i = 0; i < MAX_GENTITIES; i++, gent++) {
		//if (!gent->playerState || !gent->phys)
		if (/*!gent->s || */!gent->phys) 
			continue;

		//gent->playerState->eFlags &= ~EF_ON_PHYS;
		gent->s.eFlags &= ~EF_ON_PHYS;
	}
	
	int delta = level.time - last_time;
	gi.Phys_World_Advance(gworld, delta);
	last_time = level.time;
	
	gent = g_entities;
	for (int i = 0; i < MAX_GENTITIES; i++, gent++) {
		if (!(gent->client && gent->phys)) 
			continue;

		vec3_t btorig;
		if (gent->phys_is_crouched) {
			gi.Phys_Object_Get_Origin(gent->phys2, btorig);
			VectorCopy(btorig, gent->client->ps.origin);
			VectorCopy(btorig, gent->currentOrigin);
			gi.Phys_Obj_Get_Linear_Velocity(gent->phys2, gent->client->ps.velocity);
		} else {
			gi.Phys_Object_Get_Origin(gent->phys, btorig);
			VectorCopy(btorig, gent->client->ps.origin);
			VectorCopy(btorig, gent->currentOrigin);
			gi.Phys_Obj_Get_Linear_Velocity(gent->phys, gent->client->ps.velocity);
		}
		/*
		if (gent->phys_post_do_vellerp) {
			qboolean clip = qtrue;
			if (gent->playerState->moveDir[0] || gent->playerState->moveDir[1]) clip = qtrue;
			vellerp(gent->playerState->velocity, gent->phys_post_target_velocity, 0.95, clip, gent->playerState->velocity);
			gent->phys_post_do_vellerp = qfalse;
		}
		*/
		gi.linkentity( gent );
	}
}

void G_Phys_Upd_Res() {
	if (!gworld) 
		return;

	gi.Phys_World_Set_Resolution(gworld, g_phys_resolution->integer);
}

void G_Phys_Upd_Grav() {
	if (!gworld) 
		return;
	gi.Phys_World_Set_Gravity(gworld, g_gravity->value);
}

void G_Phys_Set_Friction(gentity_t * ent, float f) {
	if (!ent->phys) 
		return;

	phys_properties_t * props;
	props = gi.Phys_Object_Get_Properties(ent->phys);
	props->friction = f;
	gi.Phys_Object_Set_Properties(ent->phys);
	if (ent->phys2) {
		props = gi.Phys_Object_Get_Properties(ent->phys2);
		props->friction = f;
		gi.Phys_Object_Set_Properties(ent->phys2);
	}
}


static phys_transform_t trans;
static phys_properties_t props;

void G_Phys_UpdateEnt(gentity_t * ent) {
	
	phys_properties_t * props;
	phys_properties_t * props2;
	
	if (!ent->phys) 
		return;

	switch (ent->s.eType) {
	case ET_PROP:
	case ET_GENERAL:
	default:
		gi.Phys_Object_Get_Origin(ent->phys, trans.origin);
		gi.Phys_Object_Get_Rotation(ent->phys, trans.angles);
		G_SetOrigin(ent, trans.origin);
		G_SetAngles(ent, trans.angles);

		ent->s.pos.trType = TR_INTERPOLATE;
		ent->s.apos.trType = TR_INTERPOLATE;

		gi.linkentity(ent);
		break;
	case ET_PLAYER:
		props = gi.Phys_Object_Get_Properties(ent->phys);
		props2 = gi.Phys_Object_Get_Properties(ent->phys2);
		if (ent->client->noclip) {
			props->contents = 0;
			props2->contents = 0;
		} else {
			props->contents = ent->contents;
			props2->contents = ent->contents;
		}
		gi.Phys_Object_Set_Properties(ent->phys);
		gi.Phys_Object_Set_Properties(ent->phys2);
		break;
	/*case ET_NPC:
		VectorCopy(ent->currentOrigin, trans.origin);
		VectorCopy(ent->currentAngles, trans.angles);
		gi.Phys_Object_Set_Origin(ent->phys, trans.origin);
		gi.Phys_Object_Set_Rotation(ent->phys, trans.angles);
		gi.Phys_Obj_Set_Linear_Velocity(ent->phys, ent->playerState->velocity);
		gi.Phys_Object_Set_Origin(ent->phys2, trans.origin);
		gi.Phys_Object_Set_Rotation(ent->phys2, trans.angles);
		gi.Phys_Obj_Set_Linear_Velocity(ent->phys2, ent->playerState->velocity);
		break;*/
	case ET_MOVER:
		VectorCopy(ent->currentOrigin, trans.origin);
		VectorCopy(ent->currentAngles, trans.angles);
		gi.Phys_Object_Set_Origin(ent->phys, trans.origin);
		gi.Phys_Object_Set_Rotation(ent->phys, trans.angles);
		phys_properties_t * props = gi.Phys_Object_Get_Properties(ent->phys);

		if (props->contents != ent->contents) {
			props->contents = ent->contents;
			gi.Phys_Object_Set_Properties(ent->phys);
		}
		break;
	}
}

void G_Phys_AddBMover(gentity_t * mover) {
	if (!mover->bmodel) 
		return;

	int bmodi = strtol(mover->model + 1, NULL, 10);
	
	VectorClear(trans.origin);
	VectorClear(trans.angles);
	
	props.mass = 0;
	props.friction = 0.5;
	props.restitution = 0.125;
	props.dampening = 0;
	props.actor = qfalse;
	props.kinematic = qtrue;
	props.disabled = qfalse;
	props.contents = mover->contents;
	props.token = mover;
	
	mover->phys = gi.Phys_Object_Create_From_BModel(gworld, bmodi, &trans, &props);
}

void G_Phys_AddClientCapsule(gentity_t * ent) {
	if (ent->phys) gi.Phys_World_Remove_Object(gworld, ent->phys);
	if (ent->phys2) gi.Phys_World_Remove_Object(gworld, ent->phys2);
	
	//gi.Printf("%s", ent->classname);
	
	props.mass = -1;
	props.friction = bg_phys_clfric_stop->value;
	props.restitution = 0;
	props.dampening = 0;
	props.actor = qtrue;
	props.kinematic = qfalse;
	props.disabled = qfalse;
	props.contents = ent->contents;
	props.token = ent->client;
	
	VectorCopy(ent->currentOrigin, trans.origin);
	VectorClear(trans.angles);
	
	float radius = (fabs(ent->maxs[0] - ent->mins[0]) + fabs(ent->maxs[1] - ent->mins[1])) / 4;
	float cheight = fabs(ent->client->standheight - ent->mins[2]) - 2 * radius;
	float cheight2 = fabs(ent->client->crouchheight - ent->mins[2])- 2 * radius;
	float voffs = (ent->mins[2] + ent->client->standheight) / 2;
	float voffs2 = (ent->mins[2] + ent->client->crouchheight) / 2;
	
	ent->phys = gi.Phys_Object_Create_Capsule(gworld, cheight, radius, voffs, &trans, &props);
	props.disabled = qtrue;
	ent->phys2 = gi.Phys_Object_Create_Capsule(gworld, cheight2, radius, voffs2, &trans, &props);
	
	ent->phys_is_crouched = qfalse;
}

void G_Phys_SetClientCrouched(gentity_t * ent, qboolean crouched) {
	if (crouched == ent->phys_is_crouched) 
		return;
	
	phys_properties_t * props = gi.Phys_Object_Get_Properties(ent->phys);
	phys_properties_t * props2 = gi.Phys_Object_Get_Properties(ent->phys2);
	
	props->disabled = crouched;
	props2->disabled = (qboolean)!crouched;
	
	gi.Phys_Object_Set_Properties(ent->phys);
	gi.Phys_Object_Set_Properties(ent->phys2);
	
	ent->phys_is_crouched = crouched;
}

void G_Phys_Remove(gentity_t * ent) {
	if (ent->phys) gi.Phys_World_Remove_Object(gworld, ent->phys);
	if (ent->phys2) gi.Phys_World_Remove_Object(gworld, ent->phys2);
	ent->phys = NULL;
	ent->phys2 = NULL;
};

static char const * testmodels [] = {
	"models/testbox.obj",
	"models/testbox2.obj"
};
static size_t const testmodels_num = sizeof(testmodels) / sizeof(char const *);

void G_TEST_PhysTestEnt(vec3_t pos) {
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
