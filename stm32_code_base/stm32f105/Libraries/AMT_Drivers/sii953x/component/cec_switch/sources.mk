ifneq (${INC_CEC_SWITCH},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_cec_switch_api.o
VPATH += component/cec_switch
endif