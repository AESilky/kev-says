# Library: UI (obj only)
add_library(ui INTERFACE)

target_sources(ui INTERFACE
    ui.c
    ui_disp.c
)

add_subdirectory(display)

target_link_libraries(ui INTERFACE
    pico_stdlib
)
