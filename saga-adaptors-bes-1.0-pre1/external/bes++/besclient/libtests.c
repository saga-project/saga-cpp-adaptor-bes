#include <stdlib.h>
#include <stdio.h>

#include "bes.h"

int
main(int argc, char *argv[])
{
    struct jsdl_job_definition *jd;
    struct jsdl_hpcp_application *app;
    struct jsdl_range_value *cpucount;
    struct bes_context *ctx;
    struct bes_activity_status status;
    struct bes_activity_document *document;
    epr_t endpoint, activity;
    char *user = "merzky";
    char *pass = "aaa";
    char *capath = "../besserver/cert";
    char *endpoint_file = "local.endpoint.xml";
    char *args[] = { "1" };
    int rc, interval = 1;

    if (rc = bes_init(&ctx)) {
        return rc;
    }

    if (rc = bes_security(ctx, NULL, NULL, capath, user, pass)) {
        fprintf(stderr, "%s\n", bes_get_lasterror(ctx));
        return rc;
    }

    if (rc = bes_readEPRFromFile(ctx, endpoint_file, &endpoint)) {
        fprintf(stderr, "%s\n", bes_get_lasterror(ctx));
        return rc;
    }

    if (rc = jsdl_newRangeValue(&cpucount)) {
        fprintf(stderr, "Can't allocate RangeValue\n");
        return rc;
    }
    if (rc = jsdl_addExact(cpucount, 1.0, 0.0)) {
        fprintf(stderr, "Can't add Exact to RangeValue\n");
        return rc;
    }

    if (rc = jsdl_newJobDefinition(JSDL_HPC_PROFILE_APPLICATION, &jd)) {
        fprintf(stderr, "Can't allocate JobDefinition\n");
        return rc;
    }

    jd->JobName = "BESLibTest";
    jd->JobProject = "BES";
    jd->TotalCPUCount = cpucount;
    app = (struct jsdl_hpcp_application*)jd->Application;
    app->Executable = "/bin/sleep";
    app->num_args = 1;
    app->Argument = args;
    app->Output = "/dev/null";
    app->WorkingDirectory = "/tmp";

    if (rc = bes_createActivity(ctx, endpoint, jd, &activity)) {
        fprintf(stderr, "createActivity: %s\n", bes_get_lasterror(ctx));
        return rc;
    }
                
    while (1) {

        if (rc = bes_add_usertoken(ctx, user, pass)) {
            fprintf(stderr, "add_usertoken: %s\n", bes_get_lasterror(ctx));
            return rc;
        }

        if (rc = bes_getActivityStatuses(ctx, endpoint, activity, &status)) {
            fprintf(stderr, "getActivityStatuses: %s\n", bes_get_lasterror(ctx));
            return rc;
        }
            
        if (status.state == BES_Cancelled
            || status.state == BES_Failed
            || status.state == BES_Finished) {
            break;
        }
        
        printf(".");
        fflush(stdout);
        sleep(interval);
    }
    
    printf("\n");
    
    if (rc = bes_add_usertoken(ctx, user, pass)) {
        fprintf(stderr, "add_usertoken: %s\n", bes_get_lasterror(ctx));
        return rc;
    }
    if (rc = bes_getActivityDocuments(ctx, endpoint, activity, &document)) {
        fprintf(stderr, "%s\n", bes_get_lasterror(ctx));
        return rc;
    }
    bes_freeActivityDocument(document);
    
    bes_finalize(&ctx);

    return rc;
}
