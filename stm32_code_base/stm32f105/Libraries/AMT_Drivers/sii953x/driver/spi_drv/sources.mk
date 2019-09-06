ifneq (${INC_SPI},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_spi.o

VPATH += driver/spi_drv
endif