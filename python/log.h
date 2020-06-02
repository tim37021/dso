#pragma once

#include <cstdio>

// TODO disable LOG when NDEBUG 
#define LOG(msg) fprintf(stderr, "%s\n", msg);
#define LOG_ERROR(msg) fprintf(stderr, "%s\n", msg);
