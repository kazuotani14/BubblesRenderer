#pragma once

// Disable compiler warnings for this third-party code
// TODO make this portable https://www.fluentcpp.com/2019/08/30/how-to-disable-a-warning-in-cpp/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#pragma GCC diagnostic pop
