${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_sii953x.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_3dgpio.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_cra_cfg.o

VPATH += driver/sii953x_drv
VPATH += application
