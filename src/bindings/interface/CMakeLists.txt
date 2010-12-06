
########### next target ###############

SET(neointerface_LIB_SRCS
   ScriptMgr.cpp
   ScriptMgr.h
   config.h
   system.cpp
   Scripts/sc_default.cpp
   Scripts/sc_defines.cpp
   Scripts/sc_defines.h
)

add_library(neointerface SHARED ${neointerface_LIB_SRCS})

target_link_libraries(neointerface)

set_target_properties(neointerface PROPERTIES VERSION 4.2.0 SOVERSION 4)
install(TARGETS neointerface DESTINATION lib)
