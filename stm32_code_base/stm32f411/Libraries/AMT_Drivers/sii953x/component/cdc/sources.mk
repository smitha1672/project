ifneq (${INC_CDC},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_cdc_component.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_cdc_main.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_cdc_heac.o

VPATH += component/cdc
endif
