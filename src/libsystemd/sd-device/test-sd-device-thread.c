/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "sd-device.h"

#include "device-util.h"

#define handle_error_errno(error, msg)                          \
        ({                                                      \
                errno = abs(error);                             \
                perror(msg);                                    \
                EXIT_FAILURE;                                   \
        })

static void* thread(void *p) {
        sd_device **d = p;

        *d = sd_device_unref(*d);

        return NULL;
}

int main(int argc, char *argv[]) {
        sd_device *loopback;
        pthread_t t;
        const char *key, *value;
        int r;

        unsetenv("SYSTEMD_MEMPOOL");

        r = sd_device_new_from_syspath(&loopback, "/sys/class/net/lo");
        if (r < 0)
                return handle_error_errno(r, "Failed to create loopback device object");

        FOREACH_DEVICE_PROPERTY(loopback, key, value)
                printf("%s=%s\n", key, value);

        r = pthread_create(&t, NULL, thread, &loopback);
        if (r != 0)
                return handle_error_errno(r, "Failed to create thread");

        r = pthread_join(t, NULL);
        if (r != 0)
                return handle_error_errno(r, "Failed to wait thread finished");

        if (loopback)
                return handle_error_errno(r, "loopback device is not unref()ed");

        return 0;
}
