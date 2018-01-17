/******************************************************************************
 *
 * File:        observations.h        
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

#if !defined(_OBSERVATIONS_H)

#include "stringtable.h"
#include "hash.h"
#include "kdtree.h"
#include "enkfprm.h"
#include "grid.h"
#include "obstypes.h"
#include "model.h"

/*
 * (we keep the integer types signed for compatibility with netcdf 3)
 */
typedef struct {
    /*
     * for primary observations - the ordered number of the (compacted)
     * primary observations;
     * for super observations - the ordered number of superobservations
     */
    int id;
    /*
     * for primary observations - the original ID corresponds to the number
     * of the primary observation during the very first read of data files; for
     * superobs - to the original ID of the very first observation merged into
     * this superob
     */
    int id_orig;
    short int type;
    short int product;
    short int instrument;
    short int fid;
    short int batch;
    double value;
    double std;
    double lon;
    double lat;
    double depth;
    double model_depth;
    double fi;
    double fj;
    double fk;
    double date;
    int status;                 /* 0 = OK */
    /*
     * auxiliary information:
     *  - # of obs for a sob
     *  - sob id for the original ob
     */
    int aux;
} observation;

typedef struct {
    stringtable* products;
    stringtable* instruments;
    stringtable* datafiles;

    int nobstypes;
    obstype* obstypes;
#if defined(ENKF_CALC)
    kdtree** loctrees;
#endif
    int** obsids;

    double da_date;             /* days since 00:00:00
                                 * BASEDAY-BASEMONTH-BASEYEAR */
    char* datestr;

    int allobs;                 /* flag - whether to keep obs outside model
                                 * grid */
    int nallocated;
    int nobs;
    observation* data;
    int compacted;

    int stride;

    int hasstats;
    int ngood;
    int noutside_grid;
    int noutside_obsdomain;
    int noutside_obswindow;
    int nland;
    int nshallow;
    int nbadbatch;
    int nrange;
    int nmodified;

    hashtable* badbatches;

    int ncformat;
    int nccompression;
} observations;

observations* obs_create(void);
observations* obs_create_fromprm(enkfprm* prm);
observations* obs_create_fromdata(observations* parentobs, int nobs, observation data[]);
void obs_destroy(observations* obs);
void obs_checkalloc(observations* obs);
void obs_addtype(observations* obs, obstype* src, obsdomain* domain);
void obs_compact(observations* obs);
void obs_inorder(observations* obs);
void obs_calcstats(observations* obs);
void obs_markbadbatches(observations* obs);
void obs_read(observations* obs, char fname[]);
void obs_write(observations* obs, char fname[]);
void obs_superob(observations* obs, __compar_d_fn_t cmp_obs, observations** sobs, int sobid);
void obs_writeaux(observations* obs, char fname[]);
void obs_find_bytype(observations* obs, int type, int* nobs, int** obsids);
void obs_find_bytypeandtime(observations* obs, int type, int time, int* nobs, int** obsids);
void obs_printob(observations* obs, int id);
void obs_createkdtrees(observations* obs, model* m);
void obs_destroykdtrees(observations* obs);
void obs_findlocal(observations* obs, model* m, grid* g, int i, int j, int* n, int** ids, double** lcoeffs);

#define _OBSERVATIONS_H
#endif
