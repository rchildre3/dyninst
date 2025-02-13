cmake_minimum_required(VERSION 3.4.0)

# There is a bug in 3.19.0 that causes .S files to be treated like C files
if(CMAKE_VERSION VERSION_EQUAL "3.19.0")
    message(FATAL_ERROR "Dyninst cannot use CMake version 3.19.0")
endif()

project(Dyninst)

set(DYNINST_ROOT ${PROJECT_SOURCE_DIR})
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(STERILE_BUILD
    ON
    CACHE BOOL "Do not download/build any third-party dependencies from source")

list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib"
     isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif()

set(RT_SOURCE_DIR ${DYNINST_ROOT}/dyninstAPI_RT)
set(RT_BINARY_DIR ${PROJECT_BINARY_DIR}/dyninstAPI_RT)

set(CMAKE_MODULE_PATH "${DYNINST_ROOT}/cmake" "${DYNINST_ROOT}/cmake/Modules"
                      ${CMAKE_MODULE_PATH})

# Set the C and C++ language standards
include(LanguageStandards)

# Find the necessary third-party libraries
find_package(ThreadDB)
find_package(Threads)
include(Boost)
include(ThreadingBuildingBlocks)
include(ElfUtils)

if(UNIX)
    include(LibIberty REQUIRED)
endif()

include(shared)

if(USE_OpenMP)
    find_package(OpenMP REQUIRED)
endif()

configure_file(cmake/version.h.in common/h/dyninstversion.h)
include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_BINARY_DIR}/common/h)
set(HEADER_DIRS
    common
    dataflowAPI
    dyninstAPI
    instructionAPI
    parseAPI
    patchAPI
    proccontrol
    stackwalk
    symtabAPI)
if(NOT ${PLATFORM} MATCHES nt)
    set(HEADER_DIRS ${HEADER_DIRS} dwarf elf symlite)
endif()

foreach(dir ${HEADER_DIRS})
    include_directories(${DYNINST_ROOT}/${dir}/h)
endforeach()

set(ADD_VALGRIND_ANNOTATIONS
    OFF
    CACHE BOOL "Enable annotations for Valgrind analysis")
if(ADD_VALGRIND_ANNOTATIONS)
    find_package(Valgrind REQUIRED)
    include_directories(${Valgrind_INCLUDE_DIRS})
    add_definitions(-DENABLE_VG_ANNOTATIONS)
endif()

include_directories(${DYNINST_ROOT} ${DYNINST_ROOT}/external ${TBB_INCLUDE_DIRS})

# Component time
add_subdirectory(common)
if(NOT ${PLATFORM} MATCHES nt)
    add_subdirectory(elf)
    add_subdirectory(dwarf)
    add_subdirectory(symlite)
endif()
add_subdirectory(instructionAPI)
add_subdirectory(symtabAPI)
add_subdirectory(parseAPI)
add_subdirectory(proccontrol)
add_subdirectory(stackwalk)
add_subdirectory(patchAPI)

if(${SYMREADER} MATCHES symtabAPI)
    add_subdirectory(dyninstAPI)
    add_subdirectory(dynC_API)
    add_subdirectory(parseThat)
endif()

# Build the RT library as a separate project so we can change compilers
message(STATUS "Configuring DyninstAPI_RT in ${RT_BINARY_DIR}")
file(REMOVE_RECURSE ${RT_BINARY_DIR}/CMakeCache.txt ${RT_BINARY_DIR}/CMakeFiles
     ${RT_BINARY_DIR}/Makefile)
file(MAKE_DIRECTORY ${RT_BINARY_DIR})
set(RT_C_COMPILER
    ${CMAKE_C_COMPILER}
    CACHE STRING "Compiler for runtime library")
set(ENABLE_STATIC_LIBS
    NO
    CACHE STRING "Build static libraries as well?")
message(STATUS "Configuring RT library")

execute_process(
    WORKING_DIRECTORY ${RT_BINARY_DIR}
    COMMAND
        ${CMAKE_COMMAND} "-DCMAKE_C_COMPILER=${RT_C_COMPILER}"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}"
        "-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}" "-DINSTALL_LIB_DIR=${INSTALL_LIB_DIR}"
        "-DINSTALL_INCLUDE_DIR=${INSTALL_INCLUDE_DIR}" "-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}"
        "-DBUILD_RTLIB_32=${BUILD_RTLIB_32}" "-DPLATFORM=${PLATFORM}"
        "-DDYNINST_DISABLE_DIAGNOSTIC_SUPPRESSIONS=${DYNINST_DISABLE_DIAGNOSTIC_SUPPRESSIONS}"
        "-DDYNINST_EXTRA_WARNINGS=${DYNINST_EXTRA_WARNINGS}"
        "-DDYNINST_WARNINGS_AS_ERRORS=${DYNINST_WARNINGS_AS_ERRORS}"
        "-DDYNINST_CXX_LANGUAGE_STANDARD=${DYNINST_CXX_LANGUAGE_STANDARD}"
        "-DDYNINST_C_LANGUAGE_STANDARD=${DYNINST_C_LANGUAGE_STANDARD}" -G
        "${CMAKE_GENERATOR}" -B "${RT_BINARY_DIR}" -S "${RT_SOURCE_DIR}")
find_file(
    ${RT_MAKEFILE} Makefile
    PATHS ${RT_BINARY_DIR}
    NO_DEFAULT_PATH)
message(STATUS "RTlib Makefile: ${RT_MAKEFILE}")
if(MSVC)
    include_external_msproject(DyninstAPI_RT dyninstAPI_RT/dyninstAPI_RT.vcxproj)
    include_external_msproject(DyninstAPI_RT_static
                               dyninstAPI_RT/dyninstAPI_RT_static.vcxproj)
else()
    add_custom_target(
        DyninstRT ALL
        $(MAKE)
        WORKING_DIRECTORY ${RT_BINARY_DIR}
        COMMENT "Building DyninstRT")
    if(TARGET TBB)
        add_dependencies(DyninstRT TBB)
    endif()
    if(TARGET dyninstAPI)
        add_dependencies(dyninstAPI DyninstRT)
    endif()
    if(TARGET dyninstAPI-static)
        add_dependencies(dyninstAPI-static DyninstRT)
    endif()
    if(TARGET boost)
        add_dependencies(DyninstRT boost)
    endif()
endif()

install(SCRIPT "${RT_BINARY_DIR}/cmake_install.cmake")

set(VERSION_STRING
    "${DYNINST_MAJOR_VERSION}.${DYNINST_MINOR_VERSION}.${DYNINST_PATCH_VERSION}")
set(DYNINST_NAME "DyninstAPI-${VERSION_STRING}")

if(TARGET boost)
    add_dependencies(common boost)
endif()

# add_subdirectory(testsuite)

# Copy out all of the cmake files so they can be used by the Testsuite
install(DIRECTORY ${DYNINST_ROOT}/cmake/ DESTINATION ${INSTALL_CMAKE_DIR})
install(FILES ${PROJECT_BINARY_DIR}/CMakeCache.txt DESTINATION ${INSTALL_CMAKE_DIR})
