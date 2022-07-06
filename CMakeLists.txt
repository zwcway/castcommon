cmake_minimum_required(VERSION 3.7)

project(common LANGUAGES C)

SET(COMMON_SOURCES
    audio.c
    block_queue.c
    connection.c
    crc.c
    log.c
    ip.c
    speaker_struct.c
    synctime.c
    utils.c

    "codec/wave.c"

    "dsp/resample.c"
    "dsp/mixer.c"

    event/epoll.c
    event/event.c
    event/poll.c
    event/receive.c
    event/select.c
    event/send.c
    event/udp.c

    package/package.c

    pipeline/element.c
    pipeline/pipeline.c
    )
file(GLOB COMMON_HEADERS CONFIGURE_DEPENDS
    "*.h"
    "codec/*.h"
    "dsp/*.h"
    "event/*.h"
    "package/*.h"
    "pipeline/*.h")


add_library(common STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})

if (WIN32)
  target_link_libraries(common ws2_32.lib Iphlpapi.lib)
endif ()
