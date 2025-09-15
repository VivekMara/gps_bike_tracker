idf_component_register(SRCS "main.c"
                       PRIV_REQUIRES spi_flash
                       PRIV_REQUIRES esp_driver_uart
                       PRIV_REQUIRES esp_driver_gpio
                       INCLUDE_DIRS "")
