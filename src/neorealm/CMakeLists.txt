########### next target ###############

SET(neo-realm_SRCS
AuthCodes.h 
AuthSocket.cpp 
AuthSocket.h 
Main.cpp 
RealmList.cpp 
RealmList.h
)

SET(neo-realm_LINK_FLAGS "")

add_executable(neo-realm ${neo-realm_SRCS})
add_definitions(
-D_NEO_REALM_CONFIG='"${CONF_DIR}/neorealm.conf"'
)
IF (DO_MYSQL)
   SET(neo-realm_LINK_FLAGS "-pthread ${neo-realm_LINK_FLAGS}")
ENDIF(DO_MYSQL)

IF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
   SET(neo-realm_LINK_FLAGS "-framework Carbon ${neo-realm_LINK_FLAGS}")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")

SET_TARGET_PROPERTIES(neo-realm PROPERTIES LINK_FLAGS "${neo-realm_LINK_FLAGS}")

target_link_libraries(
neo-realm
shared
neoframework
neosockets
neodatabase
neoauth
neoconfig
zlib
gomp
${SSLLIB}
${MYSQL_LIBRARIES}
${OSX_LIBS}
)

install(TARGETS neo-realm DESTINATION bin)


########### install files ###############

install(FILES  neorealm.conf.dist DESTINATION etc)

