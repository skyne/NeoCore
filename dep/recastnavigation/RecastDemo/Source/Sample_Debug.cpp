//
// Copyright (c) 2009-2010 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include "Sample_Debug.h"
#include "InputGeom.h"
#include "Recast.h"
#include "DetourNavMesh.h"
#include "RecastDebugDraw.h"
#include "DetourDebugDraw.h"
#include "RecastDump.h"
#include "imgui.h"
#include "SDL.h"
#include "SDL_opengl.h"

#include "NavMeshTesterTool.h"

#include "Debug.h"

#ifdef WIN32
#	define snprintf _snprintf
#endif

Sample_Debug::Sample_Debug() : Sample_SoloMeshTiled(),
    m_hf(0), m_hfCount(0),
    m_chf(0), m_chfCount(0),
    m_cset(0), m_csetCount(0),
    m_pmeshCount(0), m_dmeshCount(0),
    m_tile(0)
{
	resetCommonSettings();

    setTool(new NavMeshTesterTool);
}

Sample_Debug::~Sample_Debug()
{
    cleanup();
}

void Sample_Debug::cleanup()
{
    int i;

    for (i = 0; i < m_hfCount; i++)
        rcFreeHeightField(m_hf[i]);
    m_hf = 0;

    for (i = 0; i < m_chfCount; i++)
        rcFreeCompactHeightfield(m_chf[i]);
    m_chf = 0;

    for (i = 0; i < m_csetCount; i++)
        rcFreeContourSet(m_cset[i]);
    m_cset = 0;

    for (i = 0; i < m_pmeshCount; i++)
        rcFreePolyMesh(m_pmeshes[i]);
    m_pmeshes = 0;

    for (i = 0; i < m_dmeshCount; i++)
        rcFreePolyMeshDetail(m_dmeshes[i]);
    m_dmeshes = 0;

    dtFreeNavMesh(m_navMesh);
    m_navMesh = 0;
}

void Sample_Debug::handleSettings()
{
}

void Sample_Debug::handleTools()
{
    Sample_SoloMeshTiled::handleTools();
}

void Sample_Debug::handleDebugMode()
{	// Check which modes are valid.
	bool valid[MAX_DRAWMODE];
	for (int i = 0; i < MAX_DRAWMODE; ++i)
		valid[i] = false;

	bool hasChf = false;
	bool hasSolid = false;
	bool hasCset = false;
	bool hasPmesh = false;
	bool hasDmesh = false;

	if (m_hf) hasSolid = true;
	if (m_chf) hasChf = true;
	if (m_cset) hasCset = true;
	if (m_pmeshes) hasPmesh = true;
	if (m_dmeshes) hasDmesh = true;
	
	if (m_geom)
	{
		valid[DRAWMODE_NAVMESH] = m_navMesh != 0;
		valid[DRAWMODE_NAVMESH_TRANS] = m_navMesh != 0;
		valid[DRAWMODE_NAVMESH_BVTREE] = m_navMesh != 0;
		valid[DRAWMODE_NAVMESH_INVIS] = m_navMesh != 0;
		valid[DRAWMODE_MESH] = true;
		valid[DRAWMODE_VOXELS] = hasSolid;
		valid[DRAWMODE_VOXELS_WALKABLE] = hasSolid;
		valid[DRAWMODE_COMPACT] = hasChf;
		valid[DRAWMODE_COMPACT_DISTANCE] = hasChf;
		valid[DRAWMODE_COMPACT_REGIONS] = hasChf;
		valid[DRAWMODE_REGION_CONNECTIONS] = hasCset;
		valid[DRAWMODE_RAW_CONTOURS] = hasCset;
		valid[DRAWMODE_BOTH_CONTOURS] = hasCset;
		valid[DRAWMODE_CONTOURS] = hasCset;
		valid[DRAWMODE_POLYMESH] = hasPmesh;
		valid[DRAWMODE_POLYMESH_DETAIL] = hasDmesh;
	}
	
	int unavail = 0;
	for (int i = 0; i < MAX_DRAWMODE; ++i)
		if (!valid[i]) unavail++;
	
	if (unavail == MAX_DRAWMODE)
		return;
	
	imguiLabel("Draw");
	if (imguiCheck("Input Mesh", m_drawMode == DRAWMODE_MESH, valid[DRAWMODE_MESH]))
		m_drawMode = DRAWMODE_MESH;
	if (imguiCheck("Navmesh", m_drawMode == DRAWMODE_NAVMESH, valid[DRAWMODE_NAVMESH]))
		m_drawMode = DRAWMODE_NAVMESH;
	if (imguiCheck("Navmesh Invis", m_drawMode == DRAWMODE_NAVMESH_INVIS, valid[DRAWMODE_NAVMESH_INVIS]))
		m_drawMode = DRAWMODE_NAVMESH_INVIS;
	if (imguiCheck("Navmesh Trans", m_drawMode == DRAWMODE_NAVMESH_TRANS, valid[DRAWMODE_NAVMESH_TRANS]))
		m_drawMode = DRAWMODE_NAVMESH_TRANS;
	if (imguiCheck("Navmesh BVTree", m_drawMode == DRAWMODE_NAVMESH_BVTREE, valid[DRAWMODE_NAVMESH_BVTREE]))
		m_drawMode = DRAWMODE_NAVMESH_BVTREE;
	if (imguiCheck("Voxels", m_drawMode == DRAWMODE_VOXELS, valid[DRAWMODE_VOXELS]))
		m_drawMode = DRAWMODE_VOXELS;
	if (imguiCheck("Walkable Voxels", m_drawMode == DRAWMODE_VOXELS_WALKABLE, valid[DRAWMODE_VOXELS_WALKABLE]))
		m_drawMode = DRAWMODE_VOXELS_WALKABLE;
	if (imguiCheck("Compact", m_drawMode == DRAWMODE_COMPACT, valid[DRAWMODE_COMPACT]))
		m_drawMode = DRAWMODE_COMPACT;
	if (imguiCheck("Compact Distance", m_drawMode == DRAWMODE_COMPACT_DISTANCE, valid[DRAWMODE_COMPACT_DISTANCE]))
		m_drawMode = DRAWMODE_COMPACT_DISTANCE;
	if (imguiCheck("Compact Regions", m_drawMode == DRAWMODE_COMPACT_REGIONS, valid[DRAWMODE_COMPACT_REGIONS]))
		m_drawMode = DRAWMODE_COMPACT_REGIONS;
	if (imguiCheck("Region Connections", m_drawMode == DRAWMODE_REGION_CONNECTIONS, valid[DRAWMODE_REGION_CONNECTIONS]))
		m_drawMode = DRAWMODE_REGION_CONNECTIONS;
	if (imguiCheck("Raw Contours", m_drawMode == DRAWMODE_RAW_CONTOURS, valid[DRAWMODE_RAW_CONTOURS]))
		m_drawMode = DRAWMODE_RAW_CONTOURS;
	if (imguiCheck("Both Contours", m_drawMode == DRAWMODE_BOTH_CONTOURS, valid[DRAWMODE_BOTH_CONTOURS]))
		m_drawMode = DRAWMODE_BOTH_CONTOURS;
	if (imguiCheck("Contours", m_drawMode == DRAWMODE_CONTOURS, valid[DRAWMODE_CONTOURS]))
		m_drawMode = DRAWMODE_CONTOURS;
	if (imguiCheck("Poly Mesh", m_drawMode == DRAWMODE_POLYMESH, valid[DRAWMODE_POLYMESH]))
		m_drawMode = DRAWMODE_POLYMESH;
	if (imguiCheck("Poly Mesh Detail", m_drawMode == DRAWMODE_POLYMESH_DETAIL, valid[DRAWMODE_POLYMESH_DETAIL]))
		m_drawMode = DRAWMODE_POLYMESH_DETAIL;

    float tile = m_tile;
    imguiSlider("Tile", &tile, 0.f, float(m_pmeshCount - 1), 1.f, true);
    m_tile = int(tile);
}

void Sample_Debug::handleRender()
{
	if (!m_geom || !m_geom->getMesh())
		return;
	
	DebugDrawGL dd;
	
	glEnable(GL_FOG);
	glDepthMask(GL_TRUE);
	
	if (m_drawMode == DRAWMODE_MESH)
	{
		// Draw mesh
		duDebugDrawTriMeshSlope(&dd, m_geom->getMesh()->getVerts(), m_geom->getMesh()->getVertCount(),
								m_geom->getMesh()->getTris(), m_geom->getMesh()->getNormals(), m_geom->getMesh()->getTriCount(),
								m_agentMaxSlope);
		m_geom->drawOffMeshConnections(&dd);
	}
	else if (m_drawMode != DRAWMODE_NAVMESH_TRANS)
	{
		// Draw mesh
		duDebugDrawTriMesh(&dd, m_geom->getMesh()->getVerts(), m_geom->getMesh()->getVertCount(),
						   m_geom->getMesh()->getTris(), m_geom->getMesh()->getNormals(), m_geom->getMesh()->getTriCount(), 0);
		m_geom->drawOffMeshConnections(&dd);
	}
	
	glDisable(GL_FOG);
	glDepthMask(GL_FALSE);
	
	// Draw bounds
	const float* bmin = m_geom->getMeshBoundsMin();
	const float* bmax = m_geom->getMeshBoundsMax();
	duDebugDrawBoxWire(&dd, bmin[0],bmin[1],bmin[2], bmax[0],bmax[1],bmax[2], duRGBA(255,255,255,128), 1.0f);
	
	// Tiling grid.
	int gw = 0, gh = 0;
	rcCalcGridSize(bmin, bmax, m_cellSize, &gw, &gh);
	const int tw = (gw + (int)m_tileSize-1) / (int)m_tileSize;
	const int th = (gh + (int)m_tileSize-1) / (int)m_tileSize;
	duDebugDrawGridXZ(&dd, bmin[0],bmin[1],bmin[2], tw,th, m_tileSize, duRGBA(0,0,0,64), 1.0f);
	
	if (m_navMesh &&
		(m_drawMode == DRAWMODE_NAVMESH ||
		 m_drawMode == DRAWMODE_NAVMESH_TRANS ||
		 m_drawMode == DRAWMODE_NAVMESH_BVTREE ||
		 m_drawMode == DRAWMODE_NAVMESH_INVIS))
	{
		if (m_drawMode != DRAWMODE_NAVMESH_INVIS)
			duDebugDrawNavMesh(&dd, *m_navMesh, m_navMeshDrawFlags);
		if (m_drawMode == DRAWMODE_NAVMESH_BVTREE)
			duDebugDrawNavMeshBVTree(&dd, *m_navMesh);
	}
	
	glDepthMask(GL_TRUE);
	
    if (m_drawMode == DRAWMODE_COMPACT)
		for (int i = 0; i < m_chfCount; ++i)
            if(i == m_tile)
			duDebugDrawCompactHeightfieldSolid(&dd, *m_chf[i]);

	if (m_drawMode == DRAWMODE_COMPACT_DISTANCE)
		for (int i = 0; i < m_chfCount; ++i)
            if(i == m_tile)
			duDebugDrawCompactHeightfieldDistance(&dd, *m_chf[i]);

	if (m_drawMode == DRAWMODE_COMPACT_REGIONS)
		for (int i = 0; i < m_chfCount; ++i)
            if(i == m_tile)
			duDebugDrawCompactHeightfieldRegions(&dd, *m_chf[i]);
	
	if (m_drawMode == DRAWMODE_VOXELS)
	{
		glEnable(GL_FOG);
		for (int i = 0; i < m_hfCount; ++i)
            if(i == m_tile)
			duDebugDrawHeightfieldSolid(&dd, *m_hf[i]);
		glDisable(GL_FOG);
	}

	if (m_drawMode == DRAWMODE_VOXELS_WALKABLE)
	{
		glEnable(GL_FOG);
		for (int i = 0; i < m_hfCount; ++i)
            if(i == m_tile)
			duDebugDrawHeightfieldWalkable(&dd, *m_hf[i]);
		glDisable(GL_FOG);
	}

	if (m_drawMode == DRAWMODE_RAW_CONTOURS)
	{
		glDepthMask(GL_FALSE);
		for (int i = 0; i < m_csetCount; ++i)
            if(i == m_tile)
			duDebugDrawRawContours(&dd, *m_cset[i]);
		glDepthMask(GL_TRUE);
	}

	if (m_drawMode == DRAWMODE_BOTH_CONTOURS)
	{
		glDepthMask(GL_FALSE);
		for (int i = 0; i < m_csetCount; ++i)
            if(i == m_tile)
		{
			duDebugDrawRawContours(&dd, *m_cset[i], 0.5f);
			duDebugDrawContours(&dd, *m_cset[i]);
		}
		glDepthMask(GL_TRUE);
	}

	if (m_drawMode == DRAWMODE_CONTOURS)
	{
		glDepthMask(GL_FALSE);
		for (int i = 0; i < m_csetCount; ++i)
            if(i == m_tile)
			duDebugDrawContours(&dd, *m_cset[i]);
		glDepthMask(GL_TRUE);
	}

	if (m_drawMode == DRAWMODE_REGION_CONNECTIONS)
	{
		for (int i = 0; i < m_chfCount; ++i)
            if(i == m_tile)
			duDebugDrawCompactHeightfieldRegions(&dd, *m_chf[i]);
		
		glDepthMask(GL_FALSE);
		for (int i = 0; i < m_csetCount; ++i)
            if(i == m_tile)
			duDebugDrawRegionConnections(&dd, *m_cset[i]);
		glDepthMask(GL_TRUE);
	}

	if (/*m_pmesh &&*/ m_drawMode == DRAWMODE_POLYMESH)
	{
		glDepthMask(GL_FALSE);
        for(int i = 0; i < m_pmeshCount; ++i)
            if(i == m_tile)
		    duDebugDrawPolyMesh(&dd, *m_pmeshes[i]);
		glDepthMask(GL_TRUE);
	}
	if (/*m_dmesh &&*/ m_drawMode == DRAWMODE_POLYMESH_DETAIL)
	{
		glDepthMask(GL_FALSE);
        for(int i = 0; i < m_dmeshCount; ++i)
            if(i == m_tile)
		    duDebugDrawPolyMeshDetail(&dd, *m_dmeshes[i]);
		glDepthMask(GL_TRUE);
	}
	
	m_geom->drawConvexVolumes(&dd);

	if (m_tool)
		m_tool->handleRender();

	glDepthMask(GL_TRUE);
}

void Sample_Debug::handleRenderOverlay(double* proj, double* model, int* view)
{
    Sample_SoloMeshTiled::handleRenderOverlay(proj, model, view);
}

void Sample_Debug::handleMeshChanged(InputGeom* geom)
{
    Sample_SoloMeshTiled::handleMeshChanged(geom);
}

const float* Sample_Debug::getBoundsMin()
{
    return Sample_SoloMeshTiled::getBoundsMin();
}

const float* Sample_Debug::getBoundsMax()
{
    return Sample_SoloMeshTiled::getBoundsMax();
}

void Sample_Debug::handleClick(const float* p, bool shift)
{
    Sample_SoloMeshTiled::handleClick(NULL, p, shift);
}

void Sample_Debug::handleStep()
{
    Sample_SoloMeshTiled::handleStep();
}

bool Sample_Debug::handleBuild()
{
    char sMapID[5];
    sprintf(sMapID, "%.3s", m_meshName);
    int mapID = atoi(sMapID);

    cleanup();

    duReadNavMesh(mapID, m_navMesh);
    m_pmeshCount = duReadPolyMesh(mapID, m_pmeshes);
    m_hfCount = duReadHeightfield(mapID, m_hf);
    m_chfCount = duReadCompactHeightfield(mapID, m_chf);
    m_dmeshCount = duReadDetailMesh(mapID, m_dmeshes);
    m_csetCount = duReadContourSet(mapID, m_cset);

    if(m_navMesh)
    {
        m_drawMode = DRAWMODE_NAVMESH_TRANS;
        m_tileSize = m_navMesh->getParams()->tileHeight;

        m_navQuery = dtAllocNavMeshQuery();
        m_navQuery->init(m_navMesh, 2048);
    }

    if(m_tool)
        m_tool->init(this);

    return true;
}

void Sample_Debug::setHighlightedTile(const float* pos)
{

	if (!pos || !m_navMesh)
	{
		m_highLightedTileX = -1;
		m_highLightedTileY = -1;
		return;
	}

    dtNavMeshQuery* query = dtAllocNavMeshQuery();
    query->init(m_navMesh, 2048);

    float extents[3] = {2.f, 4.f, 2.f};
    dtPolyRef polyRef = query->findNearestPoly(pos, extents, &dtQueryFilter(), 0);
    unsigned char area = m_navMesh->getPolyArea(polyRef);
    unsigned short flags = m_navMesh->getPolyFlags(polyRef);

    const dtMeshTile* tile;
    const dtPoly* poly;
    m_navMesh->getTileAndPolyByRef(polyRef, &tile, &poly);

    if(!tile)
    {
		m_highLightedTileX = -1;
		m_highLightedTileY = -1;
		return;
    }
    m_highLightedTileX = tile->header->x;
	m_highLightedTileY = tile->header->y;
}
