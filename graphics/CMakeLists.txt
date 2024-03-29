cmake_minimum_required(VERSION 3.18)
project(test_project C CXX ASM)
set(PICO_BOARD pico_w)

project(graphics)

add_library(graphics INTERFACE)

target_include_directories(graphics INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_sources(graphics INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/graphics.c
)

target_link_libraries(graphics INTERFACE
        hardware_dma
        hardware_irq
        hardware_pio
        pico_multicore
)

