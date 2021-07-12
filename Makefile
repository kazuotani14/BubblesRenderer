CC = g++
INCLUDE_PATH = -I./src
CFLAGS = -std=c++17 -O2 -Wall
# -Werror -DNDEBUG

STATIC_RENDER = render_to_ppm
FLUIDS_RENDER = fluids_sim
ALL_TARGETS = $(STATIC_RENDER) $(FLUIDS_RENDER)

default: $(ALL_TARGETS)

$(STATIC_RENDER): examples/$(STATIC_RENDER).cpp
	$(CC) $(INCLUDE_PATH) $(CFLAGS) -o $(STATIC_RENDER) examples/$(STATIC_RENDER).cpp
$(FLUIDS_RENDER): examples/$(FLUIDS_RENDER).cpp
	$(CC) $(INCLUDE_PATH) $(CFLAGS) -o $(FLUIDS_RENDER) examples/$(FLUIDS_RENDER).cpp

clean:
	rm -f $(ALL_TARGETS)

