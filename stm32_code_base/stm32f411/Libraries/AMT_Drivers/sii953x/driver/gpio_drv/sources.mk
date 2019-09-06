ifneq (${INC_GPIO},0)
${TARGET_EXECUTABLE}: ${TARGET_FOLDER}/si_drv_gpio.o

VPATH += driver/gpio_drv
endif