CXXFLAGS += -std=c++17 -Iinclude -Isrc
LDFLAGS += -pthread
ARFLAGS +=

SRCS = $(shell find src -name *.cc)
OBJS = $(SRCS:src/%.cc=out/%.o)

EXAMPLE_SRCS = $(shell find examples -name *.cc)
EXAMPLE_OBJS = $(EXAMPLE_SRCS:%.cc=out/%.o)
EXAMPLE_BINS = $(EXAMPLE_SRCS:%.cc=bin/%)
EXAMPLE_CXXFLAGS +=
EXAMPLE_LDFLAGS +=


.PHONY: all
all: libstdthread.a

.PHONY: examples
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
