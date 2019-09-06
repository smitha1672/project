ifneq (${INC_CBUS},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_cbus.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_cbus_cec.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_cbus_demo.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_cbus_wake_sequence.o

VPATH += application/app_cbus
endif