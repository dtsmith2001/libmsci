#ifndef _LIBAGF__SVM2CLASS__H_
#define _LIBAGF__SVM2CLASS__H_

#include "agf_defs.h"
#include "binaryclassifier.h"

namespace libagf {
  template <class real, class cls_t>
  class svm2class:public binaryclassifier<real, cls_t> {
    protected:
      real **sv;		//support vectors
      real *coef;		//coefficients
      nel_ta nsv;		//number of support vectors
      real rho;			//constant term

      //kernel function:
      real (* kernel) (real *, real *, dim_ta, void *);
      //parameters for kernel function:
      real *param;
      //kernel function with derivatives:
      real (* kernel_deriv) (real *, real *, dim_ta, void *, real *);

      //for calculating probabilities:
      real probA;
      real probB;

      //book-keeping:
      cls_t label1;
      cls_t label2;
    public:
      svm2class(char *modfile);
      ~svm2class();
      int init(char *modfile);
      virtual real R(real *x, real *praw=NULL);
      virtual cls_t class_list(cls_t *cls);
      real R_deriv(real *x, real *praw=NULL);

  };

}

#endif
