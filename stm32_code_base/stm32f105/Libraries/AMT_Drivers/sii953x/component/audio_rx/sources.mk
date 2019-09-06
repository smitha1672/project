ifneq (${INC_AUDIO_RX},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_audio_component.o
VPATH += component/audio_rx
endif