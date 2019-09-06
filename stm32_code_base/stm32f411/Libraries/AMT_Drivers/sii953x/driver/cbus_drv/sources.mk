ifneq (${INC_CBUS},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_cbus.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_cbus_main.o

VPATH += driver/cbus_drv
endif