idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES driver nopnop2002__esp_idf_ssd1306
    PRIV_REQUIRES spi_flash
)
