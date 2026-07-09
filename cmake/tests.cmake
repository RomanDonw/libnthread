macro(test name sources)
    add_executable(${name} ${sources})
    target_link_libraries(${name} PRIVATE ${PROJECT_NAME})
    target_include_directories(${name} PRIVATE "${CMAKE_SOURCE_DIR}/include")
endmacro()

# =============================================================================

test(test0 "tests/0.c")