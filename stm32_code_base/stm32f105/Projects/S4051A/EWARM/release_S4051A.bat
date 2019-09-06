ECHO OFF

SET VERSION=%1

SET MODEL=S4051A
SET sMODEL=s4051a


SET OUT_FILE=%MODEL%.bin
SET SMT_FILE=%MODEL%_SMT.bin
SET STM32_FILE=stm32.bin
SET DSP_FILE=DSPuld.bin
SET DSP_CHECK_SUM_FILE=DSPchecksum.bin
SET CCK_FILE=NoiseSound.bin
SET CCK_VERSION_FILE=NoiseVersion.bin
SET DSP_IMAGE_PATH=..\cs495314_dsp_bin\%MODEL%
SET CCK_IMAGE_PATH=..\voice_checking_bin
SET APP_PATH=..\Output\%MODEL%
SET SMT_PATH=..\Output\%MODEL%\Release
SET REL_PATH=..\Output\%MODEL%_V%VERSION%_RELEASE
SET ONEBIN_PATH=OutputBinFile

SET OUT_FILE=%MODEL%.bin
SET OUT_SMT_FILE=%MODEL%_SMT.bin

SET OUT_FLASH_V_FILE=%MODEL%_V%VERSION%_SPI_FLASH.bin
SET OUT_SMT_V_FILE=%MODEL%_V%VERSION%_SMT.bin

ECHO ==================== S T A R T   B U I L D I N G ===============================

rmdir /S ..\OUTPUT

ECHO ******************** B U I L D   %MODEL% A P P L I C A T I O N **************
iarbuild %MODEL%\%sMODEL%_app.ewp -clean APP_%MODEL% -log info
iarbuild %MODEL%\%sMODEL%_app.ewp -make APP_%MODEL% -log info

ECHO ******************** B U I L D   %MODEL%  R E L E A S E ********************
iarbuild %MODEL%\%sMODEL%_loader.ewp -clean %MODEL%_RELEASE -log info
iarbuild %MODEL%\%sMODEL%_loader.ewp -make %MODEL%_RELEASE -log info


ECHO ******************** C O P Y   B I N A R A Y ***********************************
rmdir /s %REL_PATH%
mkdir %REL_PATH%  
copy %APP_PATH%\%OUT_FILE% %REL_PATH%\%STM32_FILE%
copy %SMT_PATH%\%SMT_FILE% %REL_PATH%\
copy %CCK_IMAGE_PATH%\*.bin %REL_PATH%\
copy %DSP_IMAGE_PATH%\*.bin %REL_PATH%\ 

ECHO ******************** C O N B I N E  F I L E ************************************

rmdir %REL_PATH%\%ONEBIN_PATH%
mkdir %REL_PATH%\%ONEBIN_PATH%

copy /b %REL_PATH%\stm32.bin + %REL_PATH%\%CCK_VERSION_FILE% + %REL_PATH%\%DSP_CHECK_SUM_FILE% + %REL_PATH%\NoiseSound.bin + %REL_PATH%\flash_image_dsp.bin %REL_PATH%\%OUT_FILE%
copy /b %REL_PATH%\%OUT_SMT_FILE% %REL_PATH%\%ONEBIN_PATH%\%OUT_SMT_V_FILE%
copy /b %REL_PATH%\%CCK_VERSION_FILE% + %REL_PATH%\%DSP_CHECK_SUM_FILE% + %REL_PATH%\NoiseSound.bin + %REL_PATH%\flash_image_dsp.bin %REL_PATH%\%ONEBIN_PATH%\%OUT_FLASH_V_FILE%
del %REL_PATH%\stm32.bin %REL_PATH%\NoiseSound.bin %REL_PATH%\flash_image_dsp.bin %REL_PATH%\%OUT_SMT_FILE% %REL_PATH%\%DSP_CHECK_SUM_FILE% %REL_PATH%\%CCK_VERSION_FILE%

ECHO =========== S T A R T   B U I L D I N G  F I N I S H ================
