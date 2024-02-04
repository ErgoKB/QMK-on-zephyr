.PHONY: build flash

build:
	west build -p auto -b nrf52840dongle_nrf52840

flash: build
	west flash -r openocd --config /opt/homebrew/share/openocd/scripts/interface/cmsis-dap.cfg
