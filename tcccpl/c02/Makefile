.PHONY: clean

all: $(patsubst %.cpp,bin/%,$(wildcard *.cpp))

bin/%: %.cpp
	mkdir -p bin
	g++ -o $@ $<

clean:
	rm -r bin
check:
	clang-check *.cpp
lint:
	clang-check --fixit *.cpp
	clang-format -i -Werror *.cpp
