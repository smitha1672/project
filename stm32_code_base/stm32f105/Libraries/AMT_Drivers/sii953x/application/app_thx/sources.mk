ifneq (${INC_THX},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_thx.o
VPATH += application/app_thx
endif