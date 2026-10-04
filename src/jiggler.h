#pragma once

// Call once after settings are loaded, and again whenever the intervals or
// the enabled state change, to restart both timers.
void jigglerReset();

// Call from loop(). Sends a mouse nudge / Cmd+Tab when their timers expire.
void jigglerTick();
