/**
 * \file coorcell_pmmg.c
 * \brief functions to manage cells of coordinates and index
 * \author Cécile Dobrzynski (Bx INP/Inria)
 * \author Algiane Froehly (Inria)
 * \version 5
 * \copyright GNU Lesser General Public License.
 */

#include "coorcell_pmmg.h"

/**
 * \param a  pointer toward a PMMG_coorCell structure.
 * \param b  pointer toward a PMMG_coorCell structure.
 *
 * \return 1 if a is greater than b, -1 if b is greater than 1, 0 if they are
 * equals.
 *
 * Compare 2 coor cells (can be used inside the qsort C fnuction), first on
 * their x-coordinates, second ond their y-coordiantes then on their z-coordinates.
 *
 */
int PMMG_compare_coorCell (const void * a, const void * b) {
  PMMG_coorCell *cell1,*cell2;
  double        dist[3];
  int           k;

  cell1 = (PMMG_coorCell*)a;
  cell2 = (PMMG_coorCell*)b;

  for ( k=0; k<3; ++k ) {
    dist[k] = cell1->coor[k]-cell2->coor[k];

    if ( dist[k] >  _MMG5_EPSD ) return 1;

    if ( dist[k] < -_MMG5_EPSD ) return -1;
  }

  assert ( dist[0]*dist[0]+dist[1]*dist[1]+dist[2]*dist[2]<_MMG5_EPSD
    && "Wrong epsilon machine consistency: function to improve");

  return 0;
}

/**
 * \param list   array of PMMG_coorCell
 * \param nitem  number of items in the list
 * \param min    minimal coordinates in each direction
 * \param max    maximal coordinates in each direction
 * \param delta  scaling value
 *
 * \return 1 if success, 0 if fail;
 *
 * Scale the coordinates listed in the \a list array and fill the scaling data
 * (\a min \a max \a delta).
 *
 */
int PMMG_scale_coorCellList (PMMG_coorCell *list,int nitem,double min[3],
                             double max[3],double *delta) {
  double dd;
  int    i,j;

  /* Bounding box computation */
  for (i=0; i<3; i++) {
    min[i] =  DBL_MAX;
    max[i] = -DBL_MAX;
  }
  for ( i=0; i<nitem; ++i ) {
    for (j=0; j<3; j++) {
      if ( list[i].coor[j] > max[j] ) max[j] = list[i].coor[j];
      if ( list[i].coor[j] < min[j] ) min[j] = list[i].coor[j];
    }
  }
  (*delta) = 0.0;
  for (i=0; i<3; i++)
    if ( max[i]-min[i] > (*delta) )  (*delta) = max[i]-min[i];

  if ( (*delta) < _MMG5_EPSD ) {
    fprintf(stderr,"\n  ## Error: %s: unable to scale the list.",__func__);
    return 0 ;
  }

  /* Coordinate scaling */
  dd = 1./(*delta);
  for ( i=0; i<nitem; ++i ) {
    for (j=0; j<3; j++) list[i].coor[j] = dd*(list[i].coor[j]-min[j]);
  }

  return 1;
}

/**
 * \param list   array of PMMG_coorCell
 * \param nitem  number of items in the list
 * \param min    minimal coordinates in each direction
 * \param max    maximal coordinates in each direction
 * \param delta  scaling value
 *
 * \return 1 if success, 0 if fail;
 *
 * Unscale the coordinates listed in the \a list array.
 *
 */
int PMMG_unscale_coorCellList (PMMG_coorCell *list,int nitem,double min[3],
                               double max[3],double delta) {
  int    i,j;

  /* Coordinate scaling */
  for ( i=0; i<nitem; ++i )
    for (j=0; j<3; j++) list[i].coor[j] = delta*list[i].coor[j]+min[j];

  return 1;
}
