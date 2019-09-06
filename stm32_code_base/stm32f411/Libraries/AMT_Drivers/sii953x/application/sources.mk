ifneq (${INC_AUDIO_RX},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_audio_rx.o
endif
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_demo.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_device.o
ifneq (${INC_EDID},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_edid.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_ediddata.o
endif
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_gpio.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_infrared.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_repeater.o
ifneq (${INC_SETUP},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_setup.o
endif
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_support.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_tv.o
ifneq (${INC_TX},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_tx.o
endif
ifneq (${INC_BENCH_TEST},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk_app_btst.o
endif
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/sk953x_avr_repeater.o
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/startup_gcc.o

include application/app_arc/sources.mk
include application/app_cbus/sources.mk
include application/app_cdc/sources.mk
include application/app_cec/sources.mk
include application/app_diag/sources.mk
include application/app_emu/sources.mk
include application/app_ipv/sources.mk
include application/app_msw/sources.mk
include application/app_osd/sources.mk
include application/app_thx/sources.mk
VPATH += application
VPATH += component
VPATH += driver

 
