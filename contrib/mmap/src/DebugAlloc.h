/*****************************************************
 * Provides macros for allocating memory
 *
 * the NEW and NEW_ARRAY macro ensure that nothrow
 * is passed to function 'operator new' and
 * 'operator new[]', respectively
 * enables you to handle bad allocations by checking
 * for NULL pointers instead of handling exceptions
 *
 * DEBUG_ALLOC:
 * When used on local-scope variables, this can assist
 * in finding memory leaks.
 * Global variables stay in memory for much longer,
 * so their allocation record will linger.  If you
 * use these macros for debugging, it is recommended
 * that you NOT use these macros for global variables
 *
 * the NEW and NEW_ARRAY macro keep a record of all
 * allocations
 * DELETE and DELETE_ARRAY remove the record of the
 * allocation
 *
 * at any point, the PRINT_UNALLOC macro will print
 * a list of all references that have not been
 * deallocated
 *
 *****************************************************/

#ifndef _DEBUG_ALLOC_H
#define _DEBUG_ALLOC_H

#include <new>
#include <map>
#include <string>

using namespace std;

#ifdef DEBUG_ALLOC

extern map<void*,string>* debug_alloc;
extern map<void*,string>* debug_aalloc;

inline void* debug_new(void* ptr, char* file, int line)
{
    if(!debug_alloc)
        init_debug_alloc();

    char lineText[30];
    sprintf(lineText, ":%d", line);
    string alloc(file);
    alloc += lineText;
    ::debug_alloc->insert(std::pair<void*,string>(ptr, alloc));
    return ptr;
}

inline void* debug_newa(void* ptr, char* file, int line)
{
    if(!debug_alloc)
        init_debug_alloc();

    char lineText[30];
    sprintf(lineText, ":%d", line);
    string alloc(file);
    alloc += lineText;
    ::debug_aalloc->insert(std::pair<void*,string>(ptr, alloc));
    return ptr;
}

inline void debug_delete(void* ptr, char* file, int line)
{
    if(::debug_alloc)
        ::debug_alloc->erase(ptr);
}

inline void debug_deletea(void* ptr, char* file, int line)
{
    if(::debug_aalloc)
        ::debug_aalloc->erase(ptr);
}

inline void init_debug_alloc()
{
    if(!::debug_alloc)
        ::debug_alloc = new map<void*,string>;

    if(!::debug_aalloc)
        ::debug_aalloc = new map<void*,string>;
}

inline void print_allocated()
{
    if(::debug_alloc && ::debug_alloc->size())
    {
        printf("Allocated values:\n");
        for(map<void*,string>::iterator it = ::debug_alloc->begin(); it != ::debug_alloc->end(); ++it)
            printf("%s\n", (*it).second.c_str());
    }

    if(::debug_aalloc && ::debug_aalloc->size())
    {
        printf("Allocated arrays:\n");
        for(map<void*,string>::iterator it = ::debug_aalloc->begin(); it != ::debug_aalloc->end(); ++it)
            printf("%s\n", (*it).second.c_str());
    }
}

// wait for keystroke if there are outstanding allocated objects
inline void wait_allocated()
{
    if((::debug_alloc && ::debug_alloc->size()) || (::debug_aalloc && ::debug_aalloc->size()))
        getchar();
}

#define NEW(type, ...) \
    (type*) debug_new(new (std::nothrow) type __VA_ARGS__, __FILE__, __LINE__)

#define NEW_ARRAY(type, count) \
    (type*) debug_newa(new (std::nothrow) type[count], __FILE__, __LINE__)

#define DELETE(ptr)                             \
    do                                          \
    {                                           \
        delete ptr;                             \
        debug_delete(ptr, __FILE__, __LINE__);  \
        ptr = 0;                                \
    } while(0)

#define DELETE_ARRAY(ptr)                       \
    do                                          \
    {                                           \
        delete [] ptr;                          \
        debug_deletea(ptr, __FILE__, __LINE__); \
        ptr = 0;                                \
    } while(0)

#define PRINT_ALLOC print_allocated
#define WAIT_ALLOC wait_allocated

#else // not DEBUG_ALLOC

#define NEW(type, ...)  \
    new (nothrow) type __VA_ARGS__

#define NEW_ARRAY(type, count) \
    new (nothrow) type[count]

#define DELETE(ptr)     \
    do                  \
    {                   \
        delete ptr;     \
        ptr = 0;        \
    } while(0)

#define DELETE_ARRAY(ptr)   \
    do                      \
    {                       \
        delete [] ptr;      \
        ptr = 0;            \
    } while(0)


#define PRINT_ALLOC
#define WAIT_ALLOC

#endif  // DEBUG_ALLOC

#endif  // _DEBUG_ALLOC_H
