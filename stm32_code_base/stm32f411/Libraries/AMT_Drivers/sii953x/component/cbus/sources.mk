ifneq (${INC_CBUS},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_cbus_component.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_cbus_rcp_parse.o

VPATH += component/cbus
endif