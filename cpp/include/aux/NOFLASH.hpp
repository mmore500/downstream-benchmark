#pragma once

#if __has_include("pico/platform/sections.h")
#include "pico/platform/sections.h"
#define NOFLASH __not_in_flash("time_critical")
#else
#define NOFLASH
#endif
