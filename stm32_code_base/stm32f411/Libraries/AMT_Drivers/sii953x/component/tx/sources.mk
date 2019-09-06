ifneq (${INC_TX},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_video_tables.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_tx_component.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_tx_audio.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_tx_avi.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_tx_info_frm.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_tx_video.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_tx_hdcp.o


VPATH += component/tx
endif