cmake_minimum_required(VERSION 3.0.0)
project(traverse.cpp VERSION 0.1.0)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set (EXAMPLE_DIRECTORY "${CMAKE_SOURCE_DIR}/examples")
set(INCLUDE_DIRECTORY "${CMAKE_SOURCE_DIR}/include")
include(CPack)

add_library(traversecpp INTERFACE)
target_include_directories(traversecpp INTERFACE ${INCLUDE_DIRECTORY})

function(make_example EXAMPLE_NAME)

add_executable(${EXAMPLE_NAME} "${EXAMPLE_DIRECTORY}/${EXAMPLE_NAME}.cpp")
target_link_libraries(${EXAMPLE_NAME} PRIVATE traversecpp)

if (MSVC) 
    target_compile_options(${EXAMPLE_NAME} PRIVATE "/W4" "/permissive-")
else()
    target_compile_options(${EXAMPLE_NAME} PRIVATE -Werror -Wall -Wextra -pedantic)
endif()

add_test(${EXAMPLE_NAME} ${EXAMPLE_NAME})

endfunction()

make_example(traverse)
make_example(customization_points)
make_example(is_traversable)
make_example(composite)
make_example(fold)
