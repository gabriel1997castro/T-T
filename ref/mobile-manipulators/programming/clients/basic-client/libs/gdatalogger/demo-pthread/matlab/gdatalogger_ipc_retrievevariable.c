#include "mex.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include "gqueue.h"
#include "gmatlabdatafile.h"
#include "gdatalogger.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	char  *varname;
	char  varunit[50];
	int	  i;
	double data[GDATALOGGER_IPC_MAXQUEUESIZE];
	int ndata;
    int status;
	double *pMat;

	/* Verifier les entr�es */
	if(nrhs!=1){
		mexErrMsgTxt("One input required.");
		}
	if(nlhs!=3){
		mexErrMsgTxt("Three outputs required.");
		}
    if ( mxIsChar(prhs[0]) != 1){
      mexErrMsgTxt("Input must be a string.");
	}
    if (mxGetM(prhs[0])!=1){
      mexErrMsgTxt("Input must be a row vector.");
	}
                
	/* Variables d'entr�e */
    varname = mxArrayToString(prhs[0]);
    if(varname == NULL) {
      mexErrMsgTxt("Could not convert input to string.");
	}

	/* Fonction */

	if(!gDataLogger_Init(NULL,NULL,NULL)){
		mexErrMsgTxt("\nError in gDataLogger_Init\n\n");
	}
	status = gDataLogger_IPC_RetrieveVariable(varname, varunit, data, &ndata);

	/* Variables de sortie */
	plhs[0] = mxCreateDoubleMatrix(ndata,1,mxREAL);
	pMat = mxGetPr(plhs[0]);
	for (i=0;i<ndata;++i){
		pMat[i] = data[i];
	}
    
    if(status==GDATALOGGER_IPC_FLAGDATAAVAILABLE){
        plhs[1] = mxCreateString(varunit);
    }
    else{
        plhs[1] = mxCreateString("");
    }
    
    plhs[2] = mxCreateDoubleMatrix(1,1,mxREAL);
	pMat = mxGetPr(plhs[2]);
    *pMat = (double)status;
    
/*    printf("__LINE__ = %i\n",__LINE__); */
}
