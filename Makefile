CXXFLAGS += -std=c++17 -Iinclude -Isrc
LDFLAGS += -pthread
ARFLAGS +=

EXAMPLE_CXXFLAGS += -fsanitize=address
EXAMPLE_LDFLAGS += -fsanitize=address

SRCS = $(wildcard src/*.cc)
OBJS = $(SRCS:src/%.cc=out/%.o)

EXAMPLE_SRCS = $(wildcard examples/*.cc)
EXAMPLE_OBJS = $(EXAMPLE_SRCS:examples/%.cc=out/examples/%.o)
EXAMPLE_BINS = $(EXAMPLE_SRCS:examples/%.cc=bin/examples/%)


all: libstdthread.a

examples: $(EXAMPLE_BINS)


libstdthread.a: $(OBJS)
	mkdir -p $(@D)
	$(AR) $(ARFLAGS) $@ $^

out/%.o: src/%.cc
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $^

out/examples/%.o: examples/%.cc
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(EXAMPLE_CXXFLAGS) -c -o $@ $^

bin/examples/%: out/examples/%.o libstdthread.a
	mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $(EXAMPLE_LDFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -rf bin out libstdthread.a
