ifneq (${INC_REPEATER_AVR},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_repeater_component.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_connection_monitor.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_repeater_isr.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_repeater_sha.o

VPATH += component/repeater_avr
endif
