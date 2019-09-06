ifneq (${INC_AUDIO_RX},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_audio.o

VPATH += driver/audio_rx_drv
endif