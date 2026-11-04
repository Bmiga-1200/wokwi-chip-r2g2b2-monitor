SOURCES = src/dvia-monitor.chip.c
TARGET  = dist/dvia-monitor.chip.wasm

# ----------------------------------------------------------------
# Pattern rule to build .o files from .c files
# ----------------------------------------------------------------
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: all
all: $(TARGET) dist/dvia-monitor.chip.json

.PHONY: clean
clean:
		rm -rf dist
# ----------------------------------------------------------------
# Ensure the output directory exists
# ----------------------------------------------------------------
dist:
		mkdir -p dist

# ----------------------------------------------------------------
# Link step – builds the final wasm module
# ----------------------------------------------------------------
$(TARGET): dist $(SOURCES) src/wokwi-api.h
	  /opt/wasi-sdk/bin/clang \
	  	--target=wasm32-unknown-wasi \
		--sysroot /opt/wasi-sdk/share/wasi-sysroot \
		-nostartfiles \
		-Wl,--import-memory \
		-Wl,--export-table \
		-Wl,--no-entry \
		-Werror \
		-o $(TARGET) $(SOURCES)

# ----------------------------------------------------------------
# Copy the JSON description alongside the wasm file
# ----------------------------------------------------------------
dist/dvia-monitor.chip.json: dist dvia-monitor.chip.json
	  cp dvia-monitor.chip.json dist

.PHONY: test
test:
	  cd tests && \
      arduino-cli core install arduino:avr && \
	  arduino-cli lib install VGAXUA && \
      arduino-cli compile --export-binaries --fqbn arduino:avr:mega --jobs 0 --optimize-for-debug Pattern

slow_scan:
	  cd tests && \
      arduino-cli core install arduino:avr && \
      arduino-cli compile --export-binaries --fqbn arduino:avr:mega --jobs 0 --optimize-for-debug slow_scan