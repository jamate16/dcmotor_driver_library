add_library(driver STATIC
    Driver.h
    Driver.cpp
)

# pico_generate_pio_header(encoder ${CMAKE_CURRENT_LIST_DIR}/quadrature_encoder.pio)

# This will link all the required libraries from RP2040's SDK
# in case it has already been linked from another proprietary
# library, the guard clauses in the preprocesor directives
# will avoid duplicate code
target_link_libraries(driver PUBLIC
    pico_stdlib
    hardware_pwm
    hardware_gpio
    hardware_clocks
)

target_include_directories(driver PUBLIC ${CMAKE_CURRENT_LIST_DIR})


