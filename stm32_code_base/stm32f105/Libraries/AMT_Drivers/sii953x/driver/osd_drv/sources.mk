ifneq (${INC_OSD},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_osd.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_osd_window.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_osd_text.o
#${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_osd_draw.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_osd_ram.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_osd_spi.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_osd_tools.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_osd_resource.o

VPATH += driver/osd_drv
endif