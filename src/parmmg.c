#include "libparmmg.h"

/**
 * \param argc number of command line arguments.
 * \param argv command line arguments.
 * \return \ref PMMG_SUCCESS if success.
 * \return \ref PMMG_LOWFAILURE if failed but a conform mesh is saved.
 * \return \ref PMMG_STRONGFAILURE if failed and we can't save the mesh.
 *
 * Main program for PARMMG executable: perform parallel mesh adaptation.
 *
 */
int main(int argc,char *argv[]) {
  PMMG_pParMesh    parmesh;
  int              *part;

  if ( parmesh )  _MMG5_SAFE_FREE(parmesh);
  _MMG5_SAFE_CALLOC(parmesh,1,PMMG_ParMesh);
  
  /*Init MPI*/
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &parmesh->nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &parmesh->myrank);

#warning CECILE : do we need a variadic Init for Parmmg ?
  //???PMMG_Init(parmesh);
  
  if(!parmesh->myrank) {
    fprintf(stdout,"  -- PARMMG3d, Release %s (%s) \n",PMMG_VER,PMMG_REL);
    fprintf(stdout,"     %s\n",PMMG_CPY);
    fprintf(stdout,"     %s %s\n",__DATE__,__TIME__);

    /*Read sequential mesh*/
#warning : for the moment, we only read a mesh named m.mesh
    if(!PMMG_loadMesh(parmesh,"m.mesh")) return(PMMG_STRONGFAILURE);
    
    /*call metis for partionning*/
    _MMG5_SAFE_CALLOC(part,(parmesh->listgrp[0].mesh)->np,int);
    if(!PMMG_metispartitioning(parmesh,part)) return(PMMG_STRONGFAILURE);
    
    /*send mesh partionning to other proc*/
    if(!PMMG_distributeMesh(parmesh,part)) return(PMMG_STRONGFAILURE);
    _MMG5_SAFE_FREE(part);
  } {
    /*receive mesh*/
    if(!PMMG_distributeMesh(parmesh,NULL)) return(PMMG_STRONGFAILURE);
  }

  /*perform mesh adaptation*/
  
  

  if(!parmesh->myrank) {
    /*receive mesh*/

    /*write mesh*/
#warning : for the moment, we only write a mesh named out.mesh
    PMMG_saveMesh(parmesh,"out.mesh");
  } else {
    /*send mesh*/
  }

  /*free structures*/
  
  /*Finalize MPI*/
  MPI_Finalize();

  return(PMMG_SUCCESS);
}
