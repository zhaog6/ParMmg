/**
 * \file variadic_pmmg.c
 * \brief C variadic functions definitions for PMMG library.
 * \author Algiane Froehly (InriaSoft)
 * \version 1
 * \date 07 2018
 * \copyright GNU Lesser General Public License.
 *
 * \note This file contains some internal functions for the API, see
 * the \ref libparmmg.h header file for the documentation of all
 * the usefull user's API functions.
 *
 * variadic functions definitions for PMMG library.
 *
 */

#include "parmmg.h"

/**
 * \param argptr list of the type of structures that must be initialized inside
 * your parmesh and needed informations for ParMmg (mesh dimension and MPI
 * communicator).
 *
 * \a argptr contains at least a pointer toward a parmesh pointer preceeded by
 * the PMMG_ARG_pParMesh keyword and the mesh dimension preceeded by the
 * PMMG_ARG_dim keyword
 *
 * By default, ParMmg will initilized at least 1 mesh, 1 metric and the
 * MPI_COMM_WORLD_COMMUNICATOR inside your parmesh. Thus, the 2 following calls
 * are identicals:
 *
 * 1) MMG3D_Init_parmesh(PMMG_ARG_start,PMMG_ARG_ppParMesh,your_pParmesh_address,
 *    PMMG_ARG_pMesh,PMMG_ARG_pMet,
 *    PMMG_ARG_dim,mesh_dimension,PMMG_ARG_MPIComm,MPI_COMM_WORLD,PMMG_ARG_end)
 *
 * 2) MMG3D_Init_parmesh(PMMG_ARG_start,PMMG_ARG_ppParMesh,your_pParmesh_address,
 *    PMMG_ARG_dim,mesh_dimension,PMMG_ARG_end)
 *
 * \return 1 if success, 0 if fail
 *
 * Internal function for structure allocations (taking a va_list argument).
 *
 */
int PMMG_Init_parMesh_var( va_list argptr ) {
  PMMG_pParMesh  *parmesh;
  MPI_Comm       comm;
  int            typArg,dim,nsol;
  int            parmeshCount,meshCount,metCount,dimCount,solCount,commCount;
  PMMG_pGrp      grp;

  parmeshCount = 0;
  meshCount    = 0;
  metCount     = 0;
  solCount     = 0;
  dimCount     = 0;
  commCount    = 0;

  nsol = 0;
  dim  = 3;
  comm = MPI_COMM_WORLD;
  while ( (typArg = va_arg(argptr,int)) != PMMG_ARG_end )
  {
    switch ( typArg )
    {
    case(PMMG_ARG_ppParMesh):
      parmesh = va_arg(argptr,PMMG_pParMesh*);
      ++parmeshCount;
      break;
    case(PMMG_ARG_pMesh):
      ++meshCount;
      break;
    case(PMMG_ARG_pMet):
      ++metCount;
      break;
    case(PMMG_ARG_dim):
      ++dimCount;
      dim = va_arg(argptr,int);
      break;
    case(PMMG_ARG_MPIComm):
      ++commCount;
      comm = va_arg(argptr,MPI_Comm);
      break;
    default:
      fprintf(stderr,"\n  ## Error: PMMG_Init_parmesh:\n"
              " unexpected argument type: %s\n",PMMG_Get_pmmgArgName(typArg));
      return 0;
    }
  }

  if ( parmeshCount !=1 ) {
    fprintf(stderr,"\n  ## Error: PMMG_Init_parmesh:\n"
            " you need to initialize the parmesh structure that"
            " will contain your data (mesh, metric, communicator...\n");
    return 0;
  }

  if ( meshCount>1 ) {
    fprintf(stdout,"\n  ## Warning: PMMG_Init_parmesh:\n"
            " Only 1 mesh structure is allowed.\n");
  }
  if ( metCount>1 ) {
    fprintf(stdout,"\n  ## Warning: PMMG_Init_parmesh:\n"
            " Only 1 metric structure is allowed.\n");
  }
  if ( commCount>1 ) {
    fprintf(stdout,"\n  ## Warning: PMMG_Init_parmesh:\n"
            " More than 1 MPI communicator provided. Used the last one.\n");
  }
  if ( dimCount>1 ) {
    fprintf(stdout,"\n  ## Warning: PMMG_Init_parmesh:\n"
            " More than 1 dimension provided. Used the last one.\n");
  }
  else if ( !dimCount ) {
    fprintf(stderr,"\n  ## Error: PMMG_Init_parmesh:\n"
            " you need to provide the dimension of your mesh using the PMMG_dim"
            " keyword\n.");
    return 0;
  }

  if ( dim !=3 ) {
    fprintf(stderr,"\n  ## Error: PMMG_Init_parmesh:\n"
            " dimension other than 3D is not yet implemented.\n");
    return 0;
  }

  /* ParMesh allocation */
  assert ( (*parmesh == NULL) && "trying to initialize non empty parmesh" );
   *parmesh = calloc( 1, sizeof(PMMG_ParMesh) );

  if ( *parmesh == NULL ) {
    return 0;
  }

  /* Assign some values to memory related fields to begin working with */
  (*parmesh)->memGloMax = 4 * 1024L * 1024L;
  (*parmesh)->memMax = 4 * 1024L * 1024L;
  (*parmesh)->memCur = sizeof(PMMG_ParMesh);

  /** Init Group */
  grp = NULL;
  (*parmesh)->ngrp = 1;
  PMMG_CALLOC(*parmesh,(*parmesh)->listgrp,1,PMMG_Grp,
              "allocating groups container", goto fail_grplst );
  grp = &(*parmesh)->listgrp[0];
  grp->mesh = NULL;
  grp->met  = NULL;
  grp->sol  = NULL;
  grp->disp = NULL;
  if ( 1 != MMG3D_Init_mesh( MMG5_ARG_start,
                             MMG5_ARG_ppMesh, &grp->mesh,
                             MMG5_ARG_ppMet, &grp->met,
                             MMG5_ARG_end ) )
    goto fail_mesh;

  PMMG_Init_parameters(*parmesh,comm);

  return 1;

fail_mesh:
    PMMG_DEL_MEM(*parmesh,(*parmesh)->listgrp,1,PMMG_Grp,
                 "deallocating groups container");

fail_grplst:
  (*parmesh)->ngrp = 0;
  (*parmesh)->memMax = 0;
  (*parmesh)->memCur = 0;
  _MMG5_SAFE_FREE( *parmesh );

  return 1;
}

/**
 * \param argptr list of the parmmg structures that must be deallocated.
 *
 * \a argptr contains at least a pointer toward a \a PMMG_pParMesh structure
 * (that will contain the parmesh and identified by the PMMG_ARG_ppParMesh keyword)
 *
 * \return 0 if fail, 1 if success
 *
 * Deallocations of the parmmg structures before return
 *
 * \remark we pass the structures by reference in order to have argument
 * compatibility between the library call from a Fortran code and a C code.
 */
int PMMG_Free_all_var(va_list argptr)
{

  PMMG_pParMesh  *parmesh;
  int            typArg;
  int            parmeshCount;

  parmeshCount = 0;

  while ( (typArg = va_arg(argptr,int)) != PMMG_ARG_end )
  {
    switch ( typArg )
    {
    case(PMMG_ARG_ppParMesh):
      parmesh = va_arg(argptr,PMMG_pParMesh*);
      ++parmeshCount;
      break;
    default:
      fprintf(stdout,"\n  ## Warning: PMMG_Free_all:\n"
              " ignored argument: %s\n",PMMG_Get_pmmgArgName(typArg));
    }
  }

  PMMG_parmesh_Free_Comm( *parmesh );

  PMMG_parmesh_Free_Listgrp( *parmesh );

  _MMG5_SAFE_FREE(*parmesh);

  return 1;

}
