CC = g++
INCLUDE_PATH = -I./src
CFLAGS = -std=c++17 -O2 -Wall -Wextra -pedantic
# -Werror -DNDEBUG

STATIC_RENDER = render_to_ppm
FLUIDS_RENDER = fluids_sim
ALL_TARGETS = $(STATIC_RENDER) $(FLUIDS_RENDER)
TESTS = hittable_tests

default: $(ALL_TARGETS)
tests: $(TESTS)
all: $(ALL_TARGETS) $(TESTS)
clean:
	rm -f $(ALL_TARGETS) $(TESTS)

# Note to self: use -B to force rebuild
run_static_render:
	make $(STATIC_RENDER) -B
	./render_to_ppm > image.ppm && open image.ppm

$(STATIC_RENDER): examples/$(STATIC_RENDER).cpp
	$(CC) $(INCLUDE_PATH) $(CFLAGS) -o $(STATIC_RENDER) examples/$(STATIC_RENDER).cpp
$(FLUIDS_RENDER): examples/$(FLUIDS_RENDER).cpp
	$(CC) $(INCLUDE_PATH) $(CFLAGS) -o $(FLUIDS_RENDER) examples/$(FLUIDS_RENDER).cpp

hittable_tests: tests/hittable_tests.cpp
	$(CC) $(INCLUDE_PATH) $(CFLAGS) -o hittable_tests tests/hittable_tests.cpp


