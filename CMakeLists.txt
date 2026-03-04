include(FetchContent)

########## CMake Project Requirements ##########

cmake_minimum_required(VERSION 3.21)
project(allocator LANGUAGES CXX)

set(ENABLE_WORKFLOW OFF)
if((NOT CMAKE_BUILD_TYPE STREQUAL "Release") AND PROJECT_IS_TOP_LEVEL)
    set(ENABLE_WORKFLOW ON)
endif()

########## Utilities ##########

# Checks if the program exists and has correct version #
function(check_program_version out_var program_name required_version)
    find_program(PROGRAM_PATH_${out_var} ${program_name})
    if(NOT PROGRAM_PATH_${out_var})
        message(FATAL_ERROR "${program_name} not found")
    endif()

    execute_process(
        COMMAND ${PROGRAM_PATH_${out_var}} --version
        OUTPUT_VARIABLE VERSION_RAW
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    string(REGEX MATCH "[0-9]+\\.[0-9]+(\\.[0-9]+)?" VERSION_EXTRACTED
           "${VERSION_RAW}")
    if(VERSION_EXTRACTED VERSION_LESS ${required_version})
        message(
            FATAL_ERROR
                "${program_name} version ${required_version}+ required, found ${VERSION_EXTRACTED}"
        )
    endif()

    set(${out_var}
        ${PROGRAM_PATH_${out_var}}
        PARENT_SCOPE)
endfunction()

########## File/Folder Definitions ##########

# Build directories
set(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(BIN_DIR ${BUILD_DIR}/bin)
set(DOC_DIR ${BUILD_DIR}/documentation)

# Source directory
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
file(GLOB_RECURSE SRC_H ${SRC_DIR}/*.hpp)

# Test directory
set(TEST_DIR ${PROJECT_SOURCE_DIR}/tests)
file(GLOB_RECURSE TEST CONFIGURE_DEPENDS ${TEST_DIR}/*.cpp)

# Documentation directories
set(DOC_COVERAGE_DIR ${DOC_DIR}/coverage)
set(DOC_DOXYGEN_DIR ${DOC_DIR}/doxygen)

########## C++ Shared Compilation Flags ##########

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${SRC_DIR})

# Debug config
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -fno-omit-frame-pointer")

# Release config
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Global warnings
add_compile_options(-Wall -Wextra -pedantic)

########## Targets ##########

file(MAKE_DIRECTORY ${BIN_DIR})

##### test executables #####

add_executable(test_slab      EXCLUDE_FROM_ALL ${TEST_DIR}/test_slab.cpp)
add_executable(test_arena     EXCLUDE_FROM_ALL ${TEST_DIR}/test_arena.cpp)
add_executable(test_hybrid    EXCLUDE_FROM_ALL ${TEST_DIR}/test_hybrid.cpp)
add_executable(test_slab_nd   EXCLUDE_FROM_ALL ${TEST_DIR}/test_slab_overrides.cpp)
add_executable(test_arena_nd  EXCLUDE_FROM_ALL ${TEST_DIR}/test_arena_overrides.cpp)
add_executable(test_hybrid_nd EXCLUDE_FROM_ALL ${TEST_DIR}/test_hybrid_overrides.cpp)

set(ALL_TESTS test_slab test_arena test_hybrid
              test_slab_nd test_arena_nd test_hybrid_nd)

foreach(t ${ALL_TESTS})
    set_target_properties(${t} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
endforeach()

enable_testing()
add_test(NAME slab      COMMAND ${BIN_DIR}/test_slab)
add_test(NAME arena     COMMAND ${BIN_DIR}/test_arena)
add_test(NAME hybrid    COMMAND ${BIN_DIR}/test_hybrid)
add_test(NAME slab_nd   COMMAND ${BIN_DIR}/test_slab_nd)
add_test(NAME arena_nd  COMMAND ${BIN_DIR}/test_arena_nd)
add_test(NAME hybrid_nd COMMAND ${BIN_DIR}/test_hybrid_nd)

##### compile (build all tests) #####

add_custom_target(compile
                  DEPENDS ${ALL_TESTS}
                  COMMENT "Compiles all test executables")

if(ENABLE_WORKFLOW)

##### test (run ctest) #####

add_custom_target(run_tests
                  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                  DEPENDS ${ALL_TESTS}
                  COMMENT "Runs all tests")

##### coverage #####

# Add coverage flags to all test targets
foreach(t ${ALL_TESTS})
    target_compile_options(${t} PRIVATE --coverage)
    target_link_options(${t} PRIVATE --coverage)
endforeach()

file(MAKE_DIRECTORY ${DOC_COVERAGE_DIR})

check_program_version(GCOVR gcovr 6.0)
set(GCOVR_CONFIG ${PROJECT_SOURCE_DIR}/gcovr.cfg)

add_custom_target(
    coverage
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    COMMAND ${GCOVR} -r ${PROJECT_SOURCE_DIR}
            --config ${GCOVR_CONFIG}
    WORKING_DIRECTORY ${BUILD_DIR}
    COMMENT "Runs tests and generates coverage report"
    VERBATIM
    DEPENDS ${ALL_TESTS})

##### linting #####

check_program_version(CPPLINT cpplint 1.6)

add_custom_target(
    lint
    COMMAND ${CPPLINT} --recursive ${SRC_DIR} ${TEST_DIR}
    DEPENDS ${SRC_H} ${TEST}
    COMMENT "Checks Google C++ Style on code")

##### documentation #####

find_package(Doxygen 1.9 QUIET)

if(DOXYGEN_FOUND)
    file(MAKE_DIRECTORY ${DOC_DOXYGEN_DIR})

    add_custom_target(
        documentation
        COMMAND doxygen ${PROJECT_SOURCE_DIR}/Doxyfile
        COMMAND ${CMAKE_COMMAND} -E copy
                ${PROJECT_SOURCE_DIR}/docs/index.html ${DOC_DIR}/index.html
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        VERBATIM
        DEPENDS ${SRC_H}
        COMMENT "Generates Doxygen documentation")
else()
    message(STATUS "Doxygen not found — documentation target disabled")
    add_custom_target(documentation
                      COMMAND ${CMAKE_COMMAND} -E echo "Doxygen not found, skipping docs"
                      COMMENT "Doxygen not available")
endif()

##### default workflow target #####

add_custom_target(default ALL
                  COMMENT "The default target")
add_dependencies(default documentation)
add_dependencies(documentation lint)
add_dependencies(lint coverage)
add_dependencies(coverage run_tests)
add_dependencies(run_tests compile)

else()

##### release: just compile #####

add_custom_target(default ALL
                  COMMENT "The default target")
add_dependencies(default compile)

endif()

##### clean_all #####

add_custom_target(clean_all
                  COMMAND rm -rf ${BUILD_DIR}
                  COMMENT "Cleans the build folder")
