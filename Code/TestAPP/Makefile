all: ArdPort.bin

hw_test: hwtest.bin

TOOLCHAIN = C:/Users/Francesco/AppData/Local/Arduino15/packages/arduino/tools/arm-none-eabi-gcc/4.8.3-2014q1/bin/

CC = $(TOOLCHAIN)arm-none-eabi-gcc
CXX = $(TOOLCHAIN)arm-none-eabi-g++
OBJCOPY = $(TOOLCHAIN)arm-none-eabi-objcopy

INCLUDES = -I. -ICMSIS/Include \
	-Ivariants/AB1000 \
	-Ilibraries/Adafruit_GFX_Library \
	-Ilibraries/Adafruit_SSD1306 \
	-Ilibraries/ASF -Ilibraries/ASF/samd21/include \
	-Ilibraries/ArduinoJson \
	-Ilibraries/I2C_MASTER_DMA \
	-Ilibraries/OneButton \
	-Ilibraries/PID \
	-Ilibraries/RWW_EEPROM \
	-Ilibraries/SparkFun_ESP8266 \
	-Ilibraries/SPI \
	-Ilibraries/SpiFlash \
	-Ilibraries/ThingSpeak \
	-Ilibraries/U8glib \
	-Ilibraries/Wire \
	-Iarduino

DEFINES = -DF_CPU=48000000L -DARDUINO=10607 -DARDUINO_SAMD_ZERO -DARDUINO_ARCH_SAMD -D__SAMD21G18A__ -DBOARD=SAMD21_XPLAINED_PRO -DUSB_VID=0x1B4F -DUSB_PID=0x8D21 -DUSBCON '-DUSB_MANUFACTURER="Arzaman"' '-DUSB_PRODUCT="AB1000"' -DSSD1306 -DMAKEFILE_BUILD

CFLAGS = -mcpu=cortex-m0plus -mthumb -c -g -Os -ffunction-sections -fdata-sections -nostdlib --param max-inline-insns-single=500 -fno-exceptions -MMD $(DEFINES) $(INCLUDES)
CXXFLAGS = -mcpu=cortex-m0plus -mthumb -c -g -Os -std=gnu++11 -ffunction-sections -fdata-sections -fno-threadsafe-statics -nostdlib --param max-inline-insns-single=500 -fno-rtti -fno-exceptions -MMD $(DEFINES) $(INCLUDES)

OBJS = arduino/main.o arduino/abi.o arduino/hooks.o arduino/IPAddress.o \
	arduino/itoa.o \
	arduino/Print.o arduino/Reset.o arduino/RingBuffer.o arduino/SERCOM.o arduino/startup.o \
	arduino/Stream.o arduino/Uart.o arduino/wiring.o arduino/wiring_analog.o arduino/wiring_digital.o arduino/wiring_private.o \
	arduino/WString.o \
	arduino/avr/dtostrf.o \
	arduino/USB/CDC.o arduino/USB/PluggableUSB.o arduino/USB/USBCore.o \
	ABM_Buttons.o ABM_Buzzer.o ABM_HW_switch.o delay.o log.o \
	PID_control.o ProcessN1.o run_mode.o Sensors.o small_log.o storage_var.o u8g_port.o utlts.o \
	menu_connect_log.o menu_run_mode.o menu_sw_setup.o \
	small_icons.o SmartPID_logo_1bit.o \
	spcServer.o \
	vsd_GUI.o vsd_Menu.o vsd_ValueList.o \
	WiFi.o \
	libraries/Adafruit_GFX_Library/Adafruit_GFX.o \
	libraries/Adafruit_SSD1306/Adafruit_SSD1306.o \
	libraries/Adafruit_SSD1306/ssd1306_128x64_i2c.o \
	libraries/ASF/interrupt_sam_nvic.o \
	libraries/ASF/dma.o \
	libraries/ASF/i2c_master.o \
	libraries/ASF/sercom_asf.o \
	libraries/ASF/clock.o \
	libraries/ASF/gclk.o \
	libraries/ASF/pinmux.o \
	libraries/ASF/samd21/source/gcc/startup_samd21.o \
	libraries/I2C_MASTER_DMA/oled_dma.o libraries/I2C_MASTER_DMA/qs_i2c_master_dma.o \
	libraries/OneButton/OneButton.o \
	libraries/PID/PID_v1.o \
	libraries/RWW_EEPROM/bod.o libraries/RWW_EEPROM/eeprom.o libraries/RWW_EEPROM/nvm.o libraries/RWW_EEPROM/wdt.o \
	libraries/SparkFun_ESP8266/ESP8266UdpServer.o \
	libraries/SparkFun_ESP8266/SparkFunESP8266Client.o \
	libraries/SparkFun_ESP8266/SparkFunESP8266Server.o \
	libraries/SparkFun_ESP8266/SparkFunESP8266WiFi.o \
	libraries/SPI/SPI.o \
	libraries/SpiFlash/SpiFlash.o \
	libraries/ThingSpeak/ThingSpeak.o \
	libraries/U8glib/u8g_bitmap.o libraries/U8glib/u8g_clip.o libraries/U8glib/u8g_font.o libraries/U8glib/u8g_line.o libraries/U8glib/u8g_ll_api.o libraries/U8glib/u8g_page.o libraries/U8glib/u8g_pb.o libraries/U8glib/u8g_rect.o libraries/U8glib/u8g_state.o libraries/U8glib/u8g_font_data.o \
	libraries/Wire/Wire.o \
	variants/AB1000/variant.o

DEPS = ${OBJS:.o=.d}

clean:
	rm -f $(OBJS) $(DEPS)

ArdPort.bin:	$(OBJS)
	$(CC) -Os "-Tvariants/AB1000/linker_scripts/gcc/flash_with_bootloader.ld" --specs=nano.specs --specs=nosys.specs -mcpu=cortex-m0plus -mthumb -Wl,--check-sections -Wl,--gc-sections -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-section-align -Wl,-Map,ArdPort.map -o ArdPort.elf $(OBJS) -lm
	$(OBJCOPY) -O binary ArdPort.elf ArdPort.bin

hwtest.bin:	DEFINES += -DAPP_HWTEST
hwtest.bin:	$(OBJS) hwTest.o
	@$(CC) -Os "-Tvariants/AB1000/linker_scripts/gcc/flash_with_bootloader.ld" --specs=nano.specs --specs=nosys.specs -mcpu=cortex-m0plus -mthumb -Wl,--check-sections -Wl,--gc-sections -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-section-align -Wl,-Map,hwtest.map -o hwtest.elf $(OBJS) hwTest.o -lm
	@$(OBJCOPY) -O binary hwtest.elf hwtest.bin

-include ${DEPS}

# C files to be compiled with g++

arduino/startup.o:
	$(CXX) $(CXXFLAGS) arduino/startup.c -o arduino/startup.o
arduino/wiring.o:
	$(CXX) $(CXXFLAGS) arduino/wiring.c -o arduino/wiring.o
arduino/wiring_analog.o:
	$(CXX) $(CXXFLAGS) arduino/wiring_analog.c -o arduino/wiring_analog.o
arduino/wiring_digital.o:
	$(CXX) $(CXXFLAGS) arduino/wiring_digital.c -o arduino/wiring_digital.o
arduino/wiring_private.o:
	$(CXX) $(CXXFLAGS) arduino/wiring_private.c -o arduino/wiring_private.o
small_icons.o:
	$(CXX) $(CXXFLAGS) small_icons.c -o small_icons.o
SmartPID_logo_1bit.o:
	$(CXX) $(CXXFLAGS) SmartPID_logo_1bit.c -o SmartPID_logo_1bit.o
libraries/ASF/interrupt_sam_nvic.o:
	$(CXX) $(CXXFLAGS) libraries/ASF/interrupt_sam_nvic.c -o libraries/ASF/interrupt_sam_nvic.o
libraries/ASF/dma.o:
	$(CXX) $(CXXFLAGS) libraries/ASF/dma.c -o libraries/ASF/dma.o
libraries/ASF/clock.o:
	$(CXX) $(CXXFLAGS) libraries/ASF/clock.c -o libraries/ASF/clock.o
libraries/ASF/gclk.o:
	$(CXX) $(CXXFLAGS) libraries/ASF/gclk.c -o libraries/ASF/gclk.o
libraries/ASF/pinmux.o:
	$(CXX) $(CXXFLAGS) libraries/ASF/pinmux.c -o libraries/ASF/pinmux.o
libraries/RWW_EEPROM/bod.o:
	$(CXX) $(CXXFLAGS) libraries/RWW_EEPROM/bod.c -o libraries/RWW_EEPROM/bod.o
libraries/RWW_EEPROM/nvm.o:
	$(CXX) $(CXXFLAGS) libraries/RWW_EEPROM/nvm.c -o libraries/RWW_EEPROM/nvm.o
libraries/RWW_EEPROM/wdt.o:
	$(CXX) $(CXXFLAGS) libraries/RWW_EEPROM/wdt.c -o libraries/RWW_EEPROM/wdt.o
libraries/U8glib/u8g_bitmap.o:
	$(CXX) $(CXXFLAGS) libraries/U8glib/u8g_bitmap.c -o libraries/U8glib/u8g_bitmap.o
libraries/U8glib/u8g_clip.o:
	$(CXX) $(CXXFLAGS) libraries/U8glib/u8g_clip.c -o libraries/U8glib/u8g_clip.o
libraries/U8glib/u8g_font.o:
	$(CXX) $(CXXFLAGS) libraries/U8glib/u8g_font.c -o libraries/U8glib/u8g_font.o
libraries/U8glib/u8g_line.o:
	$(CXX) $(CXXFLAGS) libraries/U8glib/u8g_line.c -o libraries/U8glib/u8g_line.o
libraries/U8glib/u8g_ll_api.o:
	$(CXX) $(CXXFLAGS) libraries/U8glib/u8g_ll_api.c -o libraries/U8glib/u8g_ll_api.o
libraries/U8glib/u8g_page.o:
	$(CXX) $(CXXFLAGS) libraries/U8glib/u8g_page.c -o libraries/U8glib/u8g_page.o
libraries/U8glib/u8g_pb.o:
	$(CXX) $(CXXFLAGS) libraries/U8glib/u8g_pb.c -o libraries/U8glib/u8g_pb.o
libraries/U8glib/u8g_rect.o:
	$(CXX) $(CXXFLAGS) libraries/U8glib/u8g_rect.c -o libraries/U8glib/u8g_rect.o
libraries/U8glib/u8g_state.o:
	$(CXX) $(CXXFLAGS) libraries/U8glib/u8g_state.c -o libraries/U8glib/u8g_state.o
libraries/U8glib/u8g_font_data.o:
	$(CXX) $(CXXFLAGS) libraries/U8glib/u8g_font_data.c -o libraries/U8glib/u8g_font_data.o
