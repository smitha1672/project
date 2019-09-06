ifneq (${INC_MSW},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_msw.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_msw_hpd.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_msw_info.o

VPATH += driver/matrix_sw_drv
endif