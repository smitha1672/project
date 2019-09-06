ifneq (${INC_IPV},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_ipv.o

VPATH += driver/ipv_drv
endif