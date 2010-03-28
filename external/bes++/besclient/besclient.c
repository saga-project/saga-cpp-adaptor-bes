/* ----------------------------------------------------------------
 * besclient.c
 *   
 *      Client implementation of the OGSA Basic Execution Services
 *
 * Copyright (C) 2006-2009, Platform Computing Corporation. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "bes.h"

char *readPassword(char *);
void zeroPassword(char *);
void usage();

void usage(void)
{
    fprintf(stderr, "Usage: besclient -u <username> [-p <password>]");
    fprintf(stderr, " [-e <endpoint>] [-c <capath>] <command> [<command args>]\n");
    fprintf(stderr, "       besclient -x <certificate> [-k <keypassphrase>]");
    fprintf(stderr, " [-e <endpoint>] [-c <capath>] <command> [<command args>]\n");
    exit(1);
}

char *bes_activity_state[BES_State_Num] = 
    {
        "Pending", 
        "Running", 
        "Cancelled", 
        "Failed", 
        "Finished"
    };

int
main(int argc, char *argv[])
{
    struct bes_context *ctx;
    char *endpoint, *command, *user, *pass;
    char *capath, *x509cert, *x509pass;
    int ch, ret_code = 0;
    epr_t endpointepr = NULL, activityepr = NULL;
    
    endpoint = "endpoint.xml";
    capath = "./certs";
    x509cert = NULL;
    x509pass = NULL;
    user = NULL;
    pass = NULL;
    
    while ((ch = getopt(argc, argv, "Vc:e:u:p:x:k:")) != -1) {
        switch (ch) {
        case 'c':
            capath = optarg;
            break;
        case 'e':
            endpoint = optarg;
            break;
        case 'u':
            user = optarg;
            break;
        case 'p':
            pass = optarg;
            break;
        case 'x':
            x509cert = optarg;
            break;
        case 'k':
            x509pass = optarg;
            break;
        case 'V':
            printf("besclient, %s\n%s\n", VERSION_STRING, COPYRIGHT);
            exit(0);
            break;
        case '?':
        default:
            usage();
        }
    }
    
    if ( (user && x509cert) || (!user && !x509cert) ) {
        fprintf(stderr, "You must specify only one of username or certificate file.\n");
        usage();
    }
    
    argc -= optind;
    argv += optind;
    
    if (argc < 1) {
        usage();
    }
  
    command = argv[0];

    if (bes_init(&ctx)) {
        ret_code = 1;
        goto end;
    }

    if (user) {
        if (pass == NULL ) {
            pass = readPassword("WS-Security password:");
        }
    }
    else {
        if (x509pass == NULL) {
            x509pass = readPassword("Key passphrase:");
        }
    }

    if (bes_security(ctx, x509cert, x509pass, capath, user, pass)) {
        fprintf(stderr, "%s\n", bes_get_lasterror(ctx));
        ret_code = 1;
        goto end;
    }

    if (bes_readEPRFromFile(ctx, endpoint, &endpointepr)) {
        fprintf(stderr, "%s\n", bes_get_lasterror(ctx));
        ret_code = 1;
        goto end;
    }

    if (!strcmp(command, "status")
        || !strcmp(command, "terminate")
        || !strcmp(command, "activity")
        || !strcmp(command, "poll")) {
        if (bes_readEPRFromFile(ctx, argv[1], &activityepr)) {
            fprintf(stderr, "%s\n", bes_get_lasterror(ctx));
            ret_code = 1;
            goto end;
        }
    }

    if (!strcmp(command, "create")) {
        epr_t new_activityepr;
    
        if (bes_createActivityFromFile(ctx, endpointepr, argv[1], &new_activityepr)) {
            fprintf(stderr, "%s\n", bes_get_lasterror(ctx));
            ret_code = 1;
        }
        else {
            if (bes_writeEPRToFile(ctx, argv[2], new_activityepr)) {
                fprintf(stderr, "%s\n", bes_get_lasterror(ctx));
                ret_code = 1;
            }
            printf("Successfully submitted activity. Wrote EPR to %s\n", argv[2]);
      
            bes_freeEPR(&new_activityepr);
        }
    }
    else if (!strcmp(command, "status")) {
        struct bes_activity_status status;

        if (bes_getActivityStatuses(ctx, endpointepr, activityepr, &status)) {
            fprintf(stderr, "%s\n", bes_get_lasterror(ctx));
            ret_code = 1;
        }
        else {
            printf("The state of the activity is: %s\n", bes_activity_state[status.state]);
            if (status.substate) {
                printf("The substate is %s\n", status.substate);
                free(status.substate);
            }
        }
      
    }
    else if (!strcmp(command, "terminate")) {
        if (bes_terminateActivities(ctx, endpointepr, activityepr)) {
            printf("The activity was canceled\n");
        }
        else {
            printf("The activity was not canceled\n");
            fprintf(stderr, "%s\n", bes_get_lasterror(ctx));
            ret_code = 1;
        }
    }
    else if (!strcmp(command, "activity")) {
        char *strDoc;
    
        if (bes_getActivityDocumentsXML(ctx, endpointepr, activityepr, &strDoc)) {
            fprintf(stderr, "%s\n", bes_get_lasterror(ctx));
            ret_code = 1;
        }
        else {
            printf("Got result from server:\n");
            printf("%s\n", strDoc);
            free(strDoc);
        }
    }
    else if (!strcmp(command, "factory")) {
        char *strDoc;

        if (bes_getFactoryAttributesDocument(ctx, endpointepr, &strDoc)) {
            fprintf(stderr, "%s\n", bes_get_lasterror(ctx));
            ret_code = 1;
        }
        else {
            printf("Got result from server:\n");
            printf("%s\n", strDoc);
            free(strDoc);
        }
    }
    else if (!strcmp(command, "poll")) {
        struct bes_activity_status status;
        int poll_interval, ret = 0;

        if (argc != 3) {
            fprintf(stderr, "Wrong arguments for poll command.\n", command);
            goto end;
        }

        poll_interval = atoi(argv[2]);
        while (!(ret = bes_getActivityStatuses(ctx, endpointepr, activityepr, &status))) {
            if ((status.state == BES_Cancelled) 
                || (status.state == BES_Failed)
                || (status.state == BES_Finished)) {
                break;
            }
            else {
                printf(".");
                fflush(stdout);
                sleep(poll_interval);
        
                if (user) {
                    if (bes_add_usertoken(ctx, user, pass)) {
                        fprintf(stderr, "%s\n", bes_get_lasterror(ctx));
                        ret_code = 1;
                        break;
                    }
                }
            }
        }

        if (ret) {
            fprintf(stderr, "%s\n", bes_get_lasterror(ctx));
            ret_code = 1;
        }
        else {
            printf("Activity has finished. Final status is: %s\n", bes_activity_state[status.state]);
            if (status.substate) {
                printf("The substate is %s\n", status.substate);
                free(status.substate);
            }
        }
    }
    else {
        fprintf(stderr, "Command %s is unknown.\n", command);
        exit(1);
    }

    if (endpointepr) {
        bes_freeEPR(&endpointepr);
    }

    if (activityepr) {
        bes_freeEPR(&activityepr);
    }

 end:
    bes_finalize(&ctx);
  
    return ret_code;
}

void
zeroPassword(char *password)
{
    if (password) {
        memset(password, 0, strlen(password) * sizeof(char));
    }
}

char*
readPassword(char *message)
{
    char *pw, *password;
    
    if (isatty(fileno(stdin))) {
        pw = getpass(message);
    }
    else {
        fprintf(stderr, "Can only prompt for password if running from a terminal.\n");
        fprintf(stderr, "Use the -p or -k options to supply the password on the command-line.\n");
        exit(1);
    }
    
    if (pw[strlen(pw)-1] == '\n') {
        pw[strlen(pw)-1] = 0;
    }
    
    password = (char*)malloc(strlen(pw) * sizeof(char));
    if (password == NULL) {
        perror("malloc password");
        exit(1);
    }
    memset(password, 0, strlen(pw) * sizeof(char));
    strcpy(password, pw);
    memset(pw, 0, strlen(pw) * sizeof(char));
    
    return password;
}

