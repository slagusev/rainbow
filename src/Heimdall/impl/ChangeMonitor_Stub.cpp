#ifdef USE_HEIMDALL

#include "Heimdall/ChangeMonitor.h"
#if !defined(RAINBOW_OS_MACOS) && !defined(RAINBOW_OS_WINDOWS)

ChangeMonitor::ChangeMonitor(const char *const) { }
ChangeMonitor::~ChangeMonitor() { }

#endif  // !RAINBOW_OS_MACOS && !RAINBOW_OS_WINDOWS
#endif  // USE_HEIMDALL