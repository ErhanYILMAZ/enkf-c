/******************************************************************************
 *
 * File:        gxy_curv.h
 *
 * Created:     27/11/2019
 *
 * Author:      Pavel Sakov
 *              Bureau of Meteorology
 *
 * Description: The header file for gxy_curv.c.
 *
 * Revisions:   
 *
 *****************************************************************************/

#if !defined(_GXY_CURV_H)
struct gxy_curv;
typedef struct gxy_curv gxy_curv;

gxy_curv* gxy_curv_create(int ni, int nj, double** x, double** y, int** mask);
void gxy_curv_destroy(gxy_curv* gxy);
double** gxy_curv_getx(gxy_curv* gxy);
double** gxy_curv_gety(gxy_curv* gxy);
int gxy_curv_getni(gxy_curv* gxy);
int gxy_curv_getnj(gxy_curv* gxy);
int gxy_curv_xy2fij(gxy_curv* gxy, double x, double y, double* fi, double* fj);
int gxy_curv_fij2xy(gxy_curv* gxy, double fi, double fj, double* x, double* y);

#define _GXY_CURV_H
#endif