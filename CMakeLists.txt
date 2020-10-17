cmake_minimum_required(VERSION 3.16)
set(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_VERSION 1)

set(MCU ATmega328)
set(AVRDUDE C:/gcc-avr/avrdude.exe)

project(sunrise CXX C)

add_compile_definitions("__AVR_${MCU}__")

add_link_options(-Wl,--print-memory-usage)

add_executable(${PROJECT_NAME}
    main.h
    main.c
    fsm.h
    fsm.c
    ir.h
    ir.c
    pwm.h
    pwm.c
    uart.h
    uart.c
    )

add_custom_target(FLASH
    ${AVRDUDE} -p ${MCU} -c usbasp -D -U flash:w:${PROJECT_BINARY_DIR}/${PROJECT_NAME}:i
    DEPENDS ${PROJECT_NAME}
    COMMENT "Flash to ${MCU}")
