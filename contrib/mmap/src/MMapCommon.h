#ifndef _MMAP_COMMON_H
#define _MMAP_COMMON_H

// stop warning spam from ACE includes
#pragma warning(disable : 4996)

#include <string>
#include <vector>

#include "platform/Define.h"
#include "DebugAlloc.h"

using namespace std;

namespace MMAP
{
#ifndef WIN32
    inline bool matchWildcardFilter(const char* filter, const char* str)
    {
        if(!filter || !str)
            return false;

        // end on null character
        while(*filter && *str)
        {
            if(*filter == '*')
            {
                if(*++filter == '\0')   // wildcard at end of filter means all remaing chars match
                    return true;

                while(true)
                {
                    if(*filter == *str)
                        break;
                    if(*str == '\0')
                        return false;   // reached end of string without matching next filter character
                    str++;
                }
            }
            else if(*filter != *str)
                return false;           // mismatch

            filter++;
            str++;
        }

        return ((*filter == '\0' || (*filter == '*' && *++filter == '\0')) && *str == '\0');
    }
#endif

    enum ListFilesResult : int
    {
        LISTFILE_DIRECTORY_NOT_FOUND = -1,
        LISTFILE_OK = 1
    };

    inline ListFilesResult getDirContents(vector<string> &fileList, string dirpath = ".", string filter = "*", bool includeSubDirs = false)
    {
    #ifdef WIN32
        HANDLE hFind;
        WIN32_FIND_DATA findFileInfo;
        string directory;

        directory = dirpath + "/" + filter;

        hFind = FindFirstFile(directory.c_str(), &findFileInfo);

        if(hFind == INVALID_HANDLE_VALUE)
            return LISTFILE_DIRECTORY_NOT_FOUND;

        do
        {
            if(includeSubDirs || (findFileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                fileList.push_back(string(findFileInfo.cFileName));
        }
        while (FindNextFile(hFind, &findFileInfo));

        FindClose(hFind);

    #else
        const char *p = dirpath.c_str();
        DIR * dirp;
        struct dirent * dp;
        dirp = opendir(p);

        while (dirp)
        {
            errno = 0;
            if ((dp = readdir(dirp)) != NULL)
            {
                if(matchWildcardFilter(filter.c_str(), dp->d_name))
                    fileList.push_back(string(dp->d_name));
            }
            else
                break;
        }

        if(dirp)
            closedir(dirp);
        else
            return LISTFILE_DIRECTORY_NOT_FOUND;
    #endif

        return LISTFILE_OK;
    }
}

#endif
