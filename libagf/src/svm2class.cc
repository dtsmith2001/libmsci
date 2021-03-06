#include <math.h>
#include <string.h>

#include "read_ascii_all.h"
#include "error_codes.h"
#include "full_util.h"

#include "agf_lib.h"

using namespace libpetey;

namespace libagf {

  template <class real, class cls_t>
  svm2class<real, cls_t>::svm2class() {
    classifier=NULL;
    dflag=0;
  }

  template <class real, class cls_t>
  svm2class<real, cls_t>::svm2class(char *modfile) {
    cls_t *clist;
    classifier=new svm_multi<real, cls_t>(modfile);
    this->ncls=classifier->n_class();
    if (this->ncls != 2) {
      fprintf(stderr, "svm2class: only binary classifiers accepted (file, %s)\n", modfile);
      throw PARAMETER_OUT_OF_RANGE;
    }
    dflag=1;
    clist=new cls_t[this->ncls];
    classifier->class_list(clist);
    if (clist[0]==0 && clist[1]==1) {
      ind1=1;
      ind2=0;
    } else {
      ind1=0;
      ind2=1;
    }
    label1=clist[ind2];
    label2=clist[ind1];
    delete [] clist;
  }

  template <class real, class cls_t>
  svm2class<real, cls_t>::svm2class(svm_multi<real, cls_t> *other, cls_t i, cls_t j, int cflag) {
    cls_t *clist;
    cls_t oncls=other->n_class();
    if (cflag) {
      classifier=new svm_multi<real, cls_t>(other);
      dflag=1;
    } else {
      classifier=other;
      dflag=0;
    }
    //a lot of book-keeping, dammit:
    this->D=other->n_feat_t();
    this->D1=other->n_feat();
    this->ncls=2;
    if (i==j || i>=oncls || j>=oncls) throw PARAMETER_OUT_OF_RANGE;
    ind1=i;
    ind2=j;
    clist=new cls_t[classifier->n_class()];
    classifier->class_list(clist);
    label2=clist[i];
    label1=clist[j];
    delete [] clist;
  }

  template <class real, class cls_t>
  svm2class<real, cls_t>::~svm2class() {
    if (dflag) delete classifier;
  }

  //some book-keeping:
  template <class real, class cls_t>
  cls_t svm2class<real, cls_t>::class_list(cls_t *list) {
    list[0]=label1;
    list[1]=label2;
    return this->ncls;
  }

  template <class real, class cls_t>
  real svm2class<real, cls_t>::R(real *x, real *praw) {
    return classifier->R(x, ind1, ind2, praw);
  }

  template <class real, class cls_t>
  real svm2class<real, cls_t>::R_deriv(real *x, real *drdx) {
    real r=classifier->R_deriv(x, ind1, ind2, drdx);
    return r;
    real drdx2[this->D1];
    real r2=R(x);
    this->R_deriv_num(x, 0.005, drdx2);
    printf("%g %g\n", r, r2);
    for (int i=0; i<this->D1; i++) {
      printf("%g %g\n", drdx[i], drdx2[i]);
    }
    printf("\n");
  }

  template <class real, class cls_t>
  int svm2class<real, cls_t>::ltran_model(real **mat1, real *b1, dim_ta d1, dim_ta d2) {
    return classifier->ltran_model(mat1, b1, d1, d2);
  }

  template <class real, class cls_t>
  real svmrfunc(real *x, void *param, real *deriv) {
    bordparam<real> *p1=(bordparam<real> *) param;
    svm2class<real, cls_t> *p2=(svm2class<real, cls_t> *) p1->rparam;
    return p2->R_deriv(x, deriv);
  }

  template class svm2class<float, cls_ta>;
  template class svm2class<double, cls_ta>;

  template float svmrfunc<float, cls_ta>(float *, void *, float *);
  template double svmrfunc<double, cls_ta>(double *, void *, double *);

}
