#ifndef _MAP_BUILDER_2_H
#define _MAP_BUILDER_2_H

#include <vector>
#include <set>
#include <map>

#include "TileBuilder.h"
#include "IVMapManager.h"
#include "G3D/Array.h"

#include "..\..\..\dep\recastnavigation\Recast\Include\Recast.h"
#include "..\..\..\dep\recastnavigation\Detour\Include\DetourNavMesh.h"
#include "..\..\..\dep\recastnavigation\Detour\Include\DetourNavMeshBuilder.h"
#include "..\..\..\dep\recastnavigation\Detour\Include\DetourCommon.h"

using namespace std;
using namespace VMAP;
// G3D namespace typedefs conflicts with ACE typedefs

namespace MMAP
{
    typedef set<uint32> MapList;
    typedef map<uint32,set<uint32>*> TileList;

    struct IntermediateValues
    {
        rcHeightfield* heightfield;
        unsigned char* triFlags;
        rcCompactHeightfield* compactHeightfield;
        rcContourSet* contours;
        rcPolyMesh* polyMesh;
        rcPolyMeshDetail* polyMeshDetail;
    };

    class MapBuilder
    {
        public:
            MapBuilder(float maxWalkableAngle   = 60.f,
                       bool skipLiquid          = true,
                       bool skipContinents      = true,
                       bool skipJunkMaps        = true,
                       bool skipBattlegrounds   = true,
                       bool hiResHeightmaps     = false,
                       bool debugOutput         = false);

            ~MapBuilder();

            /**
             Builds a mmap for the specifiec map id.
             * First, the entire vmap is loaded. Cannot do it tile-by-tile because vmap tiles only load models
               whose origin is in that tile.  Large models may span across tiles (stormwind, etc)

             * Second, iterates through the tiles and loads their heightmaps.
               These are processed so that steep inclines are removed.
               TODO: process liquid heightmap

             * Third, the vmap model and heightmap data is aggregated

             * Fourth, data is sent off to recast for processing.  This optionally includes generating
               an obj file, for debugging with RecastDemo
            */
            void build(uint32 mapID);

            // generates an obj file for the specified map tile
            void buildTile(uint32 mapID, uint32 tileX, uint32 tileY);

            // builds list of maps, then iterates through them calling build(uint32 mapID)
            void buildAll();

        private:
            // detect maps and tiles
            void getTileList(uint32 mapID);
            void getMapList();

            // load and unload models
            void loadEntireVMap(uint32 mapID);
            void loadVMap(uint32 mapID, uint32 tileX, uint32 tileY, G3D::Array<float> &modelVerts, G3D::Array<int> &modelTris);
            void unloadEntireVMap(uint32 mapID);
            void unloadVMap(uint32 mapID, uint32 tileX, uint32 tileY);

            // vert and triangle methods
            void transform(vector<Vector3> original, vector<Vector3> &transformed,
                           float scale, G3D::Matrix3 rotation, Vector3 position);
            void copyVertices(vector<Vector3> source, G3D::Array<float> &dest);
            void copyIndices(vector<MeshTriangle> source, G3D::Array<int> &dest, int offest, bool flip);
            void copyIndices(G3D::Array<int> &dest, G3D::Array<int> src, int offset);
            void cleanVertices(G3D::Array<float> &verts, G3D::Array<int> &tris);

            void cleanup();

            // move map building
            void buildMoveMap(uint32 mapID);
            void buildNavMesh(uint32 mapID, dtNavMesh* &navMesh);
            void buildMoveMapTile(uint32 mapID,
                                  uint32 tileX,
                                  uint32 tileY,
                                  MeshData meshData,
                                  float* bmin,
                                  float* bmax,
                                  dtNavMesh* navMesh);

            void getTileBounds(uint32 tileX, uint32 tileY,
                               float* verts, int vertCount,
                               float* bmin, float* bmax);

            void initIntermediateValues(IntermediateValues &iv);
            void clearIntermediateValues(IntermediateValues &iv);

            float snapToGrid(const float coord);

            bool shouldSkipMap(uint32 mapID);
            bool isTransportMap(uint32 mapID);

            // debug output
            void generateObjFile(uint32 mapID, uint32 tileX, uint32 tileY, MeshData meshData);
            void generateRealObj(uint32 mapID, uint32 tileX, uint32 tileY, MeshData meshData);
            void writeIV(uint32 mapID, uint32 tileX, uint32 tileY, IntermediateValues iv);
            void writeHeightfield(FILE* file, const rcHeightfield* hf);
            void writeSpan(FILE* file, const rcSpan* span);
            void writeCompactHeightfield(FILE* file, const rcCompactHeightfield* chf);
            void writeContours(FILE* file, const rcContourSet* cs);
            void writePolyMesh(FILE* file, const rcPolyMesh* mesh);
            void writeDetailMesh(FILE* file, const rcPolyMeshDetail* mesh);

            IVMapManager* m_vmapManager;
            TileBuilder* m_tileBuilder;

            MapList m_maps;
            TileList m_tiles;

            bool m_debugOutput;

            bool m_skipContinents;
            bool m_skipJunkMaps;
            bool m_skipBattlegrounds;

            float m_maxWalkableAngle;
    };
}

#endif