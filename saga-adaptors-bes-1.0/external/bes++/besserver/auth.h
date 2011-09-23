/* ----------------------------------------------------------------
 * auth.h
 *
 * Copyright (C) 2006-2009, Platform Computing Corporation. All Rights Reserved.
 *
 *
 * This file is part of BESserver.
 *
 * BESserver is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#ifndef AUTH_H_
#define AUTH_H_

#include "soapH.h"
#include "namespaces.h"
#include "faults.h"

struct auth_username_token {
    struct soap *s;
    char *username;
    char *password;
};

int authenticate(struct soap*, char *, int, int*);

#endif /*AUTH_H_*/
