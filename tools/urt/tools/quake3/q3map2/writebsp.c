/* -------------------------------------------------------------------------------

   Copyright (C) 1999-2007 id Software, Inc. and contributors.
   For a list of contributors, see the accompanying CONTRIBUTORS file.

   This file is part of GtkRadiant.

   GtkRadiant is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   GtkRadiant is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GtkRadiant; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

   ----------------------------------------------------------------------------------

   This code has been altered significantly from its original form, to support
   several games based on the Quake III Arena engine, in the form of "Q3Map2."

   ------------------------------------------------------------------------------- */



/* marker */
#define WRITEBSP_C



/* dependencies */
#include "q3map2.h"

/*
   void ForceSurfaceMerge(tree_t *headnode);
   void GatherNodes_r(node_t *node, int* count);
 */

/*
   EmitShader()
   emits a bsp shader entry
 */

int EmitShader( const char *shader, int *contentFlags, int *surfaceFlags ){
	int i;
	shaderInfo_t    *si;


	/* handle special cases */
	if ( shader == NULL ) {
		shader = "noshader";
	}

	/* try to find an existing shader */
	for ( i = 0; i < numBSPShaders; i++ )
	{
		/* ydnar: handle custom surface/content flags */
		if ( surfaceFlags != NULL && bspShaders[ i ].surfaceFlags != *surfaceFlags ) {
			continue;
		}
		if ( contentFlags != NULL && bspShaders[ i ].contentFlags != *contentFlags ) {
			continue;
		}

		/* compare name */
		if ( !Q_stricmp( shader, bspShaders[ i ].shader ) ) {
			return i;
		}
	}

	/* get shaderinfo */
	si = ShaderInfoForShader( shader );

	/* emit a new shader */
	if ( i == MAX_MAP_SHADERS ) {
		Error( "MAX_MAP_SHADERS" );
	}
	numBSPShaders++;
	Q_strncpyz( bspShaders[ i ].shader, shader, sizeof( bspShaders[ i ].shader ) );
	bspShaders[ i ].surfaceFlags = si->surfaceFlags;
	bspShaders[ i ].contentFlags = si->contentFlags;

	/* handle custom content/surface flags */
	if ( surfaceFlags != NULL ) {
		bspShaders[ i ].surfaceFlags = *surfaceFlags;
	}
	if ( contentFlags != NULL ) {
		bspShaders[ i ].contentFlags = *contentFlags;
	}

	/* recursively emit any damage shaders */
	if ( si->damageShader != NULL && si->damageShader[ 0 ] != '\0' ) {
		Sys_FPrintf( SYS_VRB, "Shader %s has damage shader %s\n", si->shader, si->damageShader );
		EmitShader( si->damageShader, NULL, NULL );
	}

	/* return it */
	return i;
}



/*
   EmitPlanes()
   there is no oportunity to discard planes, because all of the original
   brushes will be saved in the map
 */

void EmitPlanes( void ){
	int i;
	bspPlane_t  *bp;
	plane_t     *mp;


	/* walk plane list */
	mp = mapplanes;
	for ( i = 0; i < nummapplanes; i++, mp++ )
	{
		bp = &bspPlanes[ numBSPPlanes ];
		VectorCopy( mp->normal, bp->normal );
		bp->dist = mp->dist;
		numBSPPlanes++;
	}

	/* emit some statistics */
	Sys_FPrintf( SYS_VRB, "%9d BSP planes\n", numBSPPlanes );
}



/*
   EmitLeaf()
   emits a leafnode to the bsp file
 */

void EmitLeaf( node_t *node ){
	bspLeaf_t       *leaf_p;
	brush_t         *b;
	drawSurfRef_t   *dsr;


	/* check limits */
	if ( numBSPLeafs >= MAX_MAP_LEAFS ) {
		Error( "MAX_MAP_LEAFS" );
	}

	leaf_p = &bspLeafs[numBSPLeafs];
	numBSPLeafs++;

	leaf_p->cluster = node->cluster;
	leaf_p->area = node->area;

	/* emit bounding box */
	VectorCopy( node->mins, leaf_p->mins );
	VectorCopy( node->maxs, leaf_p->maxs );

	/* emit leaf brushes */
	leaf_p->firstBSPLeafBrush = numBSPLeafBrushes;
	for ( b = node->brushlist; b; b = b->next )
	{
		/* something is corrupting brushes */
		if ( (int) b < 256 ) {
			Sys_Printf( "WARNING: Node brush list corrupted (0x%08X)\n", b );
			break;
		}
		//%	if( b->guard != 0xDEADBEEF )
		//%		Sys_Printf( "Brush %6d: 0x%08X Guard: 0x%08X Next: 0x%08X Original: 0x%08X Sides: %d\n", b->brushNum, b, b, b->next, b->original, b->numsides );

		if ( numBSPLeafBrushes >= MAX_MAP_LEAFBRUSHES ) {
			Error( "MAX_MAP_LEAFBRUSHES" );
		}
		bspLeafBrushes[ numBSPLeafBrushes ] = b->original->outputNum;
		numBSPLeafBrushes++;
	}

	leaf_p->numBSPLeafBrushes = numBSPLeafBrushes - leaf_p->firstBSPLeafBrush;

	/* emit leaf surfaces */
	if ( node->opaque ) {
		return;
	}

	/* add the drawSurfRef_t drawsurfs */
	leaf_p->firstBSPLeafSurface = numBSPLeafSurfaces;
	for ( dsr = node->drawSurfReferences; dsr; dsr = dsr->nextRef )
	{
		if ( numBSPLeafSurfaces >= MAX_MAP_LEAFFACES ) {
			Error( "MAX_MAP_LEAFFACES" );
		}
		bspLeafSurfaces[ numBSPLeafSurfaces ] = dsr->outputNum;
		numBSPLeafSurfaces++;
	}

	leaf_p->numBSPLeafSurfaces = numBSPLeafSurfaces - leaf_p->firstBSPLeafSurface;
}


/*
   EmitDrawNode_r()
   recursively emit the bsp nodes
 */

int EmitDrawNode_r( node_t *node ){
	bspNode_t   *n;
	int i;


	/* check for leafnode */
	if ( node->planenum == PLANENUM_LEAF ) {
		EmitLeaf( node );
		return -numBSPLeafs;
	}

	/* emit a node */
	if ( numBSPNodes == MAX_MAP_NODES ) {
		Error( "MAX_MAP_NODES" );
	}
	n = &bspNodes[ numBSPNodes ];
	numBSPNodes++;

	VectorCopy( node->mins, n->mins );
	VectorCopy( node->maxs, n->maxs );

	if ( node->planenum & 1 ) {
		Error( "WriteDrawNodes_r: odd planenum" );
	}
	n->planeNum = node->planenum;

	//
	// recursively output the other nodes
	//
	for ( i = 0 ; i < 2 ; i++ )
	{
		if ( node->children[i]->planenum == PLANENUM_LEAF ) {
			n->children[i] = -( numBSPLeafs + 1 );
			EmitLeaf( node->children[i] );
		}
		else
		{
			n->children[i] = numBSPNodes;
			EmitDrawNode_r( node->children[i] );
		}
	}

	return n - bspNodes;
}



/*
   ============
   SetModelNumbers
   ============
 */
void SetModelNumbers( void ){
	int i;
	int models;
	char value[16];

	models = 1;
	for ( i = 1 ; i < numEntities ; i++ ) {
		if ( entities[i].brushes || entities[i].patches ) {
			snprintf( value, sizeof( value ), "*%i", models );
			models++;
			SetKeyValue( &entities[i], "model", value );
		}
	}

}




/*
   SetLightStyles()
   sets style keys for entity lights
 */

void SetLightStyles( void ){
	int i, j, style, numStyles;
	qboolean keepLights;
	const char  *t;
	entity_t    *e;
	epair_t     *ep, *next;
	char value[ 16 ];
	char lightTargets[ MAX_SWITCHED_LIGHTS ][ 64 ];
	int lightStyles[ MAX_SWITCHED_LIGHTS ];


	/* ydnar: determine if we keep lights in the bsp */
	t = ValueForKey( &entities[ 0 ], "_keepLights" );
	keepLights = ( t[ 0 ] == '1' ) ? qtrue : qfalse;

	/* any light that is controlled (has a targetname) must have a unique style number generated for it */
	numStyles = 0;
	for ( i = 1; i < numEntities; i++ )
	{
		e = &entities[ i ];

		t = ValueForKey( e, "classname" );
		if ( Q_strncasecmp( t, "light", 5 ) ) {
			continue;
		}
		t = ValueForKey( e, "targetname" );
		if ( t[ 0 ] == '\0' ) {
			/* ydnar: strip the light from the BSP file */
			if ( keepLights == qfalse ) {
				ep = e->epairs;
				while ( ep != NULL )
				{
					next = ep->next;
					free( ep->key );
					free( ep->value );
					free( ep );
					ep = next;
				}
				e->epairs = NULL;
				numStrippedLights++;
			}

			/* next light */
			continue;
		}

		/* get existing style */
		style = IntForKey( e, "style" );
		if ( style < LS_NORMAL || style > LS_NONE ) {
			Error( "Invalid lightstyle (%d) on entity %d", style, i );
		}

		/* find this targetname */
		for ( j = 0; j < numStyles; j++ )
			if ( lightStyles[ j ] == style && !strcmp( lightTargets[ j ], t ) ) {
				break;
			}

		/* add a new style */
		if ( j >= numStyles ) {
			if ( numStyles == MAX_SWITCHED_LIGHTS ) {
				Error( "MAX_SWITCHED_LIGHTS (%d) exceeded, reduce the number of lights with targetnames", MAX_SWITCHED_LIGHTS );
			}
			Q_strncpyz( lightTargets[ j ], t, sizeof( lightTargets[ j ] ) );
			lightStyles[ j ] = style;
			numStyles++;
		}

		/* set explicit style */
		snprintf( value, sizeof( value ), "%d", 32 + j );
		SetKeyValue( e, "style", value );

		/* set old style */
		if ( style != LS_NORMAL ) {
			snprintf( value, sizeof( value ), "%d", style );
			SetKeyValue( e, "switch_style", value );
		}
	}

	/* emit some statistics */
	Sys_FPrintf( SYS_VRB, "%9d light entities stripped\n", numStrippedLights );
}



/*
   BeginBSPFile()
   starts a new bsp file
 */

void BeginBSPFile( void ){
	/* these values may actually be initialized if the file existed when loaded, so clear them explicitly */
	numBSPModels = 0;
	numBSPNodes = 0;
	numBSPBrushSides = 0;
	numBSPLeafSurfaces = 0;
	numBSPLeafBrushes = 0;

	/* leave leaf 0 as an error, because leafs are referenced as negative number nodes */
	numBSPLeafs = 1;


	/* ydnar: gs mods: set the first 6 drawindexes to 0 1 2 2 1 3 for triangles and quads */
	numBSPDrawIndexes = 6;
	bspDrawIndexes[ 0 ] = 0;
	bspDrawIndexes[ 1 ] = 1;
	bspDrawIndexes[ 2 ] = 2;
	bspDrawIndexes[ 3 ] = 0;
	bspDrawIndexes[ 4 ] = 2;
	bspDrawIndexes[ 5 ] = 3;
}



/*
   EndBSPFile()
   finishes a new bsp and writes to disk
 */

void EndBSPFile( void ){
	char path[ 1024 ];


	EmitPlanes();

	numBSPEntities = numEntities;
	UnparseEntities();

	/* write the surface extra file */
	WriteSurfaceExtraFile( source );

	/* write the bsp */
	snprintf( path, sizeof( path ), "%s.bsp", source );
	Sys_Printf( "Writing %s\n", path );
	WriteBSPFile( path );
}



/*
   EmitBrushes()
   writes the brush list to the bsp
 */

void EmitBrushes( brush_t *brushes, int *firstBrush, int *numBrushes ){
	int j;
	brush_t         *b;
	bspBrush_t      *db;
	bspBrushSide_t  *cp;


	/* set initial brush */
	if ( firstBrush != NULL ) {
		*firstBrush = numBSPBrushes;
	}
	if ( numBrushes != NULL ) {
		*numBrushes = 0;
	}

	/* walk list of brushes */
	for ( b = brushes; b != NULL; b = b->next )
	{
		/* check limits */
		if ( numBSPBrushes == MAX_MAP_BRUSHES ) {
			Error( "MAX_MAP_BRUSHES (%d)", numBSPBrushes );
		}

		/* get bsp brush */
		b->outputNum = numBSPBrushes;
		db = &bspBrushes[ numBSPBrushes ];
		numBSPBrushes++;
		if ( numBrushes != NULL ) {
			( *numBrushes )++;
		}

		db->shaderNum = EmitShader( b->contentShader->shader, &b->contentShader->contentFlags, &b->contentShader->surfaceFlags );
		db->firstSide = numBSPBrushSides;

		/* walk sides */
		db->numSides = 0;
		for ( j = 0; j < b->numsides; j++ )
		{
			/* set output number to bogus initially */
			b->sides[ j ].outputNum = -1;

			/* check count */
			if ( numBSPBrushSides == MAX_MAP_BRUSHSIDES ) {
				Error( "MAX_MAP_BRUSHSIDES " );
			}

			/* emit side */
			b->sides[ j ].outputNum = numBSPBrushSides;
			cp = &bspBrushSides[ numBSPBrushSides ];
			db->numSides++;
			numBSPBrushSides++;
			cp->planeNum = b->sides[ j ].planenum;

			/* emit shader */
			if ( b->sides[ j ].shaderInfo ) {
				cp->shaderNum = EmitShader( b->sides[ j ].shaderInfo->shader, &b->sides[ j ].shaderInfo->contentFlags, &b->sides[ j ].shaderInfo->surfaceFlags );
			}
			else{
				cp->shaderNum = EmitShader( NULL, NULL, NULL );
			}
		}
	}
}



/*
   EmitFogs() - ydnar
   turns map fogs into bsp fogs
 */

void EmitFogs( void ){
	int i, j;


	/* setup */
	numBSPFogs = numMapFogs;

	/* walk list */
	for ( i = 0; i < numMapFogs; i++ )
	{
		/* set shader */
		Q_strncpyz( bspFogs[ i ].shader, mapFogs[ i ].si->shader, sizeof( bspFogs[ i ].shader ) );

		/* global fog doesn't have an associated brush */
		if ( mapFogs[ i ].brush == NULL ) {
			bspFogs[ i ].brushNum = -1;
			bspFogs[ i ].visibleSide = -1;
		}
		else
		{
			/* set brush */
			bspFogs[ i ].brushNum = mapFogs[ i ].brush->outputNum;

			/* try to use forced visible side */
			if ( mapFogs[ i ].visibleSide >= 0 ) {
				bspFogs[ i ].visibleSide = mapFogs[ i ].visibleSide;
				continue;
			}

			/* find visible side */
			for ( j = 0; j < 6; j++ )
			{
				if ( mapFogs[ i ].brush->sides[ j ].visibleHull != NULL ) {
					Sys_Printf( "Fog %d has visible side %d\n", i, j );
					bspFogs[ i ].visibleSide = j;
					break;
				}
			}
		}
	}
}



/*
   BeginModel()
   sets up a new brush model
 */

void BeginModel( void ){
	bspModel_t  *mod;
	brush_t     *b;
	entity_t    *e;
	vec3_t mins, maxs;
	vec3_t lgMins, lgMaxs;          /* ydnar: lightgrid mins/maxs */
	parseMesh_t *p;
	int i;


	/* test limits */
	if ( numBSPModels == MAX_MAP_MODELS ) {
		Error( "MAX_MAP_MODELS" );
	}

	/* get model and entity */
	mod = &bspModels[ numBSPModels ];
	e = &entities[ mapEntityNum ];

	/* ydnar: lightgrid mins/maxs */
	ClearBounds( lgMins, lgMaxs );

	/* bound the brushes */
	ClearBounds( mins, maxs );
	for ( b = e->brushes; b; b = b->next )
	{
		/* ignore non-real brushes (origin, etc) */
		if ( b->numsides == 0 ) {
			continue;
		}
		AddPointToBounds( b->mins, mins, maxs );
		AddPointToBounds( b->maxs, mins, maxs );

		/* ydnar: lightgrid bounds */
		if ( b->compileFlags & C_LIGHTGRID ) {
			AddPointToBounds( b->mins, lgMins, lgMaxs );
			AddPointToBounds( b->maxs, lgMins, lgMaxs );
		}
	}

	/* bound patches */
	for ( p = e->patches; p; p = p->next )
	{
		for ( i = 0; i < ( p->mesh.width * p->mesh.height ); i++ )
			AddPointToBounds( p->mesh.verts[i].xyz, mins, maxs );
	}

	/* ydnar: lightgrid mins/maxs */
	if ( lgMins[ 0 ] < 99999 ) {
		/* use lightgrid bounds */
		VectorCopy( lgMins, mod->mins );
		VectorCopy( lgMaxs, mod->maxs );
	}
	else
	{
		/* use brush/patch bounds */
		VectorCopy( mins, mod->mins );
		VectorCopy( maxs, mod->maxs );
	}

	/* note size */
	Sys_FPrintf( SYS_VRB, "BSP bounds: { %f %f %f } { %f %f %f }\n", mins[ 0 ], mins[ 1 ], mins[ 2 ], maxs[ 0 ], maxs[ 1 ], maxs[ 2 ] );
	Sys_FPrintf( SYS_VRB, "Lightgrid bounds: { %f %f %f } { %f %f %f }\n", lgMins[ 0 ], lgMins[ 1 ], lgMins[ 2 ], lgMaxs[ 0 ], lgMaxs[ 1 ], lgMaxs[ 2 ] );

	/* set firsts */
	mod->firstBSPSurface = numBSPDrawSurfaces;
	mod->firstBSPBrush = numBSPBrushes;
}




/*
   EndModel()
   finish a model's processing
 */

void EndModel( entity_t *e, node_t *headnode, tree_t *tree ){
	bspModel_t  *mod;


	/* note it */
	Sys_FPrintf( SYS_VRB, "--- EndModel ---\n" );

	/* emit the bsp */
	mod = &bspModels[ numBSPModels ];

	//ForceSurfaceMerge(tree);

	//Emit the brushes and surfs
	EmitDrawNode_r( headnode );

	/* set surfaces and brushes */
	mod->numBSPSurfaces = numBSPDrawSurfaces - mod->firstBSPSurface;
	mod->firstBSPBrush = e->firstBrush;
	mod->numBSPBrushes = e->numBrushes;

	/* increment model count */
	numBSPModels++;
}


//Alright this was an interesting excerise.
//The below code works provided you don't try and light it.. (bsp phase only) Call it during
//Anyway, as it turns out, q3 doesnt get any faster with just merging surface calls
//600 down to 91 drawcalls didnt go faster without a dedicated T&L piece in the engine
//And its WAAAY cleaner to merge surfaces there then here.
//
//bspLeaf_t*  *LeafNodeList;

/*
   void GatherNodes_r(node_t *node, int* count)
   {
    int			i;

    // check for leafnode
    if( node->planenum == PLANENUM_LEAF )
    {
      //store leaf
      int counter=*count;
   *count=*count+1;
      LeafNodeList[counter]=node;
      return;
    }

    // recursively output the other nodes
    for (i=0 ; i<2 ; i++)
    {
        GatherNodes_r(node->children[i],count);
    }

   }


   typedef struct
   {
   int StartIndex;
   int StartVertex;

   int IndexCount;
   int VertexCount;
   int shaderNum;

   bspDrawVert_t *VertBucket;
   int *IndexBucket;

   bspDrawSurface_t *DrawSurfs[512];//only has to hold as many leafs reference this drawsurf-- so not many
   int DrawSurfCount;

   qboolean Closed; //will be false until we exceed max verts/indexes for a given surface
                    //Honestly, having to do this bites.

   bspDrawSurface_t* Surface;// Pointer to the final surface
   int SurfaceNumber;

   int   surfaceType;   //Only same surface types can be merged
   int   lightmapNum;
   } MergedSurface_t;


   void ForceSurfaceMerge( tree_t *tree)
   {
   int count=0;
   int j=0;
   int r=0;
   bspDrawSurface_t* ds ;
   bspDrawSurface_t* rs ;
   drawSurfRef_t	*dsr;  //Start
   drawSurfRef_t	*drr;  //R
   int found=-1;
   int dsfound=-1;
   int q;
   qboolean qfound;

   //Setup some buckets to hold our merged surfaces
   //   bspDrawVert_t *VertBucket=malloc(200000*sizeof(bspDrawVert_t));
   //   int *IndexBucket=malloc(600000*sizeof(int));
   MergedSurface_t *MergedSurfaces=malloc(5000*sizeof(MergedSurface_t));

   int VertCount=0;
   int IndexCount=0;
   int MergedSurfaceCount=0;
   int BeforeCount=0;

   int MAX_VERTS = 999;
   int MAX_INDEXS = 5000;

   //First build a nice easy traversable node list.
   LeafNodeList=malloc(sizeof(node_t*)*32768);
   GatherNodes_r( tree->headnode,&count);

   //Memset
   memset(MergedSurfaces,0,5000*sizeof(MergedSurface_t));

   Sys_FPrintf( SYS_STD, "Starting Merge Process to optimize draw calls for T+L\n");


   //Clear the merge flags per drawsurf
   for (j=0;j<count;j++)
   {
      //LeafNodeList[j]->planenum=0;
      if (LeafNodeList[j]->drawSurfReferences==NULL) continue;

      //For each drawsurf in this leaf
      for ( dsr = LeafNodeList[j]->drawSurfReferences; dsr; dsr = dsr->nextRef )
      {
         ds = &bspDrawSurfaces[dsr->outputNum ];
         ds->mergeNum=-1;
         //dsr->MergeAssign=-1;
      }
   }


   //First pass, just figure out our list of merged shaders and what drawsurfs they need
   //This would have been so much more elegant with some sort of vector class.
   //leafs record here which merge surface they got assigned to
   for (j=0;j<count;j++)
   {
      //LeafNodeList[j]->planenum=0;
      if (LeafNodeList[j]->drawSurfReferences==NULL) continue;

      //For each drawsurf in this leaf
      for ( dsr = LeafNodeList[j]->drawSurfReferences; dsr; dsr = dsr->nextRef )
      {
         ds = &bspDrawSurfaces[dsr->outputNum ];
         if (ds->mergeNum>-1) continue;

         //Check our merged surface bucket.  If we can't find a suitable merged surface (or an empty enough one), create one.
         found=-1;
         for (r=0;r<MergedSurfaceCount;r++)
         {
            //Todo: also check lightmap, model, etc
            if (MergedSurfaces[r].shaderNum!=ds->shaderNum) continue;
            if (MergedSurfaces[r].surfaceType!=ds->surfaceType) continue;

            if (MergedSurfaces[r].lightmapNum!=ds->lightmapNum) continue;
            //Nearly ready to add it, see if it'll actually fit.
            //if (MergedSurfaces[r].Closed==qtrue) continue;
            if (MergedSurfaces[r].VertexCount+ds->numVerts>MAX_VERTS)
               continue;
            if (MergedSurfaces[r].IndexCount+ds->numIndexes>MAX_INDEXS)
               continue;

            found =r;
            break;

         }
         if (found==-1)
         {
            //create a new merged surface
            found=MergedSurfaceCount;
            MergedSurfaceCount++;
            //init a new one
            MergedSurfaces[found].shaderNum=ds->shaderNum;
            MergedSurfaces[found].surfaceType=ds->surfaceType;
            MergedSurfaces[found].lightmapNum=ds->lightmapNum;
         }

         //Keep track of where we ended up
         //dsr->MergeAssign=found;
         ds->mergeNum=found;

         //Add this drawsurface uniquely to the list of DS's in this Merged Surface
         dsfound=MergedSurfaces[found].DrawSurfCount;
         MergedSurfaces[found].DrawSurfCount++;

         MergedSurfaces[found].DrawSurfs[dsfound]=ds;
         MergedSurfaces[found].IndexCount+=ds->numIndexes;
         MergedSurfaces[found].VertexCount+=ds->numVerts;

      }
   }
   //Bold Claims
   Sys_FPrintf( SYS_STD, "%d Draw calls optimized down to %d\n",numBSPDrawSurfaces,MergedSurfaceCount);

   //Now time to actually do the merging..
   for (j=0;j<MergedSurfaceCount;j++)
   {
      int indexoffset=0;

      //malloc the vert buckets
      MergedSurfaces[j].IndexBucket=malloc(MergedSurfaces[j].IndexCount*sizeof(int));
      MergedSurfaces[j].VertBucket=malloc(MergedSurfaces[j].VertexCount*sizeof(bspDrawVert_t));

      //reset the counts
      MergedSurfaces[j].StartIndex=0;
      MergedSurfaces[j].StartVertex=0;


      //Sys_FPrintf( SYS_STD, "Start of Surf|");
      //copy all the triangles out
      for (r=0;r<MergedSurfaces[j].DrawSurfCount;r++)
      {
         //indexes first
         int index=0;

         //Note: it appears all indexes are 0 relative to the first vertex THEY use, versus the entire vertex buffer
         //      this is really useful because it makes it easier to copy and move around
         //Sys_FPrintf( SYS_STD, "|");
         for (index=0;index<MergedSurfaces[j].DrawSurfs[r]->numIndexes;index++)
         {
            if (MergedSurfaces[j].StartIndex>=MergedSurfaces[j].IndexCount)
                 Error( "Overflowed Indexes" );

            MergedSurfaces[j].IndexBucket[MergedSurfaces[j].StartIndex++]=bspDrawIndexes[ MergedSurfaces[j].DrawSurfs[r]->firstIndex + index ]+indexoffset;
            //Sys_FPrintf( SYS_STD, "%d ",MergedSurfaces[j].IndexBucket[MergedSurfaces[j].StartIndex-1] );
         }

         //Now vertexes
         for (index=0;index<MergedSurfaces[j].DrawSurfs[r]->numVerts;index++)
         {
            if (MergedSurfaces[j].StartVertex>=MergedSurfaces[j].VertexCount)
                 Error( "Overflowed Vertexes" );

            MergedSurfaces[j].VertBucket[ MergedSurfaces[j].StartVertex++ ]=bspDrawVerts[ MergedSurfaces[j].DrawSurfs[r]->firstVert + index ];
            indexoffset++;
         }
      }
   }

   //time to replace bspdrawverts and bspdrawindexs
   numBSPDrawIndexes=0;
   numBSPDrawVerts=0;

   for (j=0;j<MergedSurfaceCount;j++)
   {
      //Create a new merged surface
      bspDrawSurface_t *output;
      int index;

       // allocate a new surface
       if( numBSPDrawSurfaces == MAX_MAP_DRAW_SURFS )
           Error( "MAX_MAP_DRAW_SURFS" );
       output = &bspDrawSurfaces[ numBSPDrawSurfaces ];
       MergedSurfaces[j].SurfaceNumber=numBSPDrawSurfaces;
      MergedSurfaces[j].Surface=output;

      numBSPDrawSurfaces++;
       memset( output, 0, sizeof( *output ) );

      //Make a rough copy of one of the surfaces from the merge
      memcpy(output,MergedSurfaces[j].DrawSurfs[0],sizeof(bspDrawSurface_t));

      //set it happy
      output->firstIndex=numBSPDrawIndexes;
      output->firstVert=numBSPDrawVerts;
      output->numIndexes=MergedSurfaces[j].IndexCount;
      output->numVerts=MergedSurfaces[j].VertexCount;

      //if it was planar, it wont be now!
      if (output->surfaceType==MST_PLANAR)
         output->surfaceType=MST_TRIANGLE_SOUP;

      for (index=0;index<MergedSurfaces[j].IndexCount;index++)
      {
            bspDrawIndexes[numBSPDrawIndexes++]=MergedSurfaces[j].IndexBucket[index];
      }
      for (index=0;index<MergedSurfaces[j].VertexCount;index++)
      {
            bspDrawVerts[numBSPDrawVerts++]=MergedSurfaces[j].VertBucket[index];
      }

   }

   //Assign all of the leafs to their new merged surface
   for (j=0;j<count;j++)
   {
      if (LeafNodeList[j]->drawSurfReferences==NULL) continue;

      //For each drawsurf in this leaf
      for ( dsr = LeafNodeList[j]->drawSurfReferences; dsr; dsr = dsr->nextRef )
      {
         ds = &bspDrawSurfaces[dsr->outputNum ];
         if (ds->mergeNum==-1)
         {
            Error( "Unassigned Surface Merge" );
         }
         dsr->outputNum=MergedSurfaces[ds->mergeNum].SurfaceNumber;
      }
   }
   free(LeafNodeList);
   }

 */
