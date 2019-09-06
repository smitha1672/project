ifneq (${INC_ARC},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_arc.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_arc_demo.o

VPATH += application/app_arc
endif