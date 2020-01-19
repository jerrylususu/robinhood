/* This file is part of the RobinHood Library
 * Copyright (C) 2019 Commissariat a l'energie atomique et aux energies
 *                    alternatives
 *
 * SPDX-License-Identifer: LGPL-3.0-or-later
 *
 * author: Quentin Bouget <quentin.bouget@cea.fr>
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "robinhood/id.h"

int
rbh_id_copy(struct rbh_id *dest, const struct rbh_id *src, char **buffer,
            size_t *bufsize)
{
    size_t size = *bufsize;
    char *data = *buffer;

    /* id->data */
    if (src->size > size) {
        errno = ENOBUFS;
        return -1;
    }

    dest->data = data;
    memcpy(data, src->data, src->size);
    data += src->size;
    size -= src->size;

    /* id->size */
    dest->size = src->size;

    *buffer = data;
    *bufsize = size;
    return 0;
}

static struct rbh_id *
rbh_id_clone(const struct rbh_id *id)
{
    struct rbh_id *clone;
    size_t size;
    char *data;
    int rc;

    size = id->size;
    clone = malloc(sizeof(*clone) + size);
    if (clone == NULL)
        return NULL;
    data = (char *)clone + sizeof(*clone);

    rc = rbh_id_copy(clone, id, &data, &size);
    assert(rc == 0);

    return clone;
}

struct rbh_id *
rbh_id_new(const char *data, size_t size)
{
    const struct rbh_id ID = {
        .data = data,
        .size = size,
    };

    return rbh_id_clone(&ID);
}

/* A struct file_handle has 3 public fields:
 *   - handle_bytes
 *   - handle_type
 *   - f_handle
 *
 * The data of a file handle is mapped in a struct rbh_id as follows:
 *
 * -----------------------------       ------------------------------
 * |     file handle           |       |             ID             |
 * |---------------------------|       |----------------------------|
 * | handle_bytes |          4 |  <=>  | data  | 0x0123456789abcdef |
 * | handle_type  | 0x01234567 |       | size  |                  8 | <--
 * | f_handle     | 0x89abcdef |       ------------------------------   |
 * -----------------------------                                        |
 *                                     sizeof(handle_type) + handle_bytes
 */
struct rbh_id *
rbh_id_from_file_handle(const struct file_handle *handle)
{
    struct rbh_id *id;
    size_t size;
    char *data;

    size = sizeof(handle->handle_type) + handle->handle_bytes;
    id = malloc(sizeof(*id) + size);
    if (id == NULL)
        return NULL;
    data = (char *)id + sizeof(*id);

    /* id->data */
    id->data = data;
    data = mempcpy(data, &handle->handle_type, sizeof(handle->handle_type));
    memcpy(data, handle->f_handle, handle->handle_bytes);

    /* id->size */
    id->size = size;

    return id;
}
