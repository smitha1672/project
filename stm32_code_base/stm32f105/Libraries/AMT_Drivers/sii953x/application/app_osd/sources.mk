ifneq (${INC_OSD},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_osd.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_osd_avr.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_osd_audio.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_osd_demo.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_osd_info.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_osd_lb_demo.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_osd_options.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_osd_portsel.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_osd_speaker.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_osd_video.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_osd_volume.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_osd_resource_data.o
VPATH += application/app_osd
endif