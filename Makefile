CXXFLAGS += -std=c++17 -Iinclude -Isrc
DEPFLAGS += -MT $@ -MMD -MP -MF out/$*.d
ARFLAGS +=
LDFLAGS += -pthread

SRCS = $(shell find src -name *.cc)
OBJS = $(SRCS:src/%.cc=out/%.o)
DEPS = $(SRCS:src/%.cc=out/%.d)

EXAMPLE_SRCS = $(shell find examples -name *.cc)
EXAMPLE_OBJS = $(EXAMPLE_SRCS:%.cc=out/%.o)
EXAMPLE_DEPS = $(EXAMPLE_SRCS:%.cc=out/%.d)
EXAMPLE_BINS = $(EXAMPLE_SRCS:%.cc=bin/%)
EXAMPLE_CXXFLAGS +=
EXAMPLE_DEPFLAGS += -MT $@ -MMD -MP -MF out/examples/$*.d
EXAMPLE_LDFLAGS += libstdthread.a


.PHONY: all
all: libstdthread.a

-include $(DEPS) $(EXAMPLE_DEPS)

.PHONY: examples
examples: $(EXAMPLE_BINS)


libstdthread.a: $(OBJS)
	mkdir -p $(@D)
	$(AR) $(ARFLAGS) $@ $^

out/%.o: src/%.cc
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c -o $@ $<


out/examples/%.o: examples/%.cc
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(EXAMPLE_CXXFLAGS) $(EXAMPLE_DEPFLAGS) -c -o $@ $<

bin/examples/%: out/examples/%.o libstdthread.a
	mkdir -p $(@D)
	$(CXX) -o $@ $< $(LDFLAGS) $(EXAMPLE_LDFLAGS)


.PHONY: clean
clean:
	rm -rf bin out libstdthread.a
