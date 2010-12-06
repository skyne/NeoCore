
#ifndef _MMAP_DEBUG_H
#define _MMAP_DEBUG_H

#include <math.h>
#include "DetourNavMesh.h"
#include "Recast.h"
#include "ChunkyTriMesh.h"
#include "MeshLoaderObj.h"

enum NavTerrain
{
    NAV_GROUND  = 0x01,
    NAV_MAGMA   = 0x02,
    NAV_SLIME   = 0x04,
    NAV_WATER   = 0x08,
    NAV_UNUSED1 = 0x10,
    NAV_UNUSED2 = 0x20
    // we only have 6 bits of a bitfield
};

//void duReadObjMesh(int mapID, rcInputGeom* geom);
void duReadNavMesh(int mapID, dtNavMesh* &navMesh);
int duReadHeightfield(int mapID, rcHeightfield** &hf);
int duReadCompactHeightfield(int mapID, rcCompactHeightfield** &chf);
int duReadContourSet(int mapID, rcContourSet** &cs);
int duReadPolyMesh(int mapID, rcPolyMesh** &mesh);
int duReadDetailMesh(int mapID, rcPolyMeshDetail** &mesh);

class myMeshLoaderObj
{
private:
	float* m_verts;
	int* m_tris;
	float* m_normals;
	int m_vertCount;
	int m_triCount;
	char m_filename[260];

public:
	myMeshLoaderObj();
	~myMeshLoaderObj();
	
	bool load(const char* fileName);

	inline const float* getVerts() const { return m_verts; }
	inline const float* getNormals() const { return m_normals; }
	inline const int* getTris() const { return m_tris; }
	inline int getVertCount() const { return m_vertCount; }
	inline int getTriCount() const { return m_triCount; }
	inline const char* getFileName() const { return m_filename; }
};

#endif
