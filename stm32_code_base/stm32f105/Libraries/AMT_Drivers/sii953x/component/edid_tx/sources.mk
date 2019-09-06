ifneq (${INC_EDID_TX},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_edid_tx_component.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_edid_tx_analyzer.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_edid_tx_composer.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_edid_tx_converter.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_edid_tx_decode_h.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_edid_tx_decode_l.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_edid_tx_scaler.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_edid_tx_fmf.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_edid_tx_tables.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_edid_black_list.o

VPATH += component/edid_tx
endif