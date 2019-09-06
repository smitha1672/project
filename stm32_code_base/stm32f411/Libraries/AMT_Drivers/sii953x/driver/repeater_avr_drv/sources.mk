ifneq (${INC_REPEATER_AVR},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_repeater.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_repeater_sha.o
VPATH += driver/repeater_avr_drv
endif