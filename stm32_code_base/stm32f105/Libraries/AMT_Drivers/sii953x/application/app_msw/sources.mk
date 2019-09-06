ifneq (${INC_MSW},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_msw_demo.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_switch.o
VPATH += application/app_msw
endif