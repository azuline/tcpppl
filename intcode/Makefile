.PHONY: clean check lint

all: $(patsubst %.cpp,bin/%,$(wildcard *.cpp))

bin/%: %.cpp
	mkdir -p bin
	g++ -std=c++23 -o $@ $<

clean:
	rm -r bin
check:
	clang-check *.cpp -- -std=c++23
	clang-format --dry-run --fail-on-incomplete-format -Werror *.cpp
lint:
	clang-check --fixit *.cpp -- -std=c++23
	clang-format -i -Werror *.cpp
