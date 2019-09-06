ifneq (${FPGA_BUILD},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_fpga.o

VPATH += platform/hal/fpga
endif