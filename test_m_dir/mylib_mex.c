/* Auto-generated MATLAB MEX FFI bindings for mylib */

#include "mex.h"
#include <string.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  char func_name[256];

  if (nrhs < 1) {
    mexErrMsgIdAndTxt("cdd:mex:nrhs",
                      "First input must be the function name string.");
  }

  if (mxGetString(prhs[0], func_name, sizeof(func_name)) != 0) {
    mexErrMsgIdAndTxt("cdd:mex:func_name",
                      "Could not convert function name to string.");
  }
}
