#include <map>
#include <set>

#include "MMapCommon.h"
#include "VMapManager2.h"
#include "MapBuilder.h"
#include "TileBuilder.h"
#include "MapTree.h"

using namespace std;
using namespace MMAP;

bool checkDirectories(bool debugOutput)
{
    vector<string> dirFiles;

    if(getDirContents(dirFiles, "maps") == LISTFILE_DIRECTORY_NOT_FOUND || !dirFiles.size())
    {
        printf("'maps' directory is empty or does not exist\n");
        return false;
    }

    dirFiles.clear();
    if(getDirContents(dirFiles, "vmaps", "*.vmtree") == LISTFILE_DIRECTORY_NOT_FOUND || !dirFiles.size())
    {
        printf("'vmaps' directory is empty or does not exist\n");
        return false;
    }

    dirFiles.clear();
    if(getDirContents(dirFiles, "mmaps") == LISTFILE_DIRECTORY_NOT_FOUND)
    {
        printf("'mmaps' directory does not exist\n");
        return false;
    }

    dirFiles.clear();
    if(debugOutput)
        if(getDirContents(dirFiles, "Meshes") == LISTFILE_DIRECTORY_NOT_FOUND)
        {
            printf("'Meshes' directory does not exist (no place to put debugOutput files)\n");
            return false;
        }

    return true;
}

void handleArgs(int argc, char** argv,
               int &mapnum,
               int &tileX,
               int &tileY,
               float &maxAngle,
               bool &skipLiquid,
               bool &skipContinents,
               bool &skipJunkMaps,
               bool &skipBattlegrounds,
               bool &hiResHeightmaps,
               bool &debugOutput,
               bool &badParam)
{
    char zero[2] = "0";
    int i;
    char* param;
    for(i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], "--maxAngle") == 0)
        {
            param = argv[++i];
            float maxangle = atof(param);
            if(maxangle <= 90.f && maxangle >= 45.f)
                maxAngle = maxangle;
            else
                printf("invalid option for '--maxAngle', using default\n");
        }
        else if(strcmp(argv[i], "--tile") == 0)
        {
            param = argv[++i];
            char* stileX = strtok(param, ",");
            char* stileY = strtok(NULL, ",");
            int tilex = atoi(stileX);
            int tiley = atoi(stileY);

            if((tilex > 0 && tilex < 64) || (tilex == 0 && strcmp(stileX, zero) == 0))
                tileX = tilex;
            if((tiley > 0 && tiley < 64) || (tiley == 0 && strcmp(stileY, zero) == 0))
                tileY = tiley;

            if(tileX < 0 || tileY < 0)
            {
                printf("invalid tile coords.\n");
                badParam = true;
            }
        }
        else if(strcmp(argv[i], "--skipLiquid") == 0)
        {
            param = argv[++i];
            if(strcmp(param, "true") == 0)
                skipLiquid = true;
            else if(strcmp(param, "false") == 0)
                skipLiquid = false;
            else
                printf("invalid option for '--skipLiquid', using default\n");
        }
        else if(strcmp(argv[i], "--skipContinents") == 0)
        {
            param = argv[++i];
            if(strcmp(param, "true") == 0)
                skipContinents = true;
            else if(strcmp(param, "false") == 0)
                skipContinents = false;
            else
                printf("invalid option for '--skipContinents', using default\n");
        }
        else if(strcmp(argv[i], "--skipJunkMaps") == 0)
        {
            param = argv[++i];
            if(strcmp(param, "true") == 0)
                skipJunkMaps = true;
            else if(strcmp(param, "false") == 0)
                skipJunkMaps = false;
            else
                printf("invalid option for '--skipJunkMaps', using default\n");
        }
        else if(strcmp(argv[i], "--skipBattlegrounds") == 0)
        {
            param = argv[++i];
            if(strcmp(param, "true") == 0)
                skipBattlegrounds = true;
            else if(strcmp(param, "false") == 0)
                skipBattlegrounds = false;
            else
                printf("invalid option for '--skipBattlegrounds', using default\n");
        }
        else if(strcmp(argv[i], "--hiResHeightmaps") == 0)
        {
            param = argv[++i];
            if(strcmp(param, "true") == 0)
                hiResHeightmaps = true;
            else if(strcmp(param, "false") == 0)
                hiResHeightmaps = false;
            else
                printf("invalid option for '--hiResHeightmaps', using default\n");
        }
        else if(strcmp(argv[i], "--debugOutput") == 0)
        {
            param = argv[++i];
            if(strcmp(param, "true") == 0)
                debugOutput = true;
            else if(strcmp(param, "false") == 0)
                debugOutput = false;
            else
                printf("invalid option for '--debugOutput', using default true\n");
        }
        else
        {
            int map = atoi(argv[i]);
            if(map > 0 || (map == 0 && (strcmp(argv[i], zero) == 0)))
                mapnum = map;
            else
            {
                printf("bad parameter\n");
                badParam = true;
            }
        }
    }
}

int finish(char* message, int returnValue)
{
    printf("%s", message);
    getchar();
    return returnValue;
}

int main(int argc, char** argv)
{
    int mapnum = -1;
    float maxAngle = 60.f;
    int tileX = -1, tileY = -1;
    bool skipLiquid = true,
         skipContinents = true,
         skipJunkMaps = true,
         skipBattlegrounds = true,
         hiResHeightmaps = false,
         debugOutput = false,
         badParam = false;

    handleArgs(argc,
              argv,
              mapnum,
              tileX,
              tileY,
              maxAngle,
              skipLiquid,
              skipContinents,
              skipJunkMaps,
              skipBattlegrounds,
              hiResHeightmaps,
              debugOutput,
              badParam);

    if(badParam)
        return finish("You have specified invalid parameters", -1);

    if(mapnum == -1 && debugOutput)
    {
        printf("You have specifed debug output, but didn't specify a map to generate.\n");
        printf("This will generate debug output for ALL maps.\n");
        printf("Are you sure you want to continue? (y/n) ");
        if(getchar() != 'y')
            return 0;
    }

    if(!checkDirectories(debugOutput))
        return finish("Press any key to close...", -1);

    MapBuilder builder(maxAngle,
                       skipLiquid,
                       skipContinents,
                       skipJunkMaps,
                       skipBattlegrounds,
                       hiResHeightmaps,
                       debugOutput);

    if(tileX > -1 && tileY > -1 && mapnum >= 0)
        builder.buildTile(mapnum, tileX, tileY);
    else if(mapnum >= 0)
        builder.build(uint32(mapnum));
    else
        builder.buildAll();

    return finish("Movemap build is complete!", 1);
}
