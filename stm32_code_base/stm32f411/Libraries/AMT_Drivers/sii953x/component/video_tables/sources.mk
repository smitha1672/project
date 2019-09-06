ifneq (${INC_VIDEO_TABLES},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_video_tables.o

VPATH += component/video_tables
endif