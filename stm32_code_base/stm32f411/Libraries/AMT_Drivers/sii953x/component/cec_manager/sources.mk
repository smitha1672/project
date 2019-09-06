ifneq (${INC_CEC},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_cec_component.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_cec_controller.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_cec_rx.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_cec_tx.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_cec_timer.o

VPATH += component/cec_manager
endif