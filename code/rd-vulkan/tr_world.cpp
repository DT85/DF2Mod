/*
===========================================================================
Copyright (C) 1999 - 2005, Id Software, Inc.
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

#include "tr_local.h"

inline void Q_CastShort2Float( float *f, const short *s )
{
	*f = ((float)*s);
}

/*
=================
R_CullTriSurf

Returns true if the grid is completely culled away.
Also sets the clipped hint bit in tess
=================
*/
static qboolean	R_CullTriSurf( srfTriangles_t *cv ) {
	int 	boxCull;

	boxCull = R_CullLocalBox( cv->bounds );

	if ( boxCull == CULL_OUT ) {
		return qtrue;
	}
	return qfalse;
}

/*
=================
R_CullGrid

Returns true if the grid is completely culled away.
Also sets the clipped hint bit in tess
=================
*/
static qboolean	R_CullGrid( srfGridMesh_t *cv ) {
	int 	boxCull;
	int 	sphereCull;

	if ( r_nocurves->integer ) {
		return qtrue;
	}

	if ( tr.currentEntityNum != REFENTITYNUM_WORLD ) {
		sphereCull = R_CullLocalPointAndRadius( cv->localOrigin, cv->meshRadius );
	} else {
		sphereCull = R_CullPointAndRadius( cv->localOrigin, cv->meshRadius );
	}
	boxCull = CULL_OUT;

	// check for trivial reject
	if ( sphereCull == CULL_OUT )
	{
		tr.pc.c_sphere_cull_patch_out++;
		return qtrue;
	}
	// check bounding box if necessary
	else if ( sphereCull == CULL_CLIP )
	{
		tr.pc.c_sphere_cull_patch_clip++;

		boxCull = R_CullLocalBox( cv->meshBounds );

		if ( boxCull == CULL_OUT )
		{
			tr.pc.c_box_cull_patch_out++;
			return qtrue;
		}
		else if ( boxCull == CULL_IN )
		{
			tr.pc.c_box_cull_patch_in++;
		}
		else
		{
			tr.pc.c_box_cull_patch_clip++;
		}
	}
	else
	{
		tr.pc.c_sphere_cull_patch_in++;
	}

	return qfalse;
}

/*
================
R_CullSurface

Tries to back face cull surfaces before they are lighted or
added to the sorting list.

This will also allow mirrors on both sides of a model without recursion.
================
*/
static qboolean	R_CullSurface( surfaceType_t *surface, shader_t *shader ) {
	srfSurfaceFace_t *sface;
	float			d;

	if ( r_nocull->integer ) {
		return qfalse;
	}

	if ( *surface == SF_GRID ) {
		return R_CullGrid( (srfGridMesh_t *)surface );
	}

	if ( *surface == SF_TRIANGLES ) {
		return R_CullTriSurf( (srfTriangles_t *)surface );
	}

	if ( *surface != SF_FACE ) {
		return qfalse;
	}

	if ( shader->cullType == CT_TWO_SIDED ) {
		return qfalse;
	}

	// face culling
	if ( !r_facePlaneCull->integer ) {
		return qfalse;
	}

	sface = ( srfSurfaceFace_t * ) surface;

	d = DotProduct (tr.ori.viewOrigin, sface->plane.normal);

	// don't cull exactly on the plane, because there are levels of rounding
	// through the BSP, ICD, and hardware that may cause pixel gaps if an
	// epsilon isn't allowed here
	if ( shader->cullType == CT_FRONT_SIDED ) {
		if ( d < sface->plane.dist - 8 ) {
			return qtrue;
		}
	} else {
		if ( d > sface->plane.dist + 8 ) {
			return qtrue;
		}
	}

	return qfalse;
}

#ifdef USE_PMLIGHT
qboolean R_LightCullBounds( const dlight_t *dl, const vec3_t mins, const vec3_t maxs )
{
	if (dl->linear) {
		if (dl->transformed[0] - dl->radius > maxs[0] && dl->transformed2[0] - dl->radius > maxs[0])
			return qtrue;
		if (dl->transformed[0] + dl->radius < mins[0] && dl->transformed2[0] + dl->radius < mins[0])
			return qtrue;

		if (dl->transformed[1] - dl->radius > maxs[1] && dl->transformed2[1] - dl->radius > maxs[1])
			return qtrue;
		if (dl->transformed[1] + dl->radius < mins[1] && dl->transformed2[1] + dl->radius < mins[1])
			return qtrue;

		if (dl->transformed[2] - dl->radius > maxs[2] && dl->transformed2[2] - dl->radius > maxs[2])
			return qtrue;
		if (dl->transformed[2] + dl->radius < mins[2] && dl->transformed2[2] + dl->radius < mins[2])
			return qtrue;

		return qfalse;
	}

	if (dl->transformed[0] - dl->radius > maxs[0])
		return qtrue;
	if (dl->transformed[0] + dl->radius < mins[0])
		return qtrue;

	if (dl->transformed[1] - dl->radius > maxs[1])
		return qtrue;
	if (dl->transformed[1] + dl->radius < mins[1])
		return qtrue;

	if (dl->transformed[2] - dl->radius > maxs[2])
		return qtrue;
	if (dl->transformed[2] + dl->radius < mins[2])
		return qtrue;

	return qfalse;
}

static qboolean R_LightCullFace( const srfSurfaceFace_t *face, const dlight_t *dl )
{
	float d = DotProduct(dl->transformed, face->plane.normal) - face->plane.dist;
	if (dl->linear)
	{
		float d2 = DotProduct(dl->transformed2, face->plane.normal) - face->plane.dist;
		if ((d < -dl->radius) && (d2 < -dl->radius))
			return qtrue;
		if ((d > dl->radius) && (d2 > dl->radius))
			return qtrue;
	}
	else
	{
		if ((d < -dl->radius) || (d > dl->radius))
			return qtrue;
	}

	return qfalse;
}

static qboolean R_LightCullSurface( const surfaceType_t *surface, const dlight_t *dl )
{
	switch (*surface) {
	case SF_FACE:
		return R_LightCullFace((const srfSurfaceFace_t*)surface, dl);
	case SF_GRID: {
		const srfGridMesh_t* grid = (const srfGridMesh_t*)surface;
		return R_LightCullBounds(dl, grid->meshBounds[0], grid->meshBounds[1]);
	}
	case SF_TRIANGLES: {
		const srfTriangles_t* tris = (const srfTriangles_t*)surface;
		return R_LightCullBounds(dl, tris->bounds[0], tris->bounds[1]);
	}
	default:
		return qfalse;
	};
}
#endif // USE_PMLIGHT

#ifdef _ALT_AUTOMAP_METHOD
static bool tr_drawingAutoMap = false;
#endif
static float g_playerHeight = 0.0f;

/*
======================
R_AddWorldSurface
======================
*/
static void R_AddWorldSurface( msurface_t *surf, int dlightBits, qboolean noViewCount = qfalse )
{
	if ( !noViewCount ) 
	{
		if ( surf->viewCount == tr.viewCount ) {
			return;
		}

		surf->viewCount = tr.viewCount;
		// FIXME: bmodel fog?
	}

	/*
	if (r_shadows->integer == 2)
	{
		dlightBits = R_DlightSurface( surf, dlightBits );
		//dlightBits = ( dlightBits != 0 );
		R_AddDrawSurf( surf->data, tr.shadowShader, surf->fogIndex, dlightBits );
	}
	*/
	//world shadows?

	// try to cull before dlighting or adding
#ifdef _ALT_AUTOMAP_METHOD
	if (!tr_drawingAutoMap && R_CullSurface( surf->data, surf->shader ) )
#else
	if ( R_CullSurface( surf->data, surf->shader ) )
#endif
	{
		return;
	}

#ifdef USE_PMLIGHT
	{
		surf->vcVisible = tr.viewCount;
		R_AddDrawSurf( surf->data, surf->shader, surf->fogIndex, 0 );
		return;
	}
#endif // USE_PMLIGHT


#ifdef _ALT_AUTOMAP_METHOD
	if (tr_drawingAutoMap)
	{
	//	if (g_playerHeight != g_lastHeight ||
	//		!g_lastHeightValid)
		if (*surf->data == SF_FACE)
		{ //only do this if we need to
			bool completelyTransparent = true;
			int i = 0;
			srfSurfaceFace_t *face = (srfSurfaceFace_t *)surf->data;
			byte *indices = (byte *)(face + face->ofsIndices);
			float *point;
			vec3_t color;
			float alpha;
			float e;
			bool polyStarted = false;

			while (i < face->numIndices)
			{
				point = &face->points[indices[i]][0];

				//base the color on the elevation... for now, just check the first point height
				if (point[2] < g_playerHeight)
				{
					e = point[2]-g_playerHeight;
				}
				else
				{
					e = g_playerHeight-point[2];
				}
				if (e < 0.0f)
				{
					e = -e;
				}

				//set alpha and color based on relative height of point
				alpha = e/256.0f;
				e /= 512.0f;

				//cap color
				if (e > 1.0f)
				{
					e = 1.0f;
				}
				else if (e < 0.0f)
				{
					e = 0.0f;
				}
				VectorSet(color, e, 1.0f-e, 0.0f);

				//cap alpha
				if (alpha > 1.0f)
				{
					alpha = 1.0f;
				}
				else if (alpha < 0.0f)
				{
					alpha = 0.0f;
				}

				if (alpha != 1.0f)
				{ //this point is not entirely alpha'd out, so still draw the surface
					completelyTransparent = false;
				}

				if (!completelyTransparent)
				{
					if (!polyStarted)
					{
						qglBegin(GL_POLYGON);
						polyStarted = true;
					}

					qglColor4f(color[0], color[1], color[2], 1.0f-alpha);
					qglVertex3f(point[i], point[i], point[2]);
				}

				i++;
			}

			if (polyStarted)
			{
				qglEnd();
			}
		}
	}
	else
#endif
	{
		R_AddDrawSurf( surf->data, surf->shader, surf->fogIndex, dlightBits );
	}
}

/*
=============================================================
	PM LIGHTING
=============================================================
*/
#ifdef USE_PMLIGHT
static void R_AddLitSurface( msurface_t *surf, const dlight_t *light )
{
	// since we're not worried about offscreen lights casting into the frustum (ATM !!!)
	// only add the "lit" version of this surface if it was already added to the view
	//if ( surf->viewCount != tr.viewCount )
	//	return;

	// surfaces that were faceculled will still have the current viewCount in vcBSP
	// because that's set to indicate that it's BEEN vis tested at all, to avoid
	// repeated vis tests, not whether it actually PASSED the vis test or not
	// only light surfaces that are GENUINELY visible, as opposed to merely in a visible LEAF
	if (surf->vcVisible != tr.viewCount) {
		return;
	}

	if (surf->shader->lightingStage < 0) {
		return;
	}

	if (surf->lightCount == tr.lightCount)
		return;

	surf->lightCount = tr.lightCount;

	if (R_LightCullSurface(surf->data, light)) {
		tr.pc.c_lit_culls++;
		return;
	}

	R_AddLitSurf(surf->data, surf->shader, surf->fogIndex);
}


static void R_RecursiveLightNode( const mnode_t *node )
{
	qboolean	children[2];
	msurface_t	**mark;
	msurface_t	*surf;
	float d;
	int c;

	do {
		// if the node wasn't marked as potentially visible, exit
		if (node->visframe != tr.visCount)
			return;

		if (node->contents != CONTENTS_NODE)
			break;

		children[0] = children[1] = qfalse;

		d = DotProduct(tr.light->origin, node->plane->normal) - node->plane->dist;
		if (d > -tr.light->radius) {
			children[0] = qtrue;
		}
		if (d < tr.light->radius) {
			children[1] = qtrue;
		}

		if (tr.light->linear) {
			d = DotProduct(tr.light->origin2, node->plane->normal) - node->plane->dist;
			if (d > -tr.light->radius) {
				children[0] = qtrue;
			}
			if (d < tr.light->radius) {
				children[1] = qtrue;
			}
		}

		if (children[0] && children[1]) {
			R_RecursiveLightNode(node->children[0]);
			node = node->children[1];
		}
		else if (children[0]) {
			node = node->children[0];
		}
		else if (children[1]) {
			node = node->children[1];
		}
		else {
			return;
		}

	} while (1);

	tr.pc.c_lit_leafs++;

	// add the individual surfaces
	c = node->nummarksurfaces;
	mark = node->firstmarksurface;
	while (c--) {
		// the surface may have already been added if it spans multiple leafs
		surf = *mark;
		R_AddLitSurface(surf, tr.light);
		mark++;
	}
}
#endif // USE_PMLIGHT

/*
=============================================================

	BRUSH MODELS

=============================================================
*/

/*
=================
R_AddBrushModelSurfaces
=================
*/
void R_AddBrushModelSurfaces ( trRefEntity_t *ent ) {
	bmodel_t	*bmodel;
	int			clip;
	model_t		*pModel;
	dlight_t	*dl;
	int			i, s;

	pModel = R_GetModelByHandle( ent->e.hModel );

	bmodel = pModel->bmodel;

	clip = R_CullLocalBox( bmodel->bounds );
	if ( clip == CULL_OUT ) {
		return;
	}

#ifdef USE_PMLIGHT
	for ( s = 0; s < bmodel->numSurfaces; s++ ) {
		R_AddWorldSurface( bmodel->firstSurface + s, 0, qtrue );
	}

	R_SetupEntityLighting( &tr.refdef, ent );

	R_TransformDlights( tr.viewParms.num_dlights, tr.viewParms.dlights, &tr.ori );

	for ( i = 0; i < tr.viewParms.num_dlights; i++ ) {
		dl = &tr.viewParms.dlights[i];

		if ( !R_LightCullBounds( dl, bmodel->bounds[0], bmodel->bounds[1] ) ) {
			tr.lightCount++;
			tr.light = dl;

			for ( s = 0; s < bmodel->numSurfaces; s++ ) {
				R_AddLitSurface( bmodel->firstSurface + s, dl );
			}
		}
	}
#endif
}

float GetQuadArea( vec3_t v1, vec3_t v2, vec3_t v3, vec3_t v4 )
{
	vec3_t	vec1, vec2, dis1, dis2;

	// Get area of tri1
	VectorSubtract( v1, v2, vec1 );
	VectorSubtract( v1, v4, vec2 );
	CrossProduct( vec1, vec2, dis1 );
	VectorScale( dis1, 0.25f, dis1 );

	// Get area of tri2
	VectorSubtract( v3, v2, vec1 );
	VectorSubtract( v3, v4, vec2 );
	CrossProduct( vec1, vec2, dis2 );
	VectorScale( dis2, 0.25f, dis2 );

	// Return addition of disSqr of each tri area
	return ( dis1[0] * dis1[0] + dis1[1] * dis1[1] + dis1[2] * dis1[2] +
				dis2[0] * dis2[0] + dis2[1] * dis2[1] + dis2[2] * dis2[2] );
}

void RE_GetBModelVerts( int bmodelIndex, vec3_t *verts, vec3_t normal )
{
	msurface_t			*surfs;
	srfSurfaceFace_t	*face;
	bmodel_t			*bmodel;
	model_t				*pModel;
	int					i;
	//	Not sure if we really need to track the best two candidates
	int					maxDist[2]={0,0};
	int					maxIndx[2]={0,0};
	int					dist = 0;
	float				dot1, dot2;

	pModel = R_GetModelByHandle( bmodelIndex );
	bmodel = pModel->bmodel;

	// Loop through all surfaces on the brush and find the best two candidates
	for ( i = 0 ; i < bmodel->numSurfaces; i++ )
	{
		surfs = bmodel->firstSurface + i;
		face = ( srfSurfaceFace_t *)surfs->data;

		// It seems that the safest way to handle this is by finding the area of the faces
		dist = GetQuadArea( face->points[0], face->points[1], face->points[2], face->points[3] );

		// Check against the highest max
		if ( dist > maxDist[0] )
		{
			// Shuffle our current maxes down
			maxDist[1] = maxDist[0];
			maxIndx[1] = maxIndx[0];

			maxDist[0] = dist;
			maxIndx[0] = i;
		}
		// Check against the second highest max
		else if ( dist >= maxDist[1] )
		{
			// just stomp the old
			maxDist[1] = dist;
			maxIndx[1] = i;
		}
	}

	// Hopefully we've found two best case candidates.  Now we should see which of these faces the viewer
	surfs = bmodel->firstSurface + maxIndx[0];
	face = ( srfSurfaceFace_t *)surfs->data;
	dot1 = DotProduct( face->plane.normal, tr.refdef.viewaxis[0] );

	surfs = bmodel->firstSurface + maxIndx[1];
	face = ( srfSurfaceFace_t *)surfs->data;
	dot2 = DotProduct( face->plane.normal, tr.refdef.viewaxis[0] );

	if ( dot2 < dot1 && dot2 < 0.0f )
	{
		i = maxIndx[1]; // use the second face
	}
	else if ( dot1 < dot2 && dot1 < 0.0f )
	{
		i = maxIndx[0]; // use the first face
	}
	else
	{ // Possibly only have one face, so may as well use the first face, which also should be the best one
		//i = rand() & 1; // ugh, we don't know which to use.  I'd hope this would never happen
		i = maxIndx[0]; // use the first face
	}

	surfs = bmodel->firstSurface + i;
	face = ( srfSurfaceFace_t *)surfs->data;

	for ( int t = 0; t < 4; t++ )
	{
		VectorCopy(	face->points[t], verts[t] );
	}
}

/*
=============================================================

	WORLD MODEL

=============================================================
*/

//draw the automap with the given transformation matrix -rww
#define QUADINFINITY			16777216
//static float g_lastHeight = 0.0f;
//static bool g_lastHeightValid = false;
static void R_RecursiveWorldNode( mnode_t *node, int planeBits, int dlightBits );


/*
================
R_RecursiveWorldNode
================
*/
static void R_RecursiveWorldNode( mnode_t *node, int planeBits, int dlightBits ) {

	do
	{
		int			newDlights[2];

#ifdef _ALT_AUTOMAP_METHOD
		if (tr_drawingAutoMap)
		{
			node->visframe = tr.visCount;
		}
#endif

		// if the node wasn't marked as potentially visible, exit
		if (node->visframe != tr.visCount)
		{
			return;
		}

		// if the bounding volume is outside the frustum, nothing
		// inside can be visible OPTIMIZE: don't do this all the way to leafs?

#ifdef _ALT_AUTOMAP_METHOD
		if ( r_nocull->integer!=1 && !tr_drawingAutoMap )
#else
		if (r_nocull->integer!=1)
#endif
		{
			int		r;

			if ( planeBits & 1 ) {
				r = BoxOnPlaneSide(node->mins, node->maxs, &tr.viewParms.frustum[0]);
				if (r == 2) {
					return;						// culled
				}
				if ( r == 1 ) {
					planeBits &= ~1;			// all descendants will also be in front
				}
			}

			if ( planeBits & 2 ) {
				r = BoxOnPlaneSide(node->mins, node->maxs, &tr.viewParms.frustum[1]);
				if (r == 2) {
					return;						// culled
				}
				if ( r == 1 ) {
					planeBits &= ~2;			// all descendants will also be in front
				}
			}

			if ( planeBits & 4 ) {
				r = BoxOnPlaneSide(node->mins, node->maxs, &tr.viewParms.frustum[2]);
				if (r == 2) {
					return;						// culled
				}
				if ( r == 1 ) {
					planeBits &= ~4;			// all descendants will also be in front
				}
			}

			if ( planeBits & 8 ) {
				r = BoxOnPlaneSide(node->mins, node->maxs, &tr.viewParms.frustum[3]);
				if (r == 2) {
					return;						// culled
				}
				if ( r == 1 ) {
					planeBits &= ~8;			// all descendants will also be in front
				}
			}

		}

		if ( node->contents != -1 ) {
			break;
		}

		// node is just a decision point, so go down both sides
		// since we don't care about sort orders, just go positive to negative
		newDlights[0] = 0;
		newDlights[1] = 0;

		// recurse down the children, front side first
		R_RecursiveWorldNode (node->children[0], planeBits, newDlights[0] );

		// tail recurse
		node = node->children[1];

	} while ( 1 );

	{
		// leaf node, so add mark surfaces
		int			c;
		msurface_t	*surf, **mark;

		tr.pc.c_leafs++;

		// add to z buffer bounds
		if ( node->mins[0] < tr.viewParms.visBounds[0][0] ) {
			tr.viewParms.visBounds[0][0] = node->mins[0];
		}
		if ( node->mins[1] < tr.viewParms.visBounds[0][1] ) {
			tr.viewParms.visBounds[0][1] = node->mins[1];
		}
		if ( node->mins[2] < tr.viewParms.visBounds[0][2] ) {
			tr.viewParms.visBounds[0][2] = node->mins[2];
		}

		if ( node->maxs[0] > tr.viewParms.visBounds[1][0] ) {
			tr.viewParms.visBounds[1][0] = node->maxs[0];
		}
		if ( node->maxs[1] > tr.viewParms.visBounds[1][1] ) {
			tr.viewParms.visBounds[1][1] = node->maxs[1];
		}
		if ( node->maxs[2] > tr.viewParms.visBounds[1][2] ) {
			tr.viewParms.visBounds[1][2] = node->maxs[2];
		}

		// add the individual surfaces
		mark = node->firstmarksurface;
		c = node->nummarksurfaces;
		while (c--) {
			// the surface may have already been added if it
			// spans multiple leafs
			surf = *mark;
			R_AddWorldSurface( surf, dlightBits );
			mark++;
		}
	}

}

/*
===============
R_PointInLeaf
===============
*/
static mnode_t *R_PointInLeaf( const vec3_t p ) {
	mnode_t		*node;
	float		d;
	cplane_t	*plane;

	if ( !tr.world ) {
		Com_Error (ERR_DROP, "R_PointInLeaf: bad model");
	}

	node = tr.world->nodes;
	while( 1 ) {
		if (node->contents != -1) {
			break;
		}
		plane = node->plane;
		d = DotProduct (p,plane->normal) - plane->dist;
		if (d > 0) {
			node = node->children[0];
		} else {
			node = node->children[1];
		}
	}

	return node;
}

/*
==============
R_ClusterPVS
==============
*/
static const byte *R_ClusterPVS ( int cluster ) {
	if (!tr.world || !tr.world->vis || cluster < 0 || cluster >= tr.world->numClusters ) {
		return tr.world->novis;
	}

	return tr.world->vis + cluster * tr.world->clusterBytes;
}

/*
=================
R_inPVS
=================
*/
qboolean R_inPVS(vec3_t p1, vec3_t p2) {
	mnode_t* leaf;
	byte* vis;

	leaf = R_PointInLeaf(p1);
	vis = ri.CM_ClusterPVS(leaf->cluster);
	leaf = R_PointInLeaf(p2);

	if (!(vis[leaf->cluster >> 3] & (1 << (leaf->cluster & 7)))) {
		return qfalse;
	}
	return qtrue;
}

/*
===============
R_MarkLeaves

Mark the leaves and nodes that are in the PVS for the current
cluster
===============
*/
static void R_MarkLeaves ( void ) {
	const byte	*vis;
	mnode_t	*leaf, *parent;
	int		i;
	int		cluster;

	// lockpvs lets designers walk around to determine the
	// extent of the current pvs
	if ( r_lockpvs->integer ) {
		return;
	}

	// current viewcluster
	leaf = R_PointInLeaf( tr.viewParms.pvsOrigin );
	cluster = leaf->cluster;

	// if the cluster is the same and the area visibility matrix
	// hasn't changed, we don't need to mark everything again

	// if r_showcluster was just turned on, remark everything
	if ( tr.viewCluster == cluster && !tr.refdef.areamaskModified
		&& !r_showcluster->modified ) {
		return;
	}

	if ( r_showcluster->modified || r_showcluster->integer ) {
		r_showcluster->modified = qfalse;
		if ( r_showcluster->integer ) {
			ri.Printf( PRINT_ALL, "cluster:%i  area:%i\n", cluster, leaf->area );
		}
	}

	tr.visCount++;
	tr.viewCluster = cluster;

	if ( r_novis->integer || tr.viewCluster == -1 ) {
		for (i=0 ; i<tr.world->numnodes ; i++) {
			if (tr.world->nodes[i].contents != CONTENTS_SOLID) {
				tr.world->nodes[i].visframe = tr.visCount;
			}
		}
		return;
	}

	vis = R_ClusterPVS (tr.viewCluster);

	for (i=0,leaf=tr.world->nodes ; i<tr.world->numnodes ; i++, leaf++) {
		cluster = leaf->cluster;
		if ( cluster < 0 || cluster >= tr.world->numClusters ) {
			continue;
		}

		// check general pvs
		if ( !(vis[cluster>>3] & (1<<(cluster&7))) ) {
			continue;
		}

		// check for door connection
		if ( (tr.refdef.areamask[leaf->area>>3] & (1<<(leaf->area&7)) ) ) {
			continue;		// not visible
		}

		parent = leaf;
		do {
			if (parent->visframe == tr.visCount)
				break;
			parent->visframe = tr.visCount;
			parent = parent->parent;
		} while (parent);
	}
}

/*
=============
R_AddWorldSurfaces
=============
*/
void R_AddWorldSurfaces ( void ) {
#ifdef USE_PMLIGHT
	dlight_t* dl;
	int i;
#endif

	if ( !r_drawworld->integer ) {
		return;
	}

	if ( tr.refdef.rdflags & RDF_NOWORLDMODEL ) {
		return;
	}

	tr.currentEntityNum = REFENTITYNUM_WORLD;
	tr.shiftedEntityNum = tr.currentEntityNum << QSORT_REFENTITYNUM_SHIFT;

	// determine which leaves are in the PVS / areamask
	R_MarkLeaves ();

	// clear out the visible min/max
	ClearBounds( tr.viewParms.visBounds[0], tr.viewParms.visBounds[1] );

	// perform frustum culling and add all the potentially visible surfaces
	if ( tr.refdef.num_dlights > 32 ) {
		tr.refdef.num_dlights = 32 ;
	}

	R_RecursiveWorldNode( tr.world->nodes, 15, ( 1 << tr.refdef.num_dlights ) - 1 );

#ifdef USE_PMLIGHT
	// "transform" all the dlights so that dl->transformed is actually populated
	// (even though HERE it's == dl->origin) so we can always use R_LightCullBounds
	// instead of having copypasted versions for both world and local cases

	R_TransformDlights(tr.viewParms.num_dlights, tr.viewParms.dlights, &tr.viewParms.world);
	for (i = 0; i < tr.viewParms.num_dlights; i++)
	{
		dl = &tr.viewParms.dlights[i];
		dl->head = dl->tail = NULL;
		if (R_CullDlight(dl) == CULL_OUT) {
			tr.pc.c_light_cull_out++;
			continue;
		}
		tr.pc.c_light_cull_in++;
		tr.lightCount++;
		tr.light = dl;
		R_RecursiveLightNode(tr.world->nodes);
	}
#endif // USE_PMLIGHT
}
