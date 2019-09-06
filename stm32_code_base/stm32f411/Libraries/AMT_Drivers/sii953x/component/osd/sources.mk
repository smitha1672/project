ifneq (${INC_OSD},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_osd_component.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_osd_window.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_osd_handler.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_osd_win_edit.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_osd_win_listbox.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_osd_win_menu.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_osd_win_msgbox.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_osd_win_slider.o

VPATH += component/osd
endif