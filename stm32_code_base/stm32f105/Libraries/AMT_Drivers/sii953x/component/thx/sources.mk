ifneq (${INC_THX},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_thx_component.o

VPATH += component/thx
endif
