
########### next target ###############

SET(neo-core_SRCS
CliRunnable.cpp 
CliRunnable.h 
Main.cpp 
Master.cpp 
Master.h 
RASocket.cpp 
RASocket.h 
WorldRunnable.cpp 
WorldRunnable.h
)

SET(neo-core_LINK_FLAGS "")

add_executable(neo-core ${neo-core_SRCS})
add_definitions(
-D_NEO_CORE_CONFIG='"${CONF_DIR}/neocore.conf"'
)
IF (DO_MYSQL)
    SET(neo-core_LINK_FLAGS "-pthread ${neo-core_LINK_FLAGS}")
ENDIF(DO_MYSQL)


IF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    SET(neo-core_LINK_FLAGS "-framework Carbon ${neo-core_LINK_FLAGS}")
    SET(SCRIPT_LIB "")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")

SET_TARGET_PROPERTIES(neo-core PROPERTIES LINK_FLAGS "${neo-core_LINK_FLAGS}")

target_link_libraries(
neo-core
game
shared
zlib
neoframework
neosockets
neodatabase
neoauth
neoconfig
vmaps
g3dlite
pathfind
recast
detour
${READLINE_LIBRARY}
gomp
${SCRIPT_LIB}
${MYSQL_LIBRARIES}
${SSLLIB}
${ACE_LIBRARY}
${ZLIB}
${OSX_LIBS}
)

IF (CENTOS)
    target_link_libraries(
        neo-core
        termcap
    )
ENDIF (CENTOS)

install(TARGETS neo-core DESTINATION bin)


########### install files ###############

install(FILES  neocore.conf.dist DESTINATION etc)

