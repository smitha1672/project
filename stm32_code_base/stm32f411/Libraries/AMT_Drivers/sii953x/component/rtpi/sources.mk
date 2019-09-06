ifneq (${INC_RTPI},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_rtpi_component.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_rtpi_register_functions.o
ifneq (${INC_CBUS}, 0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_rtpi_cbus.o
endif
ifneq (${INC_CEC}, 0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_rtpi_cec.o
endif
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_rtpi_default_reg.o
VPATH += component/rtpi
endif