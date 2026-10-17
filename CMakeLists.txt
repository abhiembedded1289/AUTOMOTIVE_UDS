# Build configuration for the portable C99 AUTOSAR-style UDS simulator.
cmake_minimum_required(VERSION 3.16)
project(autosar_uds_simulator C)

# Require standard C99 without compiler-specific language extensions.
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# List all source modules that are compiled into the simulator executable.
add_executable(autosar_uds_simulator
    src/main.c
    src/sim_can.c
    src/can_if.c
    src/can_tp.c
    src/pdu_router.c
    src/dcm.c
    src/rte_demo_app.c
    src/ecum.c
)

# Let the compiler find the project's public module interfaces.
target_include_directories(autosar_uds_simulator PRIVATE include)

# Enable useful warnings on both MSVC and GCC/Clang toolchains.
if(MSVC)
    target_compile_options(autosar_uds_simulator PRIVATE /W4)
else()
    target_compile_options(autosar_uds_simulator PRIVATE -Wall -Wextra -Wpedantic)
endif()
