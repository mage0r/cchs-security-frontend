cmake_minimum_required(VERSION 2.6)
project(door-frontend)

# find libnfc
find_package(PkgConfig)

pkg_check_modules(LIBNFC REQUIRED libnfc)
pkg_check_modules(LIBFREEFARE REQUIRED libfreefare)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

ADD_EXECUTABLE(door-system door-system.c backend-comms.c actions.c)
ADD_EXECUTABLE(format-card format-card.cpp crypto.c)
ADD_EXECUTABLE(test-crypto test-crypto.c crypto.c)

include_directories("include" ${LIBNFC_INCLUDE} ${LIBFREEFARE_INCLUDE} ${CURL_INCLUDE_DIR} ${OpenSSL_INCLUDE_DIR})

set(EXTERNAL_LIBS ${LIBNFC_LDFLAGS} ${LIBFREEFARE_LDFLAGS} ${LIBUSB_LDFLAGS} ${CURL_LIBRARIES} ${OPENSSL_CRYPTO_LIBRARY} )
target_link_libraries(door-system ${EXTERNAL_LIBS})
target_link_libraries(format-card ${EXTERNAL_LIBS})
target_link_libraries(test-crypto ${EXTERNAL_LIBS})

set(CMAKE_C_FLAGS "-g -O0 -Wall -std=c99")
set(CMAKE_CXX_FLAGS "-g -Wall")
#set(CMAKE_EXE_LINKER_FLAGS ${LIBNFC_LDFLAGS} ${LIBFREEFARE_LDFLAGS} ${LIBUSB_LDFLAGS})
