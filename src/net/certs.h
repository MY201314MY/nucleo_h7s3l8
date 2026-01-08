/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __NET_CERTS_H__
#define __NET_CERTS_H__

static const unsigned char ca_certificate[] = {
    #include "digicert.pem"
};

#endif /* __NET_CERTS_H__ */