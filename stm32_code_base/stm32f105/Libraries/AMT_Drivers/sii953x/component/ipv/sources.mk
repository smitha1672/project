ifneq (${INC_IPV},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_ipv_component.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_ipv_data.o

VPATH += component/ipv
endif