ifneq (${INC_CEC},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_cpi.o

VPATH += driver/cpi_drv
endif