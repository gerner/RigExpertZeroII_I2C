SHELL=/bin/bash
SOURCES=$(shell find . -name *.cpp -o -name *.h)

all: $(SOURCES)
	arduino-cli compile --fqbn arduino:avr:uno --library ./ examples/BasicMeasure/

install: all
	./create_zip
	arduino-cli lib install --zip-path RigExpertZeroII_I2C.zip
