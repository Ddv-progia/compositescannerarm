PROJECT(CompositeScanner)
CMAKE_MINIMUM_REQUIRED(VERSION 3.8.2)

IF(NOT ${CMAKE_VERSION} VERSION_LESS "3.10.0")
  CMAKE_POLICY(SET CMP0071 NEW)
ENDIF()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /await /bigobj /MP /wd4250 /wd4251 /wd4275 /wd4348 /wd4503")

string(REPLACE "/debug" "/debug:fastlink" CMAKE_EXE_LINKER_FLAGS_DEBUG             "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
string(REPLACE "/debug" "/debug:fastlink" CMAKE_MODULE_LINKER_FLAGS_DEBUG          "${CMAKE_MODULE_LINKER_FLAGS_DEBUG}")
string(REPLACE "/debug" "/debug:fastlink" CMAKE_SHARED_LINKER_FLAGS_DEBUG          "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")
string(REPLACE "/debug" "/debug:fastlink" CMAKE_STATIC_LINKER_FLAGS_DEBUG          "${CMAKE_STATIC_LINKER_FLAGS_DEBUG}")
string(REPLACE "/debug" "/debug:fastlink" CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO    "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}")
string(REPLACE "/debug" "/debug:fastlink" CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO}")
string(REPLACE "/debug" "/debug:fastlink" CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO}")
string(REPLACE "/debug" "/debug:fastlink" CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO}")

find_package(BerkeleyDB REQUIRED)
find_package(Boost 1.68 COMPONENTS filesystem log log_setup program_options system thread unit_test_framework REQUIRED)
find_package(FFTW3 REQUIRED)
find_package(Qt5 REQUIRED Core Gui Widgets Network Xml Sql PrintSupport Script ScriptTools)
find_package(Qwt REQUIRED)
find_package(OpenCV REQUIRED)
find_package(PugiXml REQUIRED)
find_package(VTK REQUIRED)

option(DEVTALK_ICE_CPP11 "Use the Ice C++11 mapping" OFF)
if(DEVTALK_ICE_CPP11)
  find_package(Ice COMPONENTS Ice++11 IceBox++11 IceGrid++11 IceStorm++11 IceDiscovery++11 REQUIRED)
  add_definitions("-D ICE_CPP11_MAPPING")
else()
  find_package(Ice COMPONENTS Ice IceBox IceGrid IceStorm IceDiscovery IceUtil REQUIRED)
endif()

#INCLUDE(DefineCxxAlternativeTokens)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_DEBUG_POSTFIX d)

set(DEVTALK_BUILD_ASYNC ON CACHE BOOL "Build Microsoft Concurrency Runtime wrappers")

#INCLUDE(ZeroCSlice)

add_subdirectory(libs/UCL)
add_subdirectory(iface/DevTalk)
add_subdirectory(iface/DevTalkCAND)
add_subdirectory(libs/DevTalk)
add_subdirectory(libs/DevTalkCAND)
add_subdirectory(libs/DevTalkTSD)
add_subdirectory(src)

#ADD_DEFINITIONS(
 # -DNOMINMAX
 # -DWIN32_LEAN_AND_MEAN
 # -D_SCL_SECURE_NO_WARNINGS
 # -D_CRT_SECURE_NO_WARNINGS
 # -DBOOST_ALL_NO_LIB
 # -DBOOST_ALL_DYN_LINK
 # -DBOOST_THREAD_VERSION=4
 # -DBOOST_USE_WINAPI_VERSION=0x0601
 # -D_WIN32_WINNT=0x0601)


IF(MSVC_VERSION GREATER 1800)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /await")
ENDIF()