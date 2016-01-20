#ifndef __LIBAGF_SVM_MULTI_H__DEFINED__
#define __LIBAGF_SVM_MULTI_H__DEFINED__

#include "svm2class.h"

namespace libagf {
  template <class real>
  int solve_cond_prob_1v1(real **r, int ncls, real *p);

  //multi-class classifier specialized for "one vs. one":
  template <class real, class cls_t>
  class onevone:public classifier_obj<real, cls_t> {
    protected:
      int voteflag;		//voting or solve for probabilities
      cls_t *label;

      virtual real ** classify_raw(real *x)=0;

    public:
      onevone();
      virtual ~onevone();

      virtual cls_t classify(real *x, real *p, real *praw=NULL);
      virtual cls_t class_list(cls_t *cls);

      virtual void print(FILE *fs, char *fbase=NULL, int depth=0)=0;
      virtual int commands(multi_train_param &param, cls_t **clist, char *fbase)=0;
  };

  //we can unify this at some later time with "svm2class" binary classifier
  template <class real, class cls_t>
  class svm_multi:public onevone<real, cls_t> {
    protected:
      real **sv;		//support vectors
      real **coef;		//coefficients
      nel_ta nsv_total;		//total number of support vectors
      nel_ta *nsv;		//number of support vectors
      nel_ta *start;
      real *rho;		//constant terms

          //kernel function:
      real (* kernel) (real *, real *, dim_ta, void *);
      //parameters for kernel function:
      real *param;
      //kernel function with derivatives:
      real (* kernel_deriv) (real *, real *, dim_ta, void *, real *);

      //for calculating probabilities:
      real *probA;
      real *probB;

      virtual real ** classify_raw(real *x);

    public:
      svm_multi();
      svm_multi(char *file, int vflag=0);
      virtual ~svm_multi();

      virtual void print(FILE *fs, char *fbase=NULL, int depth=0);
      virtual int commands(multi_train_param &param, cls_t **clist, char *fbase);

      real R(real *x, cls_t i, cls_t j);
      real R_deriv(real *x, cls_t i, cls_t j, real *drdx);

      svm2class<real, cls_t> * tobinary(cls_t i, cls_t j);

  };

  template <class real, class cls_t>
  class borders1v1:public onevone<real, cls_t> {
    protected:
      real ***bord;
      real ***grad;
      nel_ta *nsamp;
      virtual real ** classify_raw(real *x);

    public:
      borders1v1();
      borders1v1(char *file, int vflag=0);
      virtual ~borders1v1();

      virtual void print(FILE *fs, char *fbase=NULL, int depth=0);
      virtual int commands(multi_train_param &param, cls_t **clist, char *fbase);

  };
}

#endif

