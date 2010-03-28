/* ----------------------------------------------------------------
 * auth.c
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
#include "../config.h"

#ifdef MACOSX
#include <pam/pam_appl.h>
#else
#include <security/pam_appl.h>
#endif
#include <openssl/ssl.h>

#include "wsseapi.h"
#include "auth.h"

const char pam_service[] = "besserver";
extern uid_t service_uid;
extern char *generic_user;
extern int isElement(struct soap_dom_element *, char *, char *);

int
changeToUid(uid_t new_uid)
{
    if (seteuid(new_uid)) {
        perror("changeToUid: seteuid");
        return -1;
    }
    return 0;
}

int 
pam_conv_username_token(int num_msg, const struct pam_message **msg,
        struct pam_response **resp, void *appdata_ptr)
{
    struct auth_username_token *userinfo =
            (struct auth_username_token*)appdata_ptr;
    struct pam_response *response;
    int i;

    /* sanity check arguments */
    if (!msg || !resp || !userinfo) {
        return PAM_CONV_ERR;
    }

    response = (struct pam_response*)malloc(num_msg
            * sizeof(struct pam_response));
    if (!response) {
        return PAM_CONV_ERR;
    }
    memset(response, 0, num_msg * sizeof(struct pam_response));

    for (i = 0; i < num_msg; i++) {
        response[i].resp = NULL;
        response[i].resp_retcode = 0;
        
        switch (msg[i]->msg_style) {
        case PAM_PROMPT_ECHO_ON:
            response[i].resp = strdup(userinfo->username);
            break;
        case PAM_PROMPT_ECHO_OFF:
            response[i].resp = strdup(userinfo->password);
            break;
        default:
            if (response) free(response);
            return PAM_CONV_ERR;
        }
    }

    *resp = response;
    return PAM_SUCCESS;
}

int
getUsernameTokenFromDOM(struct soap *s, char **username, char **password)
{
    static char fname[] = "getUsernameTokenFromDOM";
    struct soap_dom_element *token;
    struct soap_dom_attribute *attr;
    char *user, *pass;
    int i, found = 0;

    for (i = 0; i < s->header->__size; i++) {
        token = &(s->header->__any[i]);
        if (isElement(token, WSSE_NS, "Security")) {
            token = token->elts;
            if (!isElement(token, WSSE_NS, "UsernameToken")) {
                return soap_wsse_fault(s, wsse__UnsupportedSecurityToken, NULL);
            }
            if (isElement(token->elts, WSSE_NS, "Username")) {
                user = token->elts->data;
                if (isElement(token->elts->next, WSSE_NS, "Password")) {
                    pass = token->elts->next->data;
                    found = 1;
                }
            }
        }
    }

    if (found) {
        *username = user;
        *password = pass;
        return SOAP_OK;
    }
    else {
        return soap_wsse_fault(s, wsse__FailedAuthentication, NULL);
    }
}

int 
authenticate(struct soap *s, char *username, int namelen, int *sslauth)
{
    static char fname[] = "authenticate";
    struct _wsse__UsernameToken *token;
    struct auth_username_token *userinfo;
    struct pam_conv *conv_info;
    pam_handle_t *pamh = NULL;
    char *user, *password;
    X509 *clientcert;
    int rc;

    clientcert = SSL_get_peer_certificate(s->ssl);
    if (clientcert) {
        if (sslauth) *sslauth = 1;
        if (SSL_get_verify_result(s->ssl) != X509_V_OK) {
            return soap_wsse_fault(s, wsse__FailedAuthentication, NULL);
        }
        /* currently will just map to a generic user */
        X509_free(clientcert);
        
        if (!generic_user) {
            /* no generic_user specified. Have no user to use for operations */
            return soap_receiver_fault(s, SYS_ERROR, NULL);
        }

        if (username && namelen) {
            strncpy(username, generic_user, namelen-1);
            username[namelen-1] = 0;
        }
        
        return SOAP_OK;
    }
    if (sslauth) *sslauth = 0;
 
    /* no X.509 certificate ... look for WS-Security tokens */
    token = soap_wsse_UsernameToken(s, NULL);
    if (token) {
        if (!token->Username || !token->Password) {
            return soap_wsse_fault(s, wsse__FailedAuthentication, NULL);
        }
        if (token->Password->Type && strcmp(token->Password->Type,
                wsse_PasswordTextURI)) {
            return soap_wsse_fault(s, wsse__UnsupportedSecurityToken, NULL);
        }
        user = token->Username;
        password = token->Password->__item;
    }
    else {
        /* see if the security header got stuck in the header __any */
        if (rc = getUsernameTokenFromDOM(s, &user, &password)) {
            return rc;
        } 
    }

    userinfo = (struct auth_username_token*)soap_malloc(s,
            sizeof(struct auth_username_token));
    conv_info = (struct pam_conv*)soap_malloc(s, sizeof(struct pam_conv));
    if (!userinfo || !conv_info) {
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }
    userinfo->s = 0;
    userinfo->username = soap_strdup(s, user);
    userinfo->password = soap_strdup(s, password);
    if (!userinfo->username || !userinfo->password) {
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }
    conv_info->conv = pam_conv_username_token;
    conv_info->appdata_ptr = (void*)userinfo;
    
    if (changeToUid(0)) {
        return soap_receiver_fault(s, SYS_ERROR, NULL);
    }
    rc = pam_start(pam_service, userinfo->username, conv_info, &pamh);
    if (rc != PAM_SUCCESS) {
        return soap_wsse_fault(s, wsse__FailedAuthentication, NULL);
    }

    rc = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
    if (rc != PAM_SUCCESS ) {
        fprintf(stderr, "Couldn't authenticate user %s: %s\n", userinfo->username, pam_strerror(pamh, rc));
    }

    pam_end(pamh, rc);

    if (changeToUid(service_uid)) {
        return soap_receiver_fault(s, SYS_ERROR, NULL);
    }
    
    if (rc != PAM_SUCCESS) {
        return soap_wsse_fault(s, wsse__FailedAuthentication, NULL);
    }

    if (username && namelen) {
        strncpy(username, userinfo->username, namelen-1);
        username[namelen-1] = 0;
    }
    
    return SOAP_OK;
}

