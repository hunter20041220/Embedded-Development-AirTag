idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES driver ssd1306
    PRIV_REQUIRES spi_flash
)
