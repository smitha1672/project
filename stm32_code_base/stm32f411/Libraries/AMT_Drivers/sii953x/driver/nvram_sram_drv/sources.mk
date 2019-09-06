ifneq (${INC_NVRAM},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_nvram_sram.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_edid_rx.o

VPATH += driver/nvram_sram_drv
endif