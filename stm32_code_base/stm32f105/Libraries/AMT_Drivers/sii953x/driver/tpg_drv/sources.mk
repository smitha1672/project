ifneq (${INC_IV},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_tpg.o

VPATH += driver/tpg_drv
endif
