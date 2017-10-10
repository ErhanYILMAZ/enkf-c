/******************************************************************************
 *
 * File:        reader_navo.c        
 *
 * Created:     12/2012
 *
 * Author:      Pavel Sakov
 *              Bureau of Meteorology
 *
 * Description: Contains 1 reader reader_navo_standard() for preprocessed
 *              data from NAVO.
 *              Parameters:
 *                ADDBIAS -- flag to whether add bias or not (default = no).
 *               
 *
 * Revisions:
 *
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "ncw.h"
#include "definitions.h"
#include "utils.h"
#include "obsmeta.h"
#include "model.h"
#include "grid.h"
#include "observations.h"
#include "prep_utils.h"

#define ADDBIAS_DEF 0

/**
 */
void reader_navo_standard(char* fname, int fid, obsmeta* meta, model* m, observations* obs)
{
    int addbias = ADDBIAS_DEF;
    int ncid;
    int dimid_nobs;
    size_t nobs_local;
    int varid_lon, varid_lat, varid_sst, varid_sstb, varid_error, varid_time;
    double* lon = NULL;
    double* lat = NULL;
    double* sst = NULL;
    double* sstb = NULL;
    double* error_std = NULL;
    double* time = NULL;
    int year, month, day;
    char tunits[MAXSTRLEN];
    size_t tunits_len;
    double tunits_multiple, tunits_offset;
    char* basename;
    int mvid;
    float** depth;
    int ktop;
    int i;

    for (i = 0; i < meta->npars; ++i) {
        if (strcasecmp(meta->pars[i].name, "ADDBIAS") == 0)
            addbias = (istrue(meta->pars[i].value)) ? 1 : 0;
        else
            enkf_quit("unknown PARAMETER \"%s\"\n", meta->pars[i].name);
    }
    enkf_printf("        ADDBIAS = %s\n", (addbias) ? "YES" : "NO");

    basename = strrchr(fname, '/');
    if (basename == NULL)
        basename = fname;
    else
        basename += 1;

    ncw_open(fname, NC_NOWRITE, &ncid);
    ncw_inq_dimid(ncid, (ncw_dim_exists(ncid, "nobs")) ? "nobs" : "length", &dimid_nobs);
    ncw_inq_dimlen(ncid, dimid_nobs, &nobs_local);
    enkf_printf("        nobs = %u\n", (unsigned int) nobs_local);

    if (nobs_local == 0) {
        ncw_close(ncid);
        return;
    }

    ncw_inq_varid(ncid, "lon", &varid_lon);
    lon = malloc(nobs_local * sizeof(double));
    ncw_get_var_double(ncid, varid_lon, lon);

    ncw_inq_varid(ncid, "lat", &varid_lat);
    lat = malloc(nobs_local * sizeof(double));
    ncw_get_var_double(ncid, varid_lat, lat);

    ncw_inq_varid(ncid, "sst", &varid_sst);
    sst = malloc(nobs_local * sizeof(double));
    ncw_get_var_double(ncid, varid_sst, sst);

    if (addbias) {
        ncw_inq_varid(ncid, "SST_bias", &varid_sstb);
        sstb = malloc(nobs_local * sizeof(double));
        ncw_get_var_double(ncid, varid_sstb, sstb);
    }

    ncw_inq_varid(ncid, "error", &varid_error);
    error_std = malloc(nobs_local * sizeof(double));
    ncw_get_var_double(ncid, varid_error, error_std);

    ncw_inq_varid(ncid, "GMT_time", &varid_time);
    time = malloc(nobs_local * sizeof(double));
    ncw_get_var_double(ncid, varid_time, time);
    ncw_inq_attlen(ncid, varid_time, "units", &tunits_len);
    ncw_get_att_text(ncid, varid_time, "units", tunits);
    basename[13] = 0;
    if (!str2int(&basename[11], &day))
        enkf_quit("NAVO reader: could not convert file name \"%s\" to date", fname);
    basename[11] = 0;
    if (!str2int(&basename[9], &month))
        enkf_quit("NAVO reader: could not convert file name \"%s\" to date", fname);
    basename[9] = 0;
    if (!str2int(&basename[5], &year))
        enkf_quit("NAVO reader: could not convert file name \"%s\" to date", fname);
    snprintf(&tunits[tunits_len], MAXSTRLEN - tunits_len, " since %4d-%02d-%02d", year, month, day);

    ncw_close(ncid);

    tunits_convert(tunits, &tunits_multiple, &tunits_offset);

    mvid = model_getvarid(m, obs->obstypes[obstype_getid(obs->nobstypes, obs->obstypes, meta->type, 1)].varnames[0], 1);
    ktop = grid_gettoplayerid(model_getvargrid(m, mvid));
    depth = model_getdepth(m, mvid, 0);

    for (i = 0; i < (int) nobs_local; ++i) {
        observation* o;
        obstype* ot;

        obs_checkalloc(obs);
        o = &obs->data[obs->nobs];

        o->product = st_findindexbystring(obs->products, meta->product);
        assert(o->product >= 0);
        o->type = obstype_getid(obs->nobstypes, obs->obstypes, meta->type, 1);
        ot = &obs->obstypes[o->type];
        o->instrument = st_add_ifabsent(obs->instruments, "AVHRR", -1);
        o->id = obs->nobs;
        o->fid = fid;
        o->batch = 0;
        o->value = (addbias) ? sst[i] + sstb[i] : sst[i];
        o->std = error_std[i];
        o->lon = lon[i];
        o->lat = lat[i];
        o->depth = 0.0;
        o->fk = (double) ktop;
        o->status = model_xy2fij(m, mvid, o->lon, o->lat, &o->fi, &o->fj);
        if (!obs->allobs && o->status == STATUS_OUTSIDEGRID)
            continue;
        if ((o->status == STATUS_OK) && (o->lon <= ot->xmin || o->lon >= ot->xmax || o->lat <= ot->ymin || o->lat >= ot->ymax))
            o->status = STATUS_OUTSIDEOBSDOMAIN;
        o->model_depth = (depth == NULL || isnan(o->fi + o->fj)) ? NAN : depth[(int) (o->fj + 0.5)][(int) (o->fi + 0.5)];
        o->date = time[i] * tunits_multiple + tunits_offset;
        o->aux = -1;

        obs->nobs++;
    }

    free(lon);
    free(lat);
    free(sst);
    if (addbias)
        free(sstb);
    free(error_std);
    free(time);
}
