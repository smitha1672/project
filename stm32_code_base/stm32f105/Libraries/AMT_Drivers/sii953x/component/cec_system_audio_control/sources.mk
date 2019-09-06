ifneq (${INC_CEC_SAC},0)
${TARGET_EXECUTABLE}/${APP_NAME}.axf: ${TARGET_FOLDER}/si_sac_main.o
${TARGET_EXECUTABLE}/${APP_NAME}.axf: ${TARGET_FOLDER}/si_sac_msg.o
VPATH += component/cec_system_audio_control
endif
