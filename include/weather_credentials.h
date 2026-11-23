#pragma once

#if __has_include("duduclock_secrets.h")
#include "duduclock_secrets.h"
#endif

#ifndef DUDUCLOCK_QWEATHER_API_KEY
#define DUDUCLOCK_QWEATHER_API_KEY ""
#endif

constexpr char weatherApiKey[] = DUDUCLOCK_QWEATHER_API_KEY;

inline bool weatherConfigured() {
  return weatherApiKey[0] != '\0';
}
