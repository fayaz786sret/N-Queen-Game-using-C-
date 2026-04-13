cmake_minimum_required(VERSION 3.14)
project(NQueens LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Uncomment and set your SFML path if cmake can't find it: ──
# set(SFML_DIR "C:/SFML-2.6.1/lib/cmake/SFML")

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(NQueens main.cpp)
target_link_libraries(NQueens sfml-graphics sfml-window sfml-system)

# Auto-copy SFML DLLs next to the .exe on Windows
if(WIN32)
    add_custom_command(TARGET NQueens POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:sfml-graphics>
            $<TARGET_FILE:sfml-window>
            $<TARGET_FILE:sfml-system>
            $<TARGET_FILE_DIR:NQueens>
    )
endif()
