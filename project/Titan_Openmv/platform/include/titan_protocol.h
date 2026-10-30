/* SPDX-License-Identifier: Apache-2.0 */
#ifndef TITAN_PROTOCOL_H
#define TITAN_PROTOCOL_H
/* Register official channels with the port-owned STDIN lifecycle hooks. */
int titan_protocol_init_default(void);
/* Called between gc_collect_start() and gc_collect_end() in the VM. */
void titan_protocol_gc_collect(void);
#endif
