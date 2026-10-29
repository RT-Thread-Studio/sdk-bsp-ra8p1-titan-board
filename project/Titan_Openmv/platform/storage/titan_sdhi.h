/* SPDX-License-Identifier: Apache-2.0 */
#ifndef TITAN_OPENMV_SDHI_H
#define TITAN_OPENMV_SDHI_H

/* These queries read cached metadata and the card-detect pad only. They do
 * not acquire a filesystem/host lock or send commands, so USB may use them.
 * After an enumerated card is removed, all access is refused until reboot:
 * live FatFs/MSC handles must never outlive the MMCSD card/device objects. */
int titan_sdhi_present(void);
int titan_sdhi_is_ready(void);
int titan_sdhi_write_protected(void);

/* Start at most one asynchronous MMCSD enumeration. The storage task must
 * consume mmcsd_wait_cd_changed() before reporting completion here; a timeout
 * alone is not a failed probe and must not start a second one. */
int titan_sdhi_probe(void);
void titan_sdhi_probe_complete(int success);

/* Worker-context CMD13 barrier: wait for programming to finish before
 * acknowledging a host WRITE10 or safe eject. */
int titan_sdhi_sync(void);

#endif
