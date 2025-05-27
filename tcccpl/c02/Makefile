.PHONY: clean

all: $(patsubst %.cpp,%.bin,$(wildcard *.cpp))

%.bin: %.cpp
	g++ -o $@ $<

clean:
	rm *.bin
