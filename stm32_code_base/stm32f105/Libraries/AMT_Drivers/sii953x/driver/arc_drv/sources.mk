ifneq (${INC_HEAC},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_arc.o

VPATH += driver/arc_drv
endif