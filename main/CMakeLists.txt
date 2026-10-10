cmake_minimum_required(VERSION 3.16)

idf_component_register(
    SRCS
        "analog.c"
        "gpio.c"
        "gatt_server.c"
        "main.c"
        "motor_drv8424.c"
        "motor_drv8871.c"
        "robodrive_comm.c"
        "robodrive_mcpwm.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        bt
        esp_adc
        esp_driver_gpio
        esp_driver_mcpwm
)

# Select board with: idf.py -B build_revA -DROBODRIVE_BOARD_REV=A build
# The value is cached per build directory.
set(ROBODRIVE_BOARD_REV "" CACHE STRING "Robodrive board revision (A or B)")
if(NOT ROBODRIVE_BOARD_REV MATCHES "^[AB]$")
    message(FATAL_ERROR "Set board revision: idf.py -B build_revA -DROBODRIVE_BOARD_REV=A build  (or B)")
endif()
target_compile_definitions(${COMPONENT_LIB} PRIVATE ROBODRIVE_BOARD_REV_${ROBODRIVE_BOARD_REV})
