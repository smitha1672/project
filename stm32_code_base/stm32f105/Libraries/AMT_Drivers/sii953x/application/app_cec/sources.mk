ifneq (${INC_CEC},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_cec.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_cec_logger.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_cec_demo.o
VPATH += application/app_cec
endif