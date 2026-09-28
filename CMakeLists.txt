cmake_minimum_required(VERSION 3.16)

project(NDTable LANGUAGES C CXX)

function(enable_project_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

add_library(NDTable
    src/Core.c
    src/Interpolation.c
)

enable_project_warnings(NDTable)

target_include_directories(NDTable
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

include(CTest)

if(BUILD_TESTING)
    add_subdirectory(external/Catch2)

    add_executable(NDTable_test
        test/CoreTests.cpp
        test/InterpolationTests.cpp
    )

    target_compile_features(NDTable_test PRIVATE cxx_std_14)
    target_link_libraries(NDTable_test PRIVATE NDTable Catch2::Catch2WithMain)
    enable_project_warnings(NDTable_test)

    add_test(NAME NDTable_test COMMAND NDTable_test)
endif()
