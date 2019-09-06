ifneq (${INC_TX},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_tpi_system.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_tpi_access.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_tpi_ddc_access.o

VPATH += driver/tpi_drv
endif