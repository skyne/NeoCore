#ifndef _MMAP_TILE_BUILDER_H
#define _MMAP_TILE_BUILDER_H

#include "MMapCommon.h"
#include "MangosMap.h"
#include "IVMapManager.h"
#include "WorldModel.h"

#include "G3D/Array.h"
#include "G3D/Vector3.h"
#include "G3D/Matrix3.h"

using namespace VMAP;
using namespace MaNGOS;

namespace MMAP
{
    enum Spot
    {
        TOP     = 1,
        RIGHT   = 2,
        LEFT    = 3,
        BOTTOM  = 4,
        ENTIRE  = 5
    };

    enum Grid
    {
        GRID_V8,
        GRID_V9
    };

    static const int V9_SIZE = 129;
    static const int V9_SIZE_SQ = V9_SIZE*V9_SIZE;
    static const int V8_SIZE = 128;
    static const int V8_SIZE_SQ = V8_SIZE*V8_SIZE;
    static const float GRID_SIZE = 533.33333f;
    static const float GRID_PART_SIZE = (float)1/V8_SIZE*GRID_SIZE;

    struct MeshData
    {
        G3D::Array<float> solidVerts;
        G3D::Array<int> solidTris;

        G3D::Array<float> liquidVerts;
        G3D::Array<int> liquidTris;
        G3D::Array<uint8> liquidType;
    };

    // see also game/src/PathFinder.h
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

    class TileBuilder
    {
        public:
            TileBuilder(bool skipLiquid, bool hiRes);
            ~TileBuilder();

            void loadMap(uint32 mapID, uint32 tileX, uint32 tileY, MeshData &meshData);

        private:

            // general
            bool loadMap(uint32 mapID, uint32 tileX, uint32 tileY, MeshData &meshData, Spot portion);
            void getLoopVars(Spot portion, int &loopStart, int &loopEnd, int &loopInc);

            // terrain
            bool m_hiResHeightMaps;

            bool loadHeightMap(uint32 mapID, uint32 tileX, uint32 tileY, G3D::Array<float> &vertices, G3D::Array<int> &triangles, Spot portion);
            void getHeightCoord(int index, Grid grid, float xOffset, float yOffset, float* coord, float* v);
            void getHeightTriangle(int square, Spot triangle, int* indices, bool liquid = false);
            bool isHole(int square, const uint16 holes[16][16]);

            // liquid
            bool m_skipLiquid;

            bool loadLiquidMap(uint32 mapID, uint32 tileX, uint32 tileY, G3D::Array<float> &vertices, G3D::Array<int> &triangles, Spot portion);
            void getLiquidCoord(int index, int index2, float xOffset, float yOffset, float* coord, float* v);
            void getLiquidTriangle(int square, Spot triangle, int* indices, uint8 width);
            uint8 getLiquidType(int square, const uint8 liquid_type[16][16]);

            // hide parameterless constructor
            TileBuilder();
    };
}

#endif
