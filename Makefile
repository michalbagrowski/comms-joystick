.PHONY: all install-cli install-core install-libs compile-transmitter compile-receiver compile upload-transmitter upload-receiver clean monitor-transmitter monitor-receiver help

ARDUINO_CLI = arduino-cli
BOARD_FQBN = esp32:esp32:esp32c3
BOARD_NAME = esp32:esp32

TX_SKETCH = transmitter/transmitter.ino
RX_SKETCH = receiver/receiver.ino

TX_PORT ?= $(shell arduino-cli board list | grep "Serial Port (USB)" | head -n1 | awk '{print $$1}')
RX_PORT ?= $(shell arduino-cli board list | grep "Serial Port (USB)" | tail -n1 | awk '{print $$1}')

all: compile upload-transmitter upload-receiver

help:
	@echo "ESP32-C3 Joystick Communication Project"
	@echo "========================================"
	@echo ""
	@echo "Available targets:"
	@echo "  all                - Compile and upload to both boards"
	@echo "  install-cli        - Install Arduino CLI (macOS)"
	@echo "  install-core       - Install ESP32 board support"
	@echo "  install-libs       - Install required libraries"
	@echo "  compile            - Compile both transmitter and receiver"
	@echo "  compile-transmitter- Compile transmitter only"
	@echo "  compile-receiver   - Compile receiver only"
	@echo "  upload-transmitter - Upload transmitter (set TX_PORT=/dev/cu.xxx)"
	@echo "  upload-receiver    - Upload receiver (set RX_PORT=/dev/cu.xxx)"
	@echo "  monitor-transmitter- Serial monitor for transmitter"
	@echo "  monitor-receiver   - Serial monitor for receiver"
	@echo "  clean              - Clean build files"
	@echo "  list-ports         - List available serial ports"
	@echo "  identify           - Identify which physical board is which"
	@echo ""
	@echo "Example usage:"
	@echo "  make install-libs"
	@echo "  make all TX_PORT=/dev/cu.usbserial-1234 RX_PORT=/dev/cu.usbserial-5678"
	@echo "  make compile"
	@echo "  make upload-transmitter TX_PORT=/dev/cu.usbserial-1234"
	@echo "  make upload-receiver RX_PORT=/dev/cu.usbserial-5678"

install-cli:
	@echo "Installing Arduino CLI..."
	@if command -v brew >/dev/null 2>&1; then \
		brew install arduino-cli; \
	else \
		echo "Homebrew not found. Please install from https://arduino.github.io/arduino-cli/"; \
	fi

install-core:
	@echo "Configuring Arduino CLI..."
	$(ARDUINO_CLI) config init --overwrite || true
	@echo "Adding ESP32 board manager URL..."
	$(ARDUINO_CLI) config add board_manager.additional_urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
	@echo "Updating core index..."
	$(ARDUINO_CLI) core update-index
	@echo "Installing ESP32 core..."
	$(ARDUINO_CLI) core install $(BOARD_NAME)

install-libs:
	@echo "Installing required libraries..."
	$(ARDUINO_CLI) lib install "Adafruit GFX Library"
	$(ARDUINO_CLI) lib install "Adafruit SSD1306"
	$(ARDUINO_CLI) lib install "ESP32Servo"
	@echo "Libraries installed successfully!"

compile: compile-transmitter compile-receiver

compile-transmitter:
	@echo "Compiling transmitter..."
	$(ARDUINO_CLI) compile --fqbn $(BOARD_FQBN) $(TX_SKETCH)

compile-receiver:
	@echo "Compiling receiver..."
	$(ARDUINO_CLI) compile --fqbn $(BOARD_FQBN) $(RX_SKETCH)

upload-transmitter:
	@echo "Detected TX port: $(TX_PORT)"
	@echo "Uploading transmitter to $(TX_PORT)..."
	$(ARDUINO_CLI) upload -p $(TX_PORT) --fqbn $(BOARD_FQBN) $(TX_SKETCH)

upload-receiver:
	@echo "Detected RX port: $(RX_PORT)"
	@echo "Uploading receiver to $(RX_PORT)..."
	$(ARDUINO_CLI) upload -p $(RX_PORT) --fqbn $(BOARD_FQBN) $(RX_SKETCH)

monitor-transmitter:
	@echo "Opening serial monitor for transmitter on $(TX_PORT)..."
	$(ARDUINO_CLI) monitor -p $(TX_PORT) -c baudrate=115200

monitor-receiver:
	@echo "Opening serial monitor for receiver on $(RX_PORT)..."
	$(ARDUINO_CLI) monitor -p $(RX_PORT) -c baudrate=115200

list-ports:
	@echo "Available serial ports:"
	@$(ARDUINO_CLI) board list
	@echo ""
	@echo "Auto-detected ports:"
	@echo "  TX_PORT = $(TX_PORT)"
	@echo "  RX_PORT = $(RX_PORT)"

identify:
	@echo "This will help you identify which board is which."
	@echo ""
	@echo "First board ($(TX_PORT)) will blink its LED..."
	@echo "If this is your TRANSMITTER board (the one with joysticks), press Ctrl+C and run 'make all'"
	@echo "If this is your RECEIVER board, swap the USB cables and run 'make all' again"
	@echo ""
	@echo "Press Enter to continue..."
	@read dummy
	@echo "int ledPin = 8; void setup() { pinMode(ledPin, OUTPUT); } void loop() { digitalWrite(ledPin, HIGH); delay(200); digitalWrite(ledPin, LOW); delay(200); }" > /tmp/blink_test.ino
	$(ARDUINO_CLI) upload -p $(TX_PORT) --fqbn $(BOARD_FQBN) /tmp/blink_test.ino 2>/dev/null || true

clean:
	@echo "Cleaning build files..."
	@rm -rf transmitter/build receiver/build
	@echo "Clean complete!"

verify: compile
	@echo "Verification complete - both sketches compiled successfully!"
