/* This file is part of the RobinHood Library
 * Copyright (C) 2019 Commissariat a l'energie atomique et aux energies
 * 		      alternatives
 *
 * SPDX-License-Identifer: LGPL-3.0-or-later
 *
 * author: Quentin Bouget <quentin.bouget@cea.fr>
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>

#include "robinhood/fsentry.h"
#ifndef HAVE_STATX
# include "robinhood/statx.h"
#endif

struct rbh_fsentry *
rbh_fsentry_new(const struct rbh_id *id, const struct rbh_id *parent_id,
                const char *name, const struct statx *statx)
{
    struct rbh_fsentry *fsentry;
    size_t name_length = 0;
    size_t data_size;
    char *data;

    if (name)
        name_length = strlen(name) + 1;

    data_size = name_length;
    data_size += id ? id->size : 0;
    data_size += parent_id ? parent_id->size : 0;
    data_size += statx ? sizeof(*statx) : 0;

    fsentry = calloc(1, sizeof(*fsentry) + data_size);
    if (fsentry == NULL)
        return NULL;
    data = (char *)fsentry + sizeof(*fsentry);

    /* fsentry->id */
    if (id) {
        memcpy(data, id->data, id->size);
        fsentry->id.data = data;
        data += id->size;
        fsentry->id.size = id->size;
        fsentry->mask |= RBH_FP_ID;
    };

    /* fsentry->parent_id */
    if (parent_id) {
        memcpy(data, parent_id->data, parent_id->size);
        fsentry->parent_id.data = data;
        data += parent_id->size;
        fsentry->parent_id.size = parent_id->size;
        fsentry->mask |= RBH_FP_PARENT_ID;
    }

    /* fsentry->name */
    if (name) {
        memcpy(data, name, name_length);
        fsentry->name = data;
        data += strlen(name) + 1;
        fsentry->mask |= RBH_FP_NAME;
    }

    /* fsentry->statx */
    if (statx) {
        memcpy(data, statx, sizeof(*statx));
        fsentry->statx = (struct statx *)data;
        data += sizeof(*statx);
        fsentry->mask |= RBH_FP_STATX;
    }

    return fsentry;
}