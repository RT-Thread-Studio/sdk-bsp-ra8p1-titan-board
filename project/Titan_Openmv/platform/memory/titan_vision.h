/* SPDX-License-Identifier: Apache-2.0 */
#ifndef TITAN_OPENMV_VISION_H
#define TITAN_OPENMV_VISION_H
#include <stdbool.h>

/* The background service is independent of MicroPython soft resets. Vision
 * callers must check readiness before using the externally linked image. */
int titan_vision_start(void);
bool titan_vision_is_ready(void);
bool titan_vision_needs_service(void);
/* Used only by the service; verification never publishes a partial image. */
int titan_vision_load(void);

#endif
