#pragma once

// ============================
// Global Profiling Switch.
// ============================

#define KAKADU_ENABLE_PROFILING 0

#if KAKADU_ENABLE_PROFILING
#define TRACY_ENABLE
#define TRACY_VERBOSE
#endif
