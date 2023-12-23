file(GLOB_RECURSE sources "*.cpp" "*.h")
add_library(goto-symex ${sources} interrupt_atomic_violation_check.cpp interrupt_atomic_violation_check.cpp interrupt_atomic_violation_check.h)
generic_includes(goto-symex)

if(CMAKE_USE_CUDD)
    target_include_directories(goto-symex PUBLIC ${CUDD_INCLUDE}/cudd/)
endif()

target_link_libraries(goto-symex util)
