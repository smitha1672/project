ifneq (${INC_CDC},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_cdc_demo.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_cdc.o

VPATH += application/app_cdc
endif