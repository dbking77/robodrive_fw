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
