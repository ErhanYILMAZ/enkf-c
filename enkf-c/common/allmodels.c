/******************************************************************************
 *
 * File:        allmodels.c        
 *
 * Created:     12/2012
 *
 * Author:      Pavel Sakov
 *              Bureau of Meteorology
 *
 * Description:
 *
 * Revisions:
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "definitions.h"
#include "utils.h"
#include "allmodels.h"
#include "mom4.h"

typedef struct {
    char* modeltype;
    modelsetup_fn setupfn;
} model_entry;

model_entry allmodelentries[] = {
    {"MOM4", mom4_setup}
};

/**
 */
static void describe_modelentries(void)
{
    int nmodelentries = sizeof(allmodelentries) / sizeof(model_entry);
    int i;

    enkf_printf("  Available models:\n");
    enkf_printf("    tag\n");
    enkf_printf("    ---\n");
    for (i = 0; i < nmodelentries; ++i)
        enkf_printf("    %s\n", allmodelentries[i].modeltype);
}

/**
 */
modelsetup_fn get_modelsetupfn(char modeltype[])
{
    int nmodelentries = sizeof(allmodelentries) / sizeof(model_entry);
    int i;

    for (i = 0; i < nmodelentries; ++i)
        if (strcmp(allmodelentries[i].modeltype, modeltype) == 0)
            return allmodelentries[i].setupfn;

    enkf_printf("\n\n  ERROR: no model \"%s\"\n\n", modeltype);
    describe_modelentries();
    enkf_quit("bailing out");

    return NULL;
}
