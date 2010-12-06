
#include <set>
#include "MMapCommon.h"
#include "MapBuilder.h"
#include "VMapManager2.h"
#include "MapTree.h"
#include "ModelInstance.h"

/*#include "Recast.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMesh.h"
#include "DetourCommon.h"*/

using namespace std;

namespace MMAP
{
    MapBuilder::MapBuilder(float maxWalkableAngle,
                           bool skipLiquid,
                           bool skipContinents, bool skipJunkMaps, bool skipBattlegrounds,
                           bool hiResHeightmaps, bool debugOutput) :
                           m_maxWalkableAngle   (maxWalkableAngle),
                           m_skipContinents     (skipContinents),
                           m_skipJunkMaps       (skipJunkMaps),
                           m_skipBattlegrounds  (skipBattlegrounds),
                           m_debugOutput        (debugOutput)
    {
        m_vmapManager = new VMapManager2();
        m_tileBuilder = new TileBuilder(skipLiquid, hiResHeightmaps);
    }

    MapBuilder::~MapBuilder()
    {
        m_maps.clear();

        for(TileList::iterator it = m_tiles.begin(); it != m_tiles.end(); ++it)
        {
            (*it).second->clear();
            delete (*it).second;
        }
        m_tiles.clear();

        delete m_tileBuilder;
        delete m_vmapManager;
    }

    void MapBuilder::getTileList(uint32 mapID)
    {
        uint32 i;
        set<uint32>* tiles;
        vector<string> files;
        uint32 tileX, tileY, tileID;
        char filter[12];

        if(m_maps.find(mapID) == m_maps.end())
            getMapList();

        if(m_maps.find(mapID) == m_maps.end())
            return;

        sprintf(filter, "%03u*.vmtile", mapID);
        getDirContents(files, "vmaps", filter);
        for(i = 0; i < files.size(); ++i)
        {
            tileX = uint32(atoi(files[i].substr(7,2).c_str()));
            tileY = uint32(atoi(files[i].substr(4,2).c_str()));
            tileID = StaticMapTree::packTileID(tileY, tileX);

            if(m_tiles.find(mapID) == m_tiles.end())
            {
                tiles = new set<uint32>;
                m_tiles.insert(pair<uint32,set<uint32>*>(mapID, tiles));
            }
            else
                tiles = m_tiles[mapID];

            tiles->insert(tileID);
        }

        sprintf(filter, "%03u*", mapID);
        files.clear();
        getDirContents(files, "maps", filter);
        for(i = 0; i < files.size(); ++i)
        {
            tileY = uint32(atoi(files[i].substr(3,2).c_str()));
            tileX = uint32(atoi(files[i].substr(5,2).c_str()));
            tileID = StaticMapTree::packTileID(tileX, tileY);

            if(m_tiles.find(mapID) == m_tiles.end())
            {
                tiles = new set<uint32>;
                m_tiles.insert(pair<uint32,set<uint32>*>(mapID, tiles));
            }
            else
                tiles = m_tiles[mapID];

            tiles->insert(tileID);
        }
    }

    void MapBuilder::getMapList()
    {
        uint32 i;
        vector<string> files;
        uint32 mapID;

        getDirContents(files, "vmaps", "*.vmtree");
        for(i = 0; i < files.size(); ++i)
        {
            mapID = uint32(atoi(files[i].substr(0,3).c_str()));
            m_maps.insert(mapID);
        }

        files.clear();
        getDirContents(files, "maps");
        for(i = 0; i < files.size(); ++i)
        {
            mapID = uint32(atoi(files[i].substr(0,3).c_str()));
            m_maps.insert(mapID);
        }
    }

    void MapBuilder::buildAll()
    {
        getMapList();

        for(MapList::iterator it = m_maps.begin(); it != m_maps.end(); ++it)
            if(!shouldSkipMap((*it)))
                build((*it));
    }

    void MapBuilder::build(uint32 mapID)
    {
        printf("Building map %03u:\n", mapID);

        getTileList(mapID);

        set<uint32>* tiles;
        if(m_tiles.find(mapID) != m_tiles.end())
            tiles = m_tiles[mapID];
        else
        {
            tiles = new set<uint32>;
            m_tiles.insert(std::pair<uint32,set<uint32>*>(mapID, tiles));
        }

        // vars that are used in multiple locations...
        uint32 i, j, tileX, tileY;
        float bmin[3], bmax[3];

        // scope the model data arrays
        do
        {
            G3D::Array<float> modelVerts;
            G3D::Array<int> modelTris;

            // make sure we process maps which don't have tiles
            if(!tiles->size())
            {
                // initialize the static tree, which loads WDT models
                loadVMap(mapID, 64, 64, modelVerts, modelTris);

                if(!modelVerts.size())
                    break;

                // get the coord bounds of the model data
                rcCalcBounds(modelVerts.getCArray(), modelVerts.size() / 3, bmin, bmax);

                // convert coord bounds to grid bounds
                uint32 minX, minY, maxX, maxY;
                maxX = 32 - bmin[0] / GRID_SIZE;
                maxY = 32 - bmin[2] / GRID_SIZE;
                minX = 32 - bmax[0] / GRID_SIZE;
                minY = 32 - bmax[2] / GRID_SIZE;

                // add all tiles within bounds to tile list.
                for(i = minX; i <= maxX; ++i)
                    for(j = minY; j <= maxY; ++j)
                        tiles->insert(StaticMapTree::packTileID(i, j));
            }
        } while(0);

        // build navMesh
        dtNavMesh* navMesh = 0;
        buildNavMesh(mapID, navMesh);
        if(!navMesh)
        {
            printf("Failed creating navmesh!              \n");
            printf("Failed!                               \n\n");
            return;
        }

        // more data storage
        G3D::Array<float> allVerts;
        G3D::Array<int> allTris;
        char tileString[10];
        
        // now start building mmtiles for each tile
        printf("We have %i tiles.                          \n", tiles->size());
        for(set<uint32>::iterator it = tiles->begin(); it != tiles->end(); ++it)
        {
            allVerts.fastClear();
            allTris.fastClear();

            MeshData meshData;

            // unpack tile coords
            StaticMapTree::unpackTileID((*it), tileX, tileY);
            sprintf(tileString, "[%02u,%02u]: ", tileX, tileY);

            // get heightmap data
            printf("%sLoading heightmap...                           \n", tileString);
            m_tileBuilder->loadMap(mapID, tileX, tileY, meshData);

            // get model data
            printf("%sLoading models...                              \n", tileString);
            loadVMap(mapID, tileY, tileX, meshData.solidVerts, meshData.solidTris);
            unloadVMap(mapID, tileY, tileX);

            // we only want tiles that people can actually walk on
            if(!meshData.solidVerts.size() && !meshData.liquidVerts.size())
                continue;

            printf("%sAggregating mesh data...                        \n", tileString);

            // remove unused vertices
            cleanVertices(meshData.solidVerts, meshData.solidTris);
            cleanVertices(meshData.liquidVerts, meshData.liquidTris);

            // gather all mesh data for final data check, and bounds calculation
            allTris.append(meshData.liquidTris);
            allVerts.append(meshData.liquidVerts);
            copyIndices(allTris, meshData.solidTris, allVerts.size() / 3);
            allVerts.append(meshData.solidVerts);
            
            if(!allVerts.size() || !allTris.size())
                continue;

            // get bounds of current tile
            getTileBounds(tileX, tileY, allVerts.getCArray(), allVerts.size() / 3, bmin, bmax);

            allVerts.fastClear();
            allTris.fastClear();

            // build navmesh tile
            buildMoveMapTile(mapID,
                             tileX,
                             tileY,
                             meshData,
                             bmin,
                             bmax,
                             navMesh);
        }

        unloadEntireVMap(mapID);

        DELETE(navMesh);

        printf("Complete!                               \n\n");
    }

    void MapBuilder::buildTile(uint32 mapID, uint32 tileX, uint32 tileY)
    {
        printf("Building map %03u, tile [%02u,%02u]\n", mapID, tileX, tileY);

        float bmin[3], bmax[3];
        G3D::Array<float> modelVerts;
        G3D::Array<int> modelTris;

        // make sure we process maps which don't have tiles
        // initialize the static tree, which loads WDT models
        loadVMap(mapID, 64, 64, modelVerts, modelTris);

        // get the coord bounds of the model data
        if(modelVerts.size())
        {
            rcCalcBounds(modelVerts.getCArray(), modelVerts.size() / 3, bmin, bmax);

            // convert coord bounds to grid bounds
            uint32 minX, minY, maxX, maxY;
            maxX = 32 - bmin[0] / GRID_SIZE;
            maxY = 32 - bmin[2] / GRID_SIZE;
            minX = 32 - bmax[0] / GRID_SIZE;
            minY = 32 - bmax[2] / GRID_SIZE;

            // if specified tile is outside of bounds, give up now
            if(tileX < minX || tileX > maxX)
                return;
            if(tileY < minY || tileY > maxY)
                return;
        }

        // build navMesh
        dtNavMesh* navMesh = 0;
        buildNavMesh(mapID, navMesh);
        if(!navMesh)
        {
            printf("Failed creating navmesh!              \n");
            printf("Failed!                               \n\n");
            return;
        }

        G3D::Array<float> allVerts;
        G3D::Array<int> allTris;
        char tileString[10];
        sprintf(tileString, "[%02u,%02u]: ", tileX, tileY);

        do
        {
            MeshData meshData;

            // get heightmap data
            printf("%sLoading heightmap...                           \r", tileString);
            m_tileBuilder->loadMap(mapID, tileX, tileY, meshData);

            // get model data
            printf("%sLoading models...                              \r", tileString);
            loadVMap(mapID, tileY, tileX, meshData.solidVerts, meshData.solidTris);
            unloadVMap(mapID, tileY, tileX);

            // if there is no data, give up now
            if(!meshData.solidVerts.size() && !meshData.liquidVerts.size())
                break;

            printf("%sAggregating mesh data...                        \r", tileString);

            // remove unused vertices
            cleanVertices(meshData.solidVerts, meshData.solidTris);
            cleanVertices(meshData.liquidVerts, meshData.liquidTris);

            // gather all mesh data for final data check, and bounds calculation
            allTris.append(meshData.liquidTris);
            allVerts.append(meshData.liquidVerts);
            copyIndices(allTris, meshData.solidTris, allVerts.size() / 3);
            allVerts.append(meshData.solidVerts);

            if(!allVerts.size() || !allTris.size())
                break;

            // get bounds of current tile
            getTileBounds(tileX, tileY, allVerts.getCArray(), allVerts.size() / 3, bmin, bmax);

            allVerts.clear();
            allTris.clear();

            // build navmesh tile
            buildMoveMapTile(mapID,
                                tileX,
                                tileY,
                                meshData,
                                bmin,
                                bmax,
                                navMesh);

        } while(0);

        printf("%sComplete!                                      \n\n", tileString);
    }

    void MapBuilder::loadEntireVMap(uint32 mapID)
    {
        printf("Loading vmap...                         \r");

        // any loadMap call initializes the StaticMapTree, which loads
        // models stored in the WDT. Necessary for maps that don't have
        // a heightmap or ADTs
        cout.setstate(cout.badbit);
        m_vmapManager->loadMap("vmaps", mapID, 64, 64);
        cout.clear(cout.goodbit);

        set<uint32> tiles;
        if(m_tiles.find(mapID) != m_tiles.end())
            tiles = *m_tiles[mapID];

        uint32 tileX, tileY;
        for(set<uint32>::iterator it = tiles.begin(); it != tiles.end(); ++it)
        {
            StaticMapTree::unpackTileID((*it), tileX, tileY);

            cout.setstate(cout.badbit);
            int result = m_vmapManager->loadMap("vmaps", mapID, tileX, tileY);
            cout.clear(cout.goodbit);
        }

        ModelInstance* models = 0;
        uint32 count = 0;

        InstanceTreeMap instanceTrees;
        ((VMapManager2*)m_vmapManager)->getInstanceMapTree(instanceTrees);

        if(!instanceTrees[mapID])
            return;

        instanceTrees[mapID]->getModelInstances(models, count);

        if(!models || !count)
            return;

        uint32 i;
        for(i = 0; i < count; ++i)
        {
            ModelInstance instance = models[i];

            G3D::Array<float>* modelVertices = NEW(G3D::Array<float>);
            G3D::Array<int>* modelTriangles = NEW(G3D::Array<int>);

            // model instances exist in tree even though there are instances of that model in this tile
            WorldModel* worldModel = instance.getWorldModel();
            if(!worldModel)
                continue;

            vector<GroupModel> groupModels;
            worldModel->getGroupModels(groupModels);

            // all M2s need to have triangle indices reversed
            bool isM2 = instance.name.find(".m2") != instance.name.npos || instance.name.find(".M2") != instance.name.npos;

            // transform data
            float scale = models[i].iScale;
            G3D::Matrix3 rotation = G3D::Matrix3::fromEulerAnglesZYX(-1*G3D::pi()*instance.iRot.y/180.f, -1*G3D::pi()*instance.iRot.x/180.f, -1*G3D::pi()*instance.iRot.z/180.f);
            Vector3 position = instance.iPos;
            position.x -= 32*533.33333f;
            position.y -= 32*533.33333f;

            for(vector<GroupModel>::iterator it = groupModels.begin(); it != groupModels.end(); ++it)
            {
                vector<Vector3> tempVertices;
                vector<Vector3> transformedVertices;
                vector<MeshTriangle> tempTriangles;

                (*it).getMeshData(tempVertices, tempTriangles);

                transform(tempVertices, transformedVertices, scale, rotation, position);

                int offset = modelVertices->size() / 3;

                copyVertices(transformedVertices, (*modelVertices));
                copyIndices(tempTriangles, (*modelTriangles), offset, isM2);
            }
        }

        unloadEntireVMap(mapID);
    }

    void MapBuilder::loadVMap(uint32 mapID, uint32 tileX, uint32 tileY, G3D::Array<float> &modelVertices, G3D::Array<int> &modelTriangles)
    {
        cout.setstate(cout.badbit);
        int result = m_vmapManager->loadMap("vmaps", mapID, tileX, tileY);
        cout.clear(cout.goodbit);

        ModelInstance* models = 0;
        uint32 count = 0;

        InstanceTreeMap instanceTrees;
        ((VMapManager2*)m_vmapManager)->getInstanceMapTree(instanceTrees);

        if(!instanceTrees[mapID])
            return;

        instanceTrees[mapID]->getModelInstances(models, count);

        if(!models || !count)
            return;

        uint32 i;
        for(i = 0; i < count; ++i)
        {
            ModelInstance instance = models[i];

            // model instances exist in tree even though there are instances of that model in this tile
            WorldModel* worldModel = instance.getWorldModel();
            if(!worldModel)
                continue;

            vector<GroupModel> groupModels;
            worldModel->getGroupModels(groupModels);

            // all M2s need to have triangle indices reversed
            bool isM2 = instance.name.find(".m2") != instance.name.npos || instance.name.find(".M2") != instance.name.npos;

            // transform data
            float scale = models[i].iScale;
            G3D::Matrix3 rotation = G3D::Matrix3::fromEulerAnglesZYX(-1*G3D::pi()*instance.iRot.y/180.f, -1*G3D::pi()*instance.iRot.x/180.f, -1*G3D::pi()*instance.iRot.z/180.f);
            Vector3 position = instance.iPos;
            position.x -= 32*533.33333f;
            position.y -= 32*533.33333f;

            for(vector<GroupModel>::iterator it = groupModels.begin(); it != groupModels.end(); ++it)
            {
                vector<Vector3> tempVertices;
                vector<Vector3> transformedVertices;
                vector<MeshTriangle> tempTriangles;

                (*it).getMeshData(tempVertices, tempTriangles);

                transform(tempVertices, transformedVertices, scale, rotation, position);

                int offset = modelVertices.size() / 3;

                copyVertices(transformedVertices, modelVertices);
                copyIndices(tempTriangles, modelTriangles, offset, isM2);
            }
        }
    }

    void MapBuilder::unloadEntireVMap(uint32 mapID)
    {
        printf("Unloading vmap...                       \r");

        set<uint32> tiles;
        if(m_tiles.find(mapID) != m_tiles.end())
            tiles = *m_tiles[mapID];

        uint32 tileX, tileY;
        for(set<uint32>::iterator it = tiles.begin(); it != tiles.end(); ++it)
        {
            StaticMapTree::unpackTileID((*it), tileX, tileY);

            cout.setstate(cout.badbit);
            m_vmapManager->unloadMap(mapID, tileX, tileY);
            cout.clear(cout.goodbit);
        }
        
        cout.setstate(cout.badbit);
        m_vmapManager->unloadMap(mapID);
        cout.clear(cout.goodbit);
    }

    void MapBuilder::unloadVMap(uint32 mapID, uint32 tileX, uint32 tileY)
    {
        printf("Unloading vmap...                       \r");

        cout.setstate(cout.badbit);
        m_vmapManager->unloadMap(mapID, tileX, tileY);
        cout.clear(cout.goodbit);
    }

    inline void MapBuilder::transform(vector<Vector3> source, vector<Vector3> &transformedVertices, float scale, G3D::Matrix3 rotation, Vector3 position)
    {
        for(vector<Vector3>::iterator it = source.begin(); it != source.end(); ++it)
        {
            // apply tranform, then mirror along the horizontal axes
            Vector3 v((*it) * rotation * scale + position);
            v.x *= -1.f;
            v.y *= -1.f;
            transformedVertices.push_back(v);
        }
    }

    inline void MapBuilder::copyVertices(vector<Vector3> source, G3D::Array<float> &dest)
    {
        for(vector<Vector3>::iterator it = source.begin(); it != source.end(); ++it)
        {
            dest.push_back((*it).y);
            dest.push_back((*it).z);
            dest.push_back((*it).x);
        }
    }

    inline void MapBuilder::copyIndices(vector<MeshTriangle> source, G3D::Array<int> &dest, int offset, bool flip)
    {
        if(flip)
        {
            for(vector<MeshTriangle>::iterator it = source.begin(); it != source.end(); ++it)
            {
                dest.push_back((*it).idx2+offset);
                dest.push_back((*it).idx1+offset);
                dest.push_back((*it).idx0+offset);
            }
        }
        else
        {
            for(vector<MeshTriangle>::iterator it = source.begin(); it != source.end(); ++it)
            {
                dest.push_back((*it).idx0+offset);
                dest.push_back((*it).idx1+offset);
                dest.push_back((*it).idx2+offset);
            }
        }
    }

    inline void MapBuilder::copyIndices(G3D::Array<int> &dest, G3D::Array<int> source, int offset)
    {
        int* src = source.getCArray();
        for(int i = 0; i < source.size(); ++i)
            dest.append(src[i] + offset);
    }

    void MapBuilder::cleanVertices(G3D::Array<float> &verts, G3D::Array<int> &tris)
    {
        map<int, int> vertMap;

        int* t = tris.getCArray();
        float* v = verts.getCArray();

        // collect all the vertex indices from triangle
        for(int i = 0; i < tris.size(); ++i)
        {
            if(vertMap.find(t[i]) != vertMap.end())
                continue;

            vertMap.insert(std::pair<int, int>(t[i], 0));
        }

        // collect the vertices
        G3D::Array<float> cleanVerts;
        int index, count = 0;
        for(map<int, int>::iterator it = vertMap.begin(); it != vertMap.end(); ++it)
        {
            index = (*it).first;
            (*it).second = count;
            cleanVerts.append(v[index*3], v[index*3+1], v[index*3+2]);
            count++;
        }
        verts.fastClear();
        verts.append(cleanVerts);
        cleanVerts.clear();

        // update triangles to use new indices
        for(int i = 0; i < tris.size(); ++i)
        {
            map<int, int>::iterator it;
            if((it = vertMap.find(t[i])) == vertMap.end())
                continue;

            t[i] = (*it).second;
        }

        vertMap.clear();
    }

    void MapBuilder::buildNavMesh(uint32 mapID, dtNavMesh* &navMesh)
    {
        getTileList(mapID);

        set<uint32> tiles;
        if(m_tiles.find(mapID) != m_tiles.end())
            tiles = *m_tiles[mapID];

        char fileName[25];
        FILE* file = 0;

        /*** calculate number of bits needed to store tiles & polys ***/
        int tileBits = rcMin((int)dtIlog2(dtNextPow2(tiles.size())), 6);    // 6 bits is enough for 4096 tiles
        if (tileBits < 1) tileBits = 1;                                     // need at least one bit!
        int polyBits = 22 - tileBits;
        int maxTiles = 1 << tileBits;
        int maxPolysPerTile = 1 << polyBits;

        /***          calculate bounds of map         ***/

        uint32 tileXMin = 64, tileYMin = 64, tileXMax = 0, tileYMax = 0, tileX, tileY;
        for(set<uint32>::iterator it = tiles.begin(); it != tiles.end(); ++it)
        {
            StaticMapTree::unpackTileID((*it), tileX, tileY);

            if(tileX > tileXMax)
                tileXMax = tileX;
            else if(tileX < tileXMin)
                tileXMin = tileX;

            if(tileY > tileYMax)
                tileYMax = tileY;
            else if(tileY < tileYMin)
                tileYMin = tileY;
        }

        float bmin[3], bmax[3];

        // use Max because '32 - tileX' is negative for values over 32
        getTileBounds(tileXMax, tileYMax, NULL, 0, bmin, bmax);

        /***       now create the navmesh       ***/

        // navmesh creation params
        dtNavMeshParams navMeshParams;
        memset(&navMeshParams, 0, sizeof(dtNavMeshParams));
        navMeshParams.tileWidth = GRID_SIZE;
        navMeshParams.tileHeight = GRID_SIZE;
        rcVcopy(navMeshParams.orig, bmin);
        navMeshParams.maxTiles = maxTiles;
        navMeshParams.maxPolys = maxPolysPerTile;

        navMesh = NEW(dtNavMesh);
        printf("Creating navMesh...                     \r");
        if(!navMesh->init(&navMeshParams))
        {
            printf("Failed creating navmesh!                \n");
            return;
        }

        sprintf(fileName, "mmaps/%03u.mmap", mapID);
        if(!(file = fopen(fileName, "wb")))
        {
            DELETE(navMesh);
            char message[1024];
            sprintf(message, "Failed to open %s for writing!\n", fileName);
            perror(message);
            return;
        }

        // now that we know navMesh params are valid, we can write them to file
        fwrite(&navMeshParams, sizeof(dtNavMeshParams), 1, file);
        fclose(file);
    }

    void MapBuilder::buildMoveMapTile(uint32 mapID, uint32 tileX, uint32 tileY,
                                      MeshData meshData, float* bmin, float* bmax,
                                      dtNavMesh* navMesh)
    {
        // console output
        char tileString[10];
        sprintf(tileString, "[%02i,%02i]: ", tileX, tileY);

        // file output
        char fileName[255];
        FILE* file = 0;

        float cellSize = .5f;       // larger number => less voxels => faster build time
                                    // too large, and tight spaces won't be pathable.
        float agentHeight = 1.5f;
        float agentRadius = .1f;
        float agentMaxClimb = 1.65f;

        IntermediateValues iv;
        initIntermediateValues(iv);

        float* tVerts = meshData.solidVerts.getCArray();
        int tVertCount = meshData.solidVerts.size() / 3;
        int* tTris = meshData.solidTris.getCArray();
        int tTriCount = meshData.solidTris.size() / 3;

        float* lVerts = meshData.liquidVerts.getCArray();
        int lVertCount = meshData.liquidVerts.size() / 3;
        int* lTris = meshData.liquidTris.getCArray();
        int lTriCount = meshData.liquidTris.size() / 3;
        uint8* lTriFlags = meshData.liquidType.getCArray();
        do
        {
            /***     init config       ***/
            rcConfig config;
            memset(&config, 0, sizeof(rcConfig));
            config.maxVertsPerPoly = 6;

            // these are WORLD UNIT based metrics
            config.cs = cellSize;
            config.ch = .3f;
            config.walkableSlopeAngle = m_maxWalkableAngle;
            rcVcopy(config.bmin, bmin);
            rcVcopy(config.bmax, bmax);

            // these are VOXEL-based metrics
            rcCalcGridSize(config.bmin, config.bmax, config.cs, &config.width, &config.height);
            config.tileSize = (int)ceilf(GRID_SIZE / config.cs);
            config.walkableRadius = (int)ceilf(agentRadius / config.cs);
            config.borderSize = config.walkableRadius + 3;
            config.maxEdgeLen = 1500;
            config.walkableHeight = (int)ceilf(agentHeight / config.ch);
            config.walkableClimb = (int)ceilf(agentHeight / config.ch);
            config.minRegionSize = (int)rcSqr(50);
            config.mergeRegionSize = (int)rcSqr(20);
            config.maxSimplificationError = 1.3f;
            config.detailSampleDist = config.cs * 16.f;
            config.detailSampleMaxError = config.ch * 1.f;

            // pad bounds with a border
            config.bmin[0] -= config.borderSize*config.cs;
            config.bmin[2] -= config.borderSize*config.cs;
            config.bmax[0] += config.borderSize*config.cs;
            config.bmax[2] += config.borderSize*config.cs;

            // this sets the dimensions of the heightfield - should maybe happen before border padding
            rcCalcGridSize(config.bmin, config.bmax, config.cs, &config.width, &config.height);

            // build performance
            rcBuildContext context;

            /***********    start build     ***********/

            // build heightfield
            printf("%sBuilding Recast Heightfield...          \r", tileString);
            iv.heightfield = rcAllocHeightfield();
            if(!iv.heightfield || !rcCreateHeightfield(&context, *iv.heightfield, config.width, config.height, config.bmin, config.bmax, config.cs, config.ch))
            {
                printf("%sFailed building heightfield!            \n", tileString);
                break;
            }

            printf("%sRasterizing triangles...                   \r", tileString);

            // flag walkable terrain triangles
            iv.triFlags = NEW_ARRAY(unsigned char,tTriCount);
            memset(iv.triFlags, NAV_GROUND, tTriCount*sizeof(unsigned char));
            rcClearUnwalkableTriangles(&context, config.walkableSlopeAngle, tVerts, tVertCount, tTris, tTriCount, iv.triFlags);
            rcRasterizeTriangles(&context, tVerts, tVertCount, tTris, iv.triFlags, tTriCount, *iv.heightfield, config.walkableClimb);
            DELETE_ARRAY(iv.triFlags);

            // filter out unwalkable spans (order of calls matters, see rcFilterLowHangingWalkableObstacles)
            rcFilterLowHangingWalkableObstacles(&context, config.walkableClimb, *iv.heightfield);
            rcFilterLedgeSpans(&context, config.walkableHeight, config.walkableClimb, *iv.heightfield);
            rcFilterWalkableLowHeightSpans(&context, config.walkableHeight, *iv.heightfield);

            // do after filtering because same rules don't apply to swimming
            rcRasterizeTriangles(&context, lVerts, lVertCount, lTris, lTriFlags, lTriCount, *iv.heightfield, config.walkableClimb);

            // compact heightfield spans
            printf("%sCompacting heightfield...               \r", tileString);
            iv.compactHeightfield = rcAllocCompactHeightfield();
            if(!iv.compactHeightfield || !rcBuildCompactHeightfield(&context, config.walkableHeight, config.walkableClimb, *iv.heightfield, *iv.compactHeightfield))
            {
                printf("%sFailed compacting heightfield!            \n", tileString);
                break;
            }

            if(!m_debugOutput)
            {
                rcFreeHeightField(iv.heightfield);
                iv.heightfield = NULL;
            }

            // build polymesh intermediates
            printf("%sEroding walkable area width...          \r", tileString);
            if(!rcErodeWalkableArea(&context, config.walkableRadius, *iv.compactHeightfield))
            {
                printf("%sFailed eroding area!                    \n", tileString);
                break;
            }

            printf("%sSmoothing area boundaries...          \r", tileString);
            if(!rcMedianFilterWalkableArea(&context, *iv.compactHeightfield))
            {
                printf("%sFailed median filter!                    \n", tileString);
                break;
            }

            printf("%sBuilding distance field...              \r", tileString);
            if(!rcBuildDistanceField(&context, *iv.compactHeightfield))
            {
                printf("%sFailed building distance field!         \n", tileString);
                break;
            }

            // bottleneck is here
            printf("%sBuilding regions...                     \r", tileString);
            if(!rcBuildRegions(&context, *iv.compactHeightfield, config.borderSize, config.minRegionSize, config.mergeRegionSize))
            {
                printf("%sFailed building regions!                \n", tileString);
                break;
            }

            printf("%sBuilding contours...                    \r", tileString);
            iv.contours = rcAllocContourSet();
            if(!iv.contours || !rcBuildContours(&context, *iv.compactHeightfield, config.maxSimplificationError, config.maxEdgeLen, *iv.contours))
            {
                printf("%sFailed building contours!               \n", tileString);
                break;
            }

            // build polymesh
            printf("%sBuilding polymesh...                    \r", tileString);
            iv.polyMesh = rcAllocPolyMesh();
            if(!iv.polyMesh || !rcBuildPolyMesh(&context, *iv.contours, config.maxVertsPerPoly, *iv.polyMesh))
            {
                printf("%sFailed building polymesh!               \n", tileString);
                break;
            }

            printf("%sBuilding polymesh detail...             \r", tileString);
            iv.polyMeshDetail = rcAllocPolyMeshDetail();
            if(!iv.polyMeshDetail || !rcBuildPolyMeshDetail(&context, *iv.polyMesh, *iv.compactHeightfield, config.detailSampleDist, config.detailSampleMaxError, *iv.polyMeshDetail))
            {
                printf("%sFailed building polymesh detail!        \n", tileString);
                break;
            }

            if(!m_debugOutput)
            {
                rcFreeCompactHeightfield(iv.compactHeightfield);
                iv.compactHeightfield = NULL;
                rcFreeContourSet(iv.contours);
                iv.contours = NULL;
            }

            // this might be handled within Recast at some point
            printf("%sCleaning vertex padding...              \r", tileString);
            for(int i = 0; i < iv.polyMesh->nverts; ++i)
            {
                unsigned short* v = &iv.polyMesh->verts[i*3];
                v[0] -= (unsigned short)config.borderSize;
                v[2] -= (unsigned short)config.borderSize;
            }

            // polymesh vertex indices are stored with ushorts in detour, can't have more than 65535
            if(iv.polyMesh->nverts >= 0xffff)
            {
                printf("%sToo many vertices!                      \n", tileString);
                break;
            }

            printf("%sSetting polys as walkable...            \r", tileString);
            for(int i = 0; i < iv.polyMesh->npolys; ++i)
                if(iv.polyMesh->areas[i] & RC_WALKABLE_AREA)
                    iv.polyMesh->flags[i] = iv.polyMesh->areas[i];

            dtNavMeshCreateParams params;
            memset(&params, 0, sizeof(params));
            params.verts = iv.polyMesh->verts;
            params.vertCount = iv.polyMesh->nverts;
            params.polys = iv.polyMesh->polys;
            params.polyAreas = iv.polyMesh->areas;
            params.polyFlags = iv.polyMesh->flags;
            params.polyCount = iv.polyMesh->npolys;
            params.nvp = iv.polyMesh->nvp;
            params.detailMeshes = iv.polyMeshDetail->meshes;
            params.detailVerts = iv.polyMeshDetail->verts;
            params.detailVertsCount = iv.polyMeshDetail->nverts;
            params.detailTris = iv.polyMeshDetail->tris;
            params.detailTriCount = iv.polyMeshDetail->ntris;
            params.walkableHeight = agentHeight;
            params.walkableRadius = agentRadius;
            params.walkableClimb = agentMaxClimb;
            params.tileX = (((bmin[0] + bmax[0]) / 2) - navMesh->getParams()->orig[0]) / GRID_SIZE;
            params.tileY = (((bmin[2] + bmax[2]) / 2) - navMesh->getParams()->orig[2]) / GRID_SIZE;
            rcVcopy(params.bmin, bmin);
            rcVcopy(params.bmax, bmax);
            params.cs = config.cs;
            params.ch = config.ch;
            params.tileSize = config.tileSize;

            // will hold final navmesh
            unsigned char* navData = 0;
            int navDataSize = 0;

            // these values are checked within dtCreateNavMeshData - handle them here
            // so we have a clear error message
            if (params.nvp > DT_VERTS_PER_POLYGON)
            {
                printf("%sInvalid verts-per-polygon value!        \n", tileString);
                break;
            }
            if (params.vertCount >= 0xffff)
            {
                printf("%sToo many vertices!                      \n", tileString);
                break;
            }
            if (!params.vertCount || !params.verts)
            {
                // occurs mostly when adjacent tiles have models
                // loaded but those models don't span into this tile

                // message is an annoyance
                //printf("%sNo vertices to build tile!              \n", tileString);
                break;
            }
            if (!params.polyCount || !params.polys)
            {
                printf("%sNo polygons to build tile!              \n", tileString);
                break;
            }
            if (!params.detailMeshes || !params.detailVerts || !params.detailTris)
            {
                printf("%sNo detail mesh to build tile!           \n", tileString);
                break;
            }

            printf("%sBuilding navmesh tile...                \r", tileString);
            if(!dtCreateNavMeshData(&params, &navData, &navDataSize))
            {
                printf("%sFailed building navmesh tile!           \n", tileString);
                delete [] navData;
                break;
            }

            dtTileRef tileRef = 0;
            printf("%Adding tile to navmesh...                \r", tileString);
            // DT_TILE_FREE_DATA tells detour to unallocate memory when the tile
            // is removed via removeTile()
            if(!(tileRef = navMesh->addTile(navData, navDataSize, DT_TILE_FREE_DATA)))
            {
                printf("%Failed adding tile to navmesh!           \n", tileString);
                delete [] navData;
                break;
            }

            sprintf(fileName, "mmaps/%03u%02i%02i.mmtile", mapID, tileY, tileX);
            if(!(file = fopen(fileName, "wb")))
            {
                char message[1024];
                sprintf(message, "Failed to open %s for writing!\n", fileName);
                perror(message);
                navMesh->removeTile(tileRef, 0, 0);
                break;
            }

            printf("%sWriting to file...                      \r", tileString);
            // should write navDataSize first... for now, just use ftell to find length when reading
            fwrite(navData, sizeof(unsigned char), navDataSize, file);
            fclose(file);

            // now that tile is written to disk, we can unload it
            navMesh->removeTile(tileRef, 0, 0);

        }while(0);

        if(m_debugOutput)
        {
            generateObjFile(mapID, tileX, tileY, meshData);
            writeIV(mapID, tileX, tileY, iv);
        }

        clearIntermediateValues(iv);
    }

    void MapBuilder::getTileBounds(uint32 tileX, uint32 tileY, float* verts, int vertCount, float* bmin, float* bmax)
    {
        // this is for elevation
        if(verts && vertCount)
            rcCalcBounds(verts, vertCount, bmin, bmax);
        else
        {
            bmin[1] = FLT_MIN;
            bmax[1] = FLT_MAX;
        }

        // this is for width and depth
        bmax[0] = (32 - int(tileX)) * GRID_SIZE;
        bmax[2] = (32 - int(tileY)) * GRID_SIZE;
        bmin[0] = bmax[0] - GRID_SIZE;
        bmin[2] = bmax[2] - GRID_SIZE;
    }

    void MapBuilder::initIntermediateValues(IntermediateValues &iv)
    {
        iv.compactHeightfield = 0;
        iv.heightfield = 0;
        iv.triFlags = 0;
        iv.contours = 0;
        iv.polyMesh = 0;
        iv.polyMeshDetail = 0;
    }

    void MapBuilder::clearIntermediateValues(IntermediateValues &iv)
    {
        rcFreeCompactHeightfield(iv.compactHeightfield); iv.compactHeightfield = NULL;
        rcFreeHeightField(iv.heightfield); iv.heightfield = NULL;
        rcFreeContourSet(iv.contours); iv.contours = NULL;
        rcFreePolyMesh(iv.polyMesh); iv.polyMesh = NULL;
        rcFreePolyMeshDetail(iv.polyMeshDetail); iv.polyMeshDetail = NULL;
        DELETE_ARRAY(iv.triFlags); iv.triFlags = NULL;
    }

    void MapBuilder::generateObjFile(uint32 mapID, uint32 tileX, uint32 tileY, MeshData meshData)
    {
        generateRealObj(mapID, tileX, tileY, meshData);

        char tileString[25];
        sprintf(tileString, "[%02u,%02u]: ", tileX, tileY);
        printf("%sWriting debug output...                       \r", tileString);

        char objFileName[255];
        FILE* objFile;

        char b = '\0';

        sprintf(objFileName, "meshes/%03u.obj", mapID);
        if(!(objFile = fopen(objFileName, "wb")))
        {
            char message[1024];
            sprintf(message, "Failed to open %s for writing!\n", objFileName);
            perror(message);
            return;
        }

        fwrite(&b, sizeof(char), 1, objFile);
        fclose(objFile);

        sprintf(objFileName, "meshes/%03u%02u%02u.mesh", mapID, tileX, tileY);
        objFile = fopen(objFileName, "wb");
        if(!objFile)
        {
            char message[1024];
            sprintf(message, "Failed to open %s for writing!\n", objFileName);
            perror(message);
            return;
        }

        G3D::Array<float> allVerts;
        G3D::Array<int> allTris;

        allTris.append(meshData.liquidTris);
        allVerts.append(meshData.liquidVerts);
        copyIndices(allTris, meshData.solidTris, allVerts.size() / 3);
        allVerts.append(meshData.solidVerts);

        float* verts = allVerts.getCArray();
        int vertCount = allVerts.size() / 3;
        int* tris = allTris.getCArray();
        int triCount = allTris.size() / 3;

        fwrite(&vertCount, sizeof(int), 1, objFile);
        fwrite(verts, sizeof(float), vertCount*3, objFile);
        fflush(objFile);

        fwrite(&triCount, sizeof(int), 1, objFile);
        fwrite(tris, sizeof(int), triCount*3, objFile);
        fflush(objFile);

        fclose(objFile);
    }

    void MapBuilder::generateRealObj(uint32 mapID, uint32 tileX, uint32 tileY, MeshData meshData)
    {
        char objFileName[255];
        FILE* objFile;

        char b = '\0';

        sprintf(objFileName, "meshes/map%03u.obj", mapID);
        if(!(objFile = fopen(objFileName, "wb")))
        {
            char message[1024];
            sprintf(message, "Failed to open %s for writing!\n", objFileName);
            perror(message);
            return;
        }

        G3D::Array<float> allVerts;
        G3D::Array<int> allTris;

        allTris.append(meshData.liquidTris);
        allVerts.append(meshData.liquidVerts);
        copyIndices(allTris, meshData.solidTris, allVerts.size() / 3);
        allVerts.append(meshData.solidVerts);

        float* verts = allVerts.getCArray();
        int* tris = allTris.getCArray();

        for(int i = 0; i < allVerts.size() / 3; i++)
            fprintf(objFile, "v %f %f %f\n", verts[i*3], verts[i*3 + 1], verts[i*3 + 2]);

        for(int i = 0; i < allTris.size() / 3; i++)
            fprintf(objFile, "f %i %i %i\n", tris[i*3], tris[i*3 + 1], tris[i*3 + 2]);

        fclose(objFile);
    }

    void MapBuilder::writeIV(uint32 mapID, uint32 tileX, uint32 tileY, IntermediateValues iv)
    {
        char fileName[255];
        char tileString[25];
        FILE* file;

        sprintf(tileString, "[%02u,%02u]: ", tileX, tileY);

        printf("%sWriting debug output...                       \r", tileString);

        // heightfield
        sprintf(fileName, "meshes/%03u%02i%02i.hf", mapID, tileX, tileY);
        if(!(file = fopen(fileName, "wb")))
        {
            char message[1024];
            sprintf(message, "%sFailed to open %s for writing!\n",  tileString, fileName);
            perror(message);
        }
        else
            writeHeightfield(file, iv.heightfield);
        if(file) fclose(file);

        printf("%sWriting debug output...                       \r", tileString);

        // compact heightfield
        sprintf(fileName, "meshes/%03u%02i%02i.chf", mapID, tileX, tileY);
        if(!(file = fopen(fileName, "wb")))
        {
            char message[1024];
            sprintf(message, "%sFailed to open %s for writing!\n",  tileString, fileName);
            perror(message);
        }
        else
            writeCompactHeightfield(file, iv.compactHeightfield);
        if(file) fclose(file);

        printf("%sWriting debug output...                       \r", tileString);

        // contours
        sprintf(fileName, "meshes/%03u%02i%02i.cs", mapID, tileX, tileY);
        if(!(file = fopen(fileName, "wb")))
        {
            char message[1024];
            sprintf(message, "%sFailed to open %s for writing!\n",  tileString, fileName);
            perror(message);
        }
        else
            writeContours(file, iv.contours);
        if(file) fclose(file);

        printf("%sWriting debug output...                       \r", tileString);

        // poly mesh
        sprintf(fileName, "meshes/%03u%02i%02i.pmesh", mapID, tileX, tileY);
        if(!(file = fopen(fileName, "wb")))
        {
            char message[1024];
            sprintf(message, "%sFailed to open %s for writing!\n",  tileString, fileName);
            perror(message);
        }
        else
            writePolyMesh(file, iv.polyMesh);
        if(file) fclose(file);

        printf("%sWriting debug output...                       \r", tileString);

        // detail mesh
        sprintf(fileName, "meshes/%03u%02i%02i.dmesh", mapID, tileX, tileY);
        if(!(file = fopen(fileName, "wb")))
        {
            char message[1024];
            sprintf(message, "%sFailed to open %s for writing!\n",  tileString, fileName);
            perror(message);
        }
        else
            writeDetailMesh(file, iv.polyMeshDetail);
        if(file) fclose(file);
    }

    void MapBuilder::writeHeightfield(FILE* file, const rcHeightfield* mesh)
    {
        if(!file || !mesh)
            return;

        fwrite(&(mesh->cs), sizeof(float), 1, file);
        fwrite(&(mesh->ch), sizeof(float), 1, file);
        fwrite(&(mesh->width), sizeof(int), 1, file);
        fwrite(&(mesh->height), sizeof(int), 1, file);
        fwrite(mesh->bmin, sizeof(float), 3, file);
        fwrite(mesh->bmax, sizeof(float), 3, file);

        for(int y = 0; y < mesh->height; ++y)
            for(int x = 0; x < mesh->width; ++x)
            {
                rcSpan* span = mesh->spans[x+y*mesh->width];

                // first, count the number of spans
                int spanCount = 0;
                while(span)
                {
                    spanCount++;
                    span = span->next;
                }

                // write the span count
                fwrite(&spanCount, sizeof(int), 1, file);

                // write the spans
                span = mesh->spans[x+y*mesh->width];
                while(span)
                {
                    fwrite(span, sizeof(rcSpan), 1, file);
                    span = span->next;
                }
            }
    }

    void MapBuilder::writeCompactHeightfield(FILE* file, const rcCompactHeightfield* chf)
    {
        if(!file | !chf)
            return;

        fwrite(&(chf->width), sizeof(chf->width), 1, file);
        fwrite(&(chf->height), sizeof(chf->height), 1, file);
        fwrite(&(chf->spanCount), sizeof(chf->spanCount), 1, file);

        fwrite(&(chf->walkableHeight), sizeof(chf->walkableHeight), 1, file);
        fwrite(&(chf->walkableClimb), sizeof(chf->walkableClimb), 1, file);

        fwrite(&(chf->maxDistance), sizeof(chf->maxDistance), 1, file);
        fwrite(&(chf->maxRegions), sizeof(chf->maxRegions), 1, file);

        fwrite(chf->bmin, sizeof(chf->bmin), 1, file);
        fwrite(chf->bmax, sizeof(chf->bmax), 1, file);

        fwrite(&(chf->cs), sizeof(chf->cs), 1, file);
        fwrite(&(chf->ch), sizeof(chf->ch), 1, file);

        int tmp = 0;
        if (chf->cells) tmp |= 1;
        if (chf->spans) tmp |= 2;
        if (chf->dist) tmp |= 4;
        if (chf->areas) tmp |= 8;

        fwrite(&tmp, sizeof(tmp), 1, file);

        if (chf->cells)
            fwrite(chf->cells, sizeof(rcCompactCell), chf->width*chf->height, file);
        if (chf->spans)
            fwrite(chf->spans, sizeof(rcCompactSpan), chf->spanCount, file);
        if (chf->dist)
            fwrite(chf->dist, sizeof(unsigned short), chf->spanCount, file);
        if (chf->areas)
            fwrite(chf->areas, sizeof(unsigned char), chf->spanCount, file);
    }

    void MapBuilder::writeContours(FILE* file, const rcContourSet* cs)
    {
        if(!file || !cs)
            return;
        
        fwrite(&(cs->cs), sizeof(float), 1, file);
        fwrite(&(cs->ch), sizeof(float), 1, file);
        fwrite(cs->bmin, sizeof(float), 3, file);
        fwrite(cs->bmax, sizeof(float), 3, file);
        fwrite(&(cs->nconts), sizeof(int), 1, file);
        for(int i = 0; i < cs->nconts; ++i)
        {
            fwrite(&cs->conts[i].area, sizeof(unsigned char), 1, file);
            fwrite(&cs->conts[i].reg, sizeof(unsigned short), 1, file);
            fwrite(&cs->conts[i].nverts, sizeof(int), 1, file);
            fwrite(cs->conts[i].verts, sizeof(int), cs->conts[i].nverts*4, file);
            fwrite(&cs->conts[i].nrverts, sizeof(int), 1, file);
            fwrite(cs->conts[i].rverts, sizeof(int), cs->conts[i].nrverts*4, file);
        }
    }

    void MapBuilder::writePolyMesh(FILE* file, const rcPolyMesh* mesh)
    {
        if(!file || !mesh)
            return;

        fwrite(&(mesh->cs), sizeof(float), 1, file);
        fwrite(&(mesh->ch), sizeof(float), 1, file);
        fwrite(&(mesh->nvp), sizeof(int), 1, file);
        fwrite(mesh->bmin, sizeof(float), 3, file);
        fwrite(mesh->bmax, sizeof(float), 3, file);
        fwrite(&(mesh->nverts), sizeof(int), 1, file);
        fwrite(mesh->verts, sizeof(unsigned short), mesh->nverts*3, file);
        fwrite(&(mesh->npolys), sizeof(int), 1, file);
        fwrite(mesh->polys, sizeof(unsigned short), mesh->npolys*mesh->nvp*2, file);
        fwrite(mesh->flags, sizeof(unsigned short), mesh->npolys, file);
        fwrite(mesh->areas, sizeof(unsigned char), mesh->npolys, file);
        fwrite(mesh->regs, sizeof(unsigned short), mesh->npolys, file);
    }

    void MapBuilder::writeDetailMesh(FILE* file, const rcPolyMeshDetail* mesh)
    {
        if(!file || !mesh)
            return;

        fwrite(&(mesh->nverts), sizeof(int), 1, file);
        fwrite(mesh->verts, sizeof(float), mesh->nverts*3, file);
        fwrite(&(mesh->ntris), sizeof(int), 1, file);
        fwrite(mesh->tris, sizeof(char), mesh->ntris*4, file);
        fwrite(&(mesh->nmeshes), sizeof(int), 1, file);
        fwrite(mesh->meshes, sizeof(short), mesh->nmeshes*4, file);
    }

    bool MapBuilder::shouldSkipMap(uint32 mapID)
    {
        // debug
        //switch(mapID)
        //{
        //    case 309:
        //        return false;
        //    default:
        //        return true;
        //}

        if(m_skipContinents)
            switch(mapID)
            {
                case 0:
                case 1:
                case 530:
                case 571:
                    return true;
                default:
                    break;
            }

        if(m_skipJunkMaps)
            switch(mapID)
            {
                case 13:    // test.wdt
                case 25:    // ScottTest.wdt
                case 29:    // Test.wdt
                case 42:    // Colin.wdt
                case 169:   // EmeraldDream.wdt (unused, and very large)
                case 451:   // development.wdt
                case 573:   // ExteriorTest.wdt
                case 597:   // CraigTest.wdt
                case 605:   // development_nonweighted.wdt
                case 606:   // QA_DVD.wdt
                    return true;
                default:
                    if(isTransportMap(mapID))
                        return true;
                    break;
            }

        if(m_skipBattlegrounds)
            switch(mapID)
            {
                case 30:    // AV
                case 37:    // ?
                case 489:   // WSG
                case 529:   // AB
                case 566:   // EotS
                case 607:   // SotA
                case 628:   // IoC
                    return true;
                default:
                    break;
            }

        return false;
    }

    bool MapBuilder::isTransportMap(uint32 mapID)
    {
        switch(mapID)
        {
            // transport maps
            case 582:
            case 584:
            case 586:
            case 587:
            case 588:
            case 589:
            case 590:
            case 591:
            case 592:
            case 593:
            case 594:
            case 596:
            case 610:
            case 612:
            case 613:
            case 614:
            case 620:
            case 621:
            case 622:
            case 623:
            case 641:
            case 642:
            case 647:
            case 672:
            case 673:
            case 712:
            case 713:
            case 718:
                return true;
            default:
                return false;
        }
    }
}