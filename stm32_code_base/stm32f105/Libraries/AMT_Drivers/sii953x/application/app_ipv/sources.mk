ifneq (${INC_IPV},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_ipv.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_ipv_demo.o
VPATH += application/app_ipv
endif