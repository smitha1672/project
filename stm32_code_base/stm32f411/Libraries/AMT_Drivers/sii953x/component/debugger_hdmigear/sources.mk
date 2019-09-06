ifneq (${INC_DEBUGGER},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_debugger_hdmigear.o

VPATH += component/debugger_hdmigear
endif
