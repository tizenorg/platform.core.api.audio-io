SET(fw_test "${fw_name}-test")

INCLUDE(FindPkgConfig)
FOREACH(flag ${${fw_test}_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -Wall -Werror -pie")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXXFLAGS} -fPIC -Wall -Werror -std=c++0x -pie")

aux_source_directory(. sources)
FOREACH(src ${sources})
    GET_FILENAME_COMPONENT(src_name ${src} NAME_WE)
    MESSAGE("${src_name}")
    ADD_EXECUTABLE(${src_name} ${src})
    TARGET_LINK_LIBRARIES(${src_name} ${fw_name} ${${fw_test}_LDFLAGS} -lm)
ENDFOREACH()
