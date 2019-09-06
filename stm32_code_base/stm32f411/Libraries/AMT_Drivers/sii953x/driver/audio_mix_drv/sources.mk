ifneq (${INC_AUDIO_MIX},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_audio_mix.o

VPATH += driver/audio_mix_drv
endif
