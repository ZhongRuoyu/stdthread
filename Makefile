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


TEST_SRCS = $(shell find test -name *.cc)
TEST_OBJS = $(TEST_SRCS:%.cc=out/%.o)
TEST_DEPS = $(TEST_SRCS:%.cc=out/%.d)
TEST_BINS = $(TEST_SRCS:%.cc=bin/%)
TEST_CXXFLAGS += -g
TEST_DEPFLAGS += -MT $@ -MMD -MP -MF out/test/$*.d
TEST_LDFLAGS += libstdthread.a


.PHONY: all
all: libstdthread.a

-include $(DEPS) $(EXAMPLE_DEPS) $(TEST_DEPS)

.PHONY: examples
examples: $(EXAMPLE_BINS)

.PHONY: test
test: $(TEST_BINS)
	@set -e; \
		test -t 1 && red="\033[31m" || red=""; \
		test -t 1 && green="\033[32m" || green=""; \
		test -t 1 && yellow="\033[33m" || yellow=""; \
		test -t 1 && reset="\033[0m" || reset=""; \
		for test in $$(printf "%s\n" $^); do \
			echo "[ $${yellow}TESTING$${reset} ] $$test"; \
			( \
				$$test && \
				echo "[ $${green}PASSED$${reset}  ] $$test" \
			) || ( \
				echo "[ $${red}FAILED$${reset}  ] $$test" && \
				exit 1 \
			); \
		done
	@echo "All tests passed."


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


out/test/%.o: test/%.cc
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(TEST_CXXFLAGS) $(TEST_DEPFLAGS) -c -o $@ $<

bin/test/%: out/test/%.o libstdthread.a
	mkdir -p $(@D)
	$(CXX) -o $@ $< $(LDFLAGS) $(TEST_LDFLAGS)


.PHONY: clean
clean:
	rm -rf bin out libstdthread.a
