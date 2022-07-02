cmake_minimum_required(VERSION 3.7)

project(common LANGUAGES C)

file(GLOB COMMON_SOURCES CONFIGURE_DEPENDS
    "*.c"
    "codec/*.c"
    "dsp/*.c"
    "event/*.c"
    "package/*.c"
    "pipeline/*.c")
file(GLOB COMMON_HEADERS CONFIGURE_DEPENDS
    "*.h"
    "codec/*.h"
    "dsp/*.h"
    "event/*.h"
    "package/*.h"
    "pipeline/*.h")

add_library(common STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})
