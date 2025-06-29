BUILD = build
CMAKE = cmake
MAKE = make
TYPE = Debug
TARGET = compute

all: run

init: 
	@mkdir -p $(BUILD)
	@echo Build type: $(TYPE) 
	@$(CMAKE) -D CMAKE_BUILD_TYPE=$(TYPE) -B $(BUILD) -DGLCORE_EXAMPLES=1

build:
	@cd $(BUILD) && $(MAKE)
.PHONY: build

build-%:
	@cd $(BUILD) && $(MAKE) $*

run: build-$(TARGET)
	@./$(BUILD)/examples/$(TARGET)

clean:
	@cd $(BUILD) && $(MAKE) clean

