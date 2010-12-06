
#include "Debug.h"

#include <vector>
#include "DetourNavMesh.h"
#include "Recast.h"
#include "MMapCommon.h"

void duReadNavMesh(int mapID, dtNavMesh* &navMesh)
{
    char fileName[25];
    FILE* file;

    sprintf(fileName, "mmaps\\%03i.mmap", mapID);
    file = fopen(fileName, "rb");

    if(!file)
        return;

    dtNavMeshParams params;
    fread(&params, sizeof(dtNavMeshParams), 1, file);
    fclose(file);

    dtFreeNavMesh(navMesh);
    navMesh = dtAllocNavMesh();
    navMesh->init(&params);

    sprintf(fileName, "%03i*.mmtile", mapID);

    vector<string> fileNames;
    MMAP::getDirContents(fileNames, "mmaps", fileName);

    for(int i = 0; i < fileNames.size(); ++i)
    {
        file = fopen(("mmaps\\" + fileNames[i]).c_str(), "rb");
        if(!file)
            continue;

        fseek(file, 0, SEEK_END);
        int length = ftell(file);
        fseek(file, 0, SEEK_SET);

        unsigned char* data = new unsigned char[length];
        fread(data, length, 1, file);
        fclose(file);

        navMesh->addTile(data, length, DT_TILE_FREE_DATA);
    }

    if(!fileNames.size())
    {
        delete navMesh;
        navMesh = 0;
    }
}

int duReadHeightfield(int mapID, rcHeightfield** &hf)
{
    char fileName[25];
    FILE* file;

    vector<string> files;
    sprintf(fileName, "%03i*.hf", mapID);
    MMAP::getDirContents(files, "meshes", fileName);

    hf = new rcHeightfield*[files.size()];

    for(int i = 0; i < files.size(); ++i)
    {
        file = fopen(("meshes/" + files[i]).c_str(), "rb");
        if(!file)
            continue;

        hf[i] = rcAllocHeightfield();

        fread(&(hf[i]->cs), sizeof(float), 1, file);
        fread(&(hf[i]->ch), sizeof(float), 1, file);
        fread(&(hf[i]->width), sizeof(int), 1, file);
        fread(&(hf[i]->height), sizeof(int), 1, file);
        fread(hf[i]->bmin, sizeof(float), 3, file);
        fread(hf[i]->bmax, sizeof(float), 3, file);

        hf[i]->spans = new rcSpan*[hf[i]->width*hf[i]->height];
        memset(hf[i]->spans, 0, sizeof(rcSpan*)*hf[i]->width*hf[i]->height);

        for(int y = 0; y < hf[i]->height; ++y)
            for(int x = 0; x < hf[i]->width; ++x)
            {
                int spanCount;
                fread(&spanCount, sizeof(int), 1, file);

                if(spanCount)
                    hf[i]->spans[x + y*hf[i]->width] = new rcSpan;
                else
                    hf[i]->spans[x + y*hf[i]->width] = NULL;

                rcSpan* span = hf[i]->spans[x + y*hf[i]->width];

                while(spanCount--)
                {
                    fread(span, sizeof(rcSpan), 1, file);

                    if(spanCount)
                    {
                        span->next = new rcSpan;
                        span = span->next;
                    }
                    else
                        span->next = NULL;
                }
            }

        fclose(file);
    }

    return files.size();
}

int duReadCompactHeightfield(int mapID, rcCompactHeightfield** &chf)
{
    char fileName[25];
    FILE* file;

    vector<string> files;
    sprintf(fileName, "%03i*.chf", mapID);
    MMAP::getDirContents(files, "meshes", fileName);

    chf = new rcCompactHeightfield*[files.size()];
    for(int i = 0; i < files.size(); ++i)
    {
        file = fopen(("meshes/" + files[i]).c_str(), "rb");
        if(!file)
            continue;

        chf[i] = rcAllocCompactHeightfield();

        fread(&chf[i]->width, sizeof(chf[i]->width), 1, file);
        fread(&chf[i]->height, sizeof(chf[i]->height), 1, file);
        fread(&chf[i]->spanCount, sizeof(chf[i]->spanCount), 1, file);

        fread(&chf[i]->walkableHeight, sizeof(chf[i]->walkableHeight), 1, file);
        fread(&chf[i]->walkableClimb, sizeof(chf[i]->walkableClimb), 1, file);

        fread(&chf[i]->maxDistance, sizeof(chf[i]->maxDistance), 1, file);
        fread(&chf[i]->maxRegions, sizeof(chf[i]->maxRegions), 1, file);

        fread(chf[i]->bmin, sizeof(chf[i]->bmin), 1, file);
        fread(chf[i]->bmax, sizeof(chf[i]->bmax), 1, file);
    
        fread(&chf[i]->cs, sizeof(chf[i]->cs), 1, file);
        fread(&chf[i]->ch, sizeof(chf[i]->ch), 1, file);

        int tmp = 0;
        fread(&tmp, sizeof(tmp), 1, file);

        if (tmp & 1)
        {
            chf[i]->cells = new rcCompactCell[chf[i]->width*chf[i]->height];
            fread(chf[i]->cells, sizeof(rcCompactCell), chf[i]->width*chf[i]->height, file);
        }
        if (tmp & 2)
        {
            chf[i]->spans = new rcCompactSpan[chf[i]->spanCount];
            fread(chf[i]->spans, sizeof(rcCompactSpan), chf[i]->spanCount, file);
        }
        if (tmp & 4)
        {
            chf[i]->dist = new unsigned short[chf[i]->spanCount];
            fread(chf[i]->dist, sizeof(unsigned short), chf[i]->spanCount, file);
        }
        if (tmp & 8)
        {
            chf[i]->areas = new unsigned char[chf[i]->spanCount];
            fread(chf[i]->areas, sizeof(unsigned char), chf[i]->spanCount, file);
        }

        fclose(file);
    }

    return files.size();
}

int duReadContourSet(int mapID, rcContourSet** &cs)
{
    char fileName[25];
    FILE* file;

    vector<string> files;
    sprintf(fileName, "%03i*.cs", mapID);
    MMAP::getDirContents(files, "meshes", fileName);

    cs = new rcContourSet*[files.size()];

    for(int i = 0; i < files.size(); ++i)
    {
        file = fopen(("meshes\\" + files[i]).c_str(), "rb");
        if(!file)
            continue;

        cs[i] = rcAllocContourSet();

        fread(&(cs[i]->cs), sizeof(float), 1, file);
        fread(&(cs[i]->ch), sizeof(float), 1, file);
        fread(cs[i]->bmin, sizeof(float), 3, file);
        fread(cs[i]->bmax, sizeof(float), 3, file);
        fread(&(cs[i]->nconts), sizeof(int), 1, file);

        if(cs[i]->nconts)
            cs[i]->conts = new rcContour[cs[i]->nconts];

        for(int j = 0; j < cs[i]->nconts; ++j)
        {
            cs[i]->conts[j].verts = 0;
            cs[i]->conts[j].rverts = 0;

            fread(&(cs[i]->conts[j].area), sizeof(unsigned char), 1, file);
            fread(&(cs[i]->conts[j].reg), sizeof(unsigned short), 1, file);

            fread(&(cs[i]->conts[j].nverts), sizeof(int), 1, file);
            cs[i]->conts[j].verts = new int[cs[i]->conts[j].nverts*4];
            fread(cs[i]->conts[j].verts, sizeof(int), cs[i]->conts[j].nverts*4, file);

            fread(&(cs[i]->conts[j].nrverts), sizeof(int), 1, file);
            cs[i]->conts[j].rverts = new int[cs[i]->conts[j].nrverts*4];
            fread(cs[i]->conts[j].rverts, sizeof(int), cs[i]->conts[j].nrverts*4, file);
        }

        fclose(file);
    }

    return files.size();
}

int duReadPolyMesh(int mapID, rcPolyMesh** &mesh)
{
    char fileName[25];
    FILE* file;

    vector<string> files;
    sprintf(fileName, "%03i*.pmesh", mapID);
    MMAP::getDirContents(files, "meshes", fileName);

    mesh = new rcPolyMesh*[files.size()];

    for(int i = 0; i < files.size(); ++i)
    {
        file = fopen(("meshes\\" + files[i]).c_str(), "rb");
        if(!file)
            continue;

        mesh[i] = rcAllocPolyMesh();

        fread(&(mesh[i]->cs), sizeof(float), 1, file);
        fread(&(mesh[i]->ch), sizeof(float), 1, file);
        fread(&(mesh[i]->nvp), sizeof(int), 1, file);
        fread(mesh[i]->bmin, sizeof(float), 3, file);
        fread(mesh[i]->bmax, sizeof(float), 3, file);
        fread(&(mesh[i]->nverts), sizeof(int), 1, file);
        mesh[i]->verts = new unsigned short[mesh[i]->nverts*3];
        fread(mesh[i]->verts, sizeof(unsigned short), mesh[i]->nverts*3, file);
        fread(&(mesh[i]->npolys), sizeof(int), 1, file);
        mesh[i]->polys = new unsigned short[mesh[i]->npolys*mesh[i]->nvp*2];
        mesh[i]->flags = new unsigned short[mesh[i]->npolys];
        mesh[i]->areas = new unsigned char[mesh[i]->npolys];
        mesh[i]->regs = new unsigned short[mesh[i]->npolys];
        fread(mesh[i]->polys, sizeof(unsigned short), mesh[i]->npolys*mesh[i]->nvp*2, file);
        fread(mesh[i]->flags, sizeof(unsigned short), mesh[i]->npolys, file);
        fread(mesh[i]->areas, sizeof(unsigned char), mesh[i]->npolys, file);
        fread(mesh[i]->regs, sizeof(unsigned short), mesh[i]->npolys, file);

        fclose(file);
    }

    return files.size();
}

int duReadDetailMesh(int mapID, rcPolyMeshDetail** &mesh)
{
    char fileName[25];
    FILE* file;

    vector<string> files;
    sprintf(fileName, "%03i*.dmesh", mapID);
    MMAP::getDirContents(files, "meshes", fileName);

    mesh = new rcPolyMeshDetail*[files.size()];

    for(int i = 0; i < files.size(); ++i)
    {
        file = fopen(("meshes\\" + files[i]).c_str(), "rb");
        if(!file)
            continue;

        mesh[i] = rcAllocPolyMeshDetail();

        fread(&(mesh[i]->nverts), sizeof(int), 1, file);
        mesh[i]->verts = new float[mesh[i]->nverts*3];
        fread(mesh[i]->verts, sizeof(float), mesh[i]->nverts*3, file);

        fread(&(mesh[i]->ntris), sizeof(int), 1, file);
        mesh[i]->tris = new unsigned char[mesh[i]->ntris*4];
        fread(mesh[i]->tris, sizeof(char), mesh[i]->ntris*4, file);

        fread(&(mesh[i]->nmeshes), sizeof(int), 1, file);
        mesh[i]->meshes = new unsigned short[mesh[i]->nmeshes*4];
        fread(mesh[i]->meshes, sizeof(short), mesh[i]->nmeshes*4, file);

        fclose(file);
    }

    return files.size();
}

myMeshLoaderObj::myMeshLoaderObj() :
	m_verts(0),
	m_tris(0),
	m_normals(0),
	m_vertCount(0),
	m_triCount(0)
{
}

myMeshLoaderObj::~myMeshLoaderObj()
{
	delete [] m_verts;
	delete [] m_normals;
	delete [] m_tris;
}

bool myMeshLoaderObj::load(const char* filename)
{
    char fileName[25];
    FILE* file;

    string fname(filename);

    vector<string> files;
    sprintf(fileName, "%s*.mesh", fname.substr(fname.find('/')+1,3).c_str());
    MMAP::getDirContents(files, "Meshes", fileName);

    if(!files.size())
        return false;

    float* verts = 0;
    int vertCount = 0;
    int* tris = 0;
    int triCount = 0;
    int newVertCount;
    int newTriCount;

    for(int i = 0; i < files.size(); ++i)
    {
        file = fopen(("meshes/" + files[i]).c_str(), "rb");
        if(!file)
            continue;

        if(verts)
        {
            fread(&newVertCount, sizeof(int), 1, file);

            float* newVerts = new float[vertCount*3 + newVertCount*3];
            memcpy(newVerts, verts, sizeof(float)*vertCount*3);

            fread(&newVerts[vertCount*3], sizeof(float), newVertCount*3, file);

            delete[] verts;
            verts = newVerts;
        }
        else
        {
            fread(&vertCount, sizeof(int), 1, file);
            verts = new float[vertCount*3];
            fread(verts, sizeof(float), vertCount*3, file);
        }

        if(tris)
        {
            fread(&newTriCount, sizeof(int), 1, file);

            int* newTris = new int[triCount*3 + newTriCount*3];
            memcpy(newTris, tris, sizeof(int)*triCount*3);

            fread(&newTris[triCount*3], sizeof(int), newTriCount*3, file);

            delete[] tris;
            tris = newTris;

            for(int j = triCount*3; j < triCount*3 + newTriCount*3; ++j)
                tris[j] += vertCount;

            vertCount += newVertCount;
            triCount += newTriCount;
        }
        else
        {
            fread(&triCount, sizeof(int), 1, file);
            tris = new int[triCount*3];
            fread(tris, sizeof(int), triCount*3, file);
        }

        fclose(file);
    }

    m_verts = verts;
    m_vertCount = vertCount;
    m_tris = tris;
    m_triCount = triCount;

	m_normals = new float[m_triCount*3];
	for (int i = 0; i < m_triCount*3; i += 3)
	{
		const float* v0 = &m_verts[m_tris[i]*3];
		const float* v1 = &m_verts[m_tris[i+1]*3];
		const float* v2 = &m_verts[m_tris[i+2]*3];
		float e0[3], e1[3];
		for (int j = 0; j < 3; ++j)
		{
			e0[j] = v1[j] - v0[j];
			e1[j] = v2[j] - v0[j];
		}
		float* n = &m_normals[i];
		n[0] = e0[1]*e1[2] - e0[2]*e1[1];
		n[1] = e0[2]*e1[0] - e0[0]*e1[2];
		n[2] = e0[0]*e1[1] - e0[1]*e1[0];
		float d = sqrtf(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
		if (d > 0)
		{
			d = 1.0f/d;
			n[0] *= d;
			n[1] *= d;
			n[2] *= d;
		}
	}
	
	strncpy(m_filename, filename, sizeof(m_filename));
	m_filename[sizeof(m_filename)-1] = '\0';
	
	return true;
}
