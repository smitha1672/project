
VPATH += application/app_emu
#${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_emu_aud.o
#${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_emu_main.o

ifneq (${INC_OSD},0)
#${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_emu_osd.o
endif

ifneq (${INC_SPI},0)
#${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_emu_spi.o
endif