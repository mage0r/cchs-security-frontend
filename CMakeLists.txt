cmake_minimum_required(VERSION 2.6)
project(door-frontend)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "cmake/Modules")
# find libnfc
find_package(PkgConfig)

pkg_check_modules(LIBNFC REQUIRED libnfc)
pkg_check_modules(LIBFREEFARE REQUIRED libfreefare)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

pkg_check_modules(DIRECT direct)
pkg_check_modules(DIRECTFB directfb)

find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(BCM2835)
set(ACTIONS_INCLUDE "")
set(ACTIONS_FILE "actions.c")
if (BCM2835_FOUND) 
	add_definitions( -DRASPI=2 )
	set(ACTIONS_FILE "actions-raspi.c")
	set(ACTIONS_INCLUDE ${BCM2835_INCLUDE_DIR})
endif (BCM2835_FOUND)

ADD_EXECUTABLE(door-system crypto.c door-system.c backend-comms.c ${ACTIONS_FILE} door-monitor.c ipc.c)
# ADD_EXECUTABLE(format-card format-card.cpp backend-comms.c crypto.c)
ADD_EXECUTABLE(test-crypto test-crypto.c crypto.c)

include_directories("include" ${LIBNFC_INCLUDE} ${LIBFREEFARE_INCLUDE} ${CURL_INCLUDE_DIR} ${OpenSSL_INCLUDE_DIR} ${ACTIONS_INCLUDE})

set(EXTERNAL_LIBS ${LIBNFC_LDFLAGS} ${LIBFREEFARE_LDFLAGS} ${LIBUSB_LDFLAGS} ${CURL_LIBRARIES} ${OPENSSL_CRYPTO_LIBRARY} )
target_link_libraries(door-system ${EXTERNAL_LIBS})
#target_link_libraries(format-card ${EXTERNAL_LIBS})
target_link_libraries(test-crypto ${EXTERNAL_LIBS})
if (BCM2835_FOUND)
	target_link_libraries(door-system ${BCM2835_LIBRARIES})

endif (BCM2835_FOUND)

if (DIRECT_FOUND AND DIRECTFB_FOUND) 
    ADD_EXECUTABLE(cutter-gui key-cutter/cutter-gui.c key-cutter/cutter-screens.c key-cutter/format-common.c crypto.c backend-comms.c )
    include_directories(${DIRECT_INCLUDE} ${DIRECTFB_INCLUDE_DIRS})
    target_link_libraries(cutter-gui ${DIRECTFB_LDFLAGS} ${DIRECT_LDFLAGS} 
            ${CURL_LIBRARIES} ${LIBNFC_LDFLAGS} ${LIBFREEFARE_LDFLAGS})
endif (DIRECT_FOUND AND DIRECTFB_FOUND)

ADD_EXECUTABLE(format-card key-cutter/format-card.c key-cutter/format-common.c crypto.c backend-comms.c)
target_link_libraries(format-card ${CURL_LIBRARIES} ${LIBNFC_LDFLAGS} ${LIBFREEFARE_LDFLAGS})
set(CMAKE_C_FLAGS "-g -O0 -Wall -std=c99")
set(CMAKE_CXX_FLAGS "-g -Wall")

#set(CMAKE_EXE_LINKER_FLAGS ${LIBNFC_LDFLAGS} ${LIBFREEFARE_LDFLAGS} ${LIBUSB_LDFLAGS})
