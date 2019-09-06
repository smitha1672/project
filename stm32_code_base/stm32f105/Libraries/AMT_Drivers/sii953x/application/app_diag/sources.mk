ifneq (${INC_DIAG},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_diag.o
VPATH += application/app_diag
endif