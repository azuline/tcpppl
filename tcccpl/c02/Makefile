.PHONY: clean check lint

all: $(patsubst %.cpp,bin/%,$(wildcard *.cpp))

bin/%: %.cpp
	mkdir -p bin
	g++ -o $@ $<

clean:
	rm -r bin
check:
	clang-check *.cpp
	clang-format --dry-run --fail-on-incomplete-format -Werror *.cpp
lint:
	clang-check --fixit *.cpp
	clang-format -i -Werror *.cpp
