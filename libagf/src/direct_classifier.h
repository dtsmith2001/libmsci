#ifndef DIRECT_CLASSIFIER_H
#define DIRECT_CLASSIFIER_H 1

#include <stdio.h>
#include <gsl/gsl_matrix.h>

#include "classifier_obj.h"

namespace libagf {
  //"abstract" base class (now supplying default behaviour, so i guess it's
  //not so abstract after all):
  template <class real, class cls_t>
  class direct_classifier:public classifier_obj<real, cls_t> {
    protected:
      //training data:
      real **train;		//locations of samples
      cls_t *cls;		//classes of samples
      nel_ta ntrain;		//number of samples

      //transformations:
      real **tran;		//transformation matrix
      real *offset;		//offset portion of transformation
      cls_t D1;			//transformed dimension

      //info.:
      char type;		//A=AGF; K=KNN; G=general (external)
      char *options;		//options/command name
      int Mflag;	//(we've let features bleed in from higher level)
      				//LIBSVM ASCII format

      real *do_xtran(real *x);		//perform transformation test point
    public:
      direct_classifier();
      direct_classifier(cls_t nc, char *opts, char t, int mf=0);
      virtual ~direct_classifier();

      int init(real **x, cls_t *c, nel_ta nsamp, dim_ta ndim);

      //linear transformation on features (plus translation...):
      //(transformations are not copied but stored as references)
      virtual int ltran(real **mat, 	//tranformation matrix
		real *b,		//constant term
		dim_ta d1,		//first dimension of trans. mat.
		dim_ta d2, 		//second 	"
		int flag);		//transform model?

      //we're roping these classes in to do double duty:
      virtual void print(FILE *fs, 		//output file stream
		char *fbase=NULL,		//base name of model files
		int depth=0);			//for pretty, indented output
      //generate commands for training:
      virtual int commands(multi_train_param &param, //see structure in multi_parse.h
		cls_t **clist, 			//list of classes, partioned if necessary
		char *fbase);			//base name of model files

  };

  template <class real, class cls_t>
  class agf_classifier:public direct_classifier<real, cls_t> {
    private:
      //diagnostics:
      real min_f;
      real max_f;
      real total_f;
      real min_W;
      real max_W;
      real total_W;
      iter_ta min_nd;
      iter_ta max_nd;
      iter_ta total_nd;
      nel_ta ntrial;
      nel_ta ntrial_k;
    protected:
      nel_ta k;
      real W;
      real var[2];
      FILE *logfs;
    public:
      agf_classifier(const char *fbase, const char *options);
      virtual ~agf_classifier();
      virtual cls_t classify(real *x, real *p, real *praw=NULL);

  };

  template <class real, class cls_t>
  class knn_classifier:public direct_classifier<real, cls_t> {
    protected:
      nel_ta k;
      FILE *logfs;
    public:
      knn_classifier(const char *fbase, const char *options);
      virtual ~knn_classifier();
      virtual cls_t classify(real *x, real *p, real *praw=NULL);
  };

  template <class real, class cls_t>
  class general_classifier:public direct_classifier<real, cls_t> {
    protected:
      char command;		//what command we use
      FILE *logfs;
      int Kflag;
    public:
      general_classifier(const char *com, const char *file, cls_t nc, int mf=0, int kf=0);
      virtual ~general_classifier();
      virtual cls_t classify(real *x, real *p, real *praw=NULL);
      virtual void batch_classify(real **x, cls_t *cls, real **p, nel_ta n, dim_ta nvar);
  };

}

#endif

