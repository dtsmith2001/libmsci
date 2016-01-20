#include <math.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "full_util.h"
#include "agf_lib.h"
#include "randomize.h"

using namespace std;
using namespace libagf;
using namespace libpetey;

int main(int argc, char *argv[]) {
  char *outfile;		//output classes
  char *confile;		//output confidences
  FILE *fs;

  svm_multi<real_a, cls_ta> *classifier;

  //transformation matrix:
  real_a **mat=0;
  real_a *ave;
  dim_ta nvar1, nvar2;

  dim_ta nvar;			//number of variables
  cls_ta nclass;		//number of classes
  nel_ta ntest;			//number of test data points

  real_a **test;		//test data vectors
  cls_ta *result;		//results of classification

  real_a **prob;		//estimated probabilities
  real_a *con;			//estimated confidence

  int errcode;

  agf_command_opts opt_args;

  opt_args.algtype=0;
  //errcode=agf_parse_command_opts(argc, argv, "a:c:nuAEMUZ", &opt_args);
  errcode=agf_parse_command_opts(argc, argv, "AEMU", &opt_args);
  if (errcode==FATAL_COMMAND_OPTION_PARSE_ERROR) return errcode;

  //parse the command line arguments:
  if (argc != 3) {
    printf("Syntax:   classify_s \\\n");
    printf("                  [-A [-M [-E missing]]] \\\n");
    //printf("                  [-n] [-u] [-a normfile] \\\n");
    printf("                  modelfile test output\n");
    printf("\n");
    printf("where:\n");
    printf("  modelfile   file containing LIBSVM classification model\n");
    printf("  test        file containing vector data to be classified\n");
    printf("  output      files containing the results of the classification:\n");
    printf("                .cls for classes, .con for confidence ratings\n");
    printf("\n");
    printf("options:\n");
    //printf("  -n          option to normalise the data\n");
    //printf("  -u          normalize borders data (stored in un-normalized coords)\n");
    //printf("  -a normfile file containing normalization data\n");
    printf("  -A          ASCII format for test data and output\n");
    printf("  -M          LIBSVM format for test data and output\n");
    printf("  -E missing  missing value for LIBSVM features data\n");
    printf("\n");
    return INSUFFICIENT_COMMAND_ARGS;
  }

  ran_init();			//random numbers resolve ties

  //read in class borders:
  //"in-house" SVM predictor:
  classifier=new svm_multi<real_a, cls_ta>(argv[0]);
  //printf("%d border vectors found: %s\n", ntrain, argv[0]);

  if (opt_args.asciiflag) {
    cls_ta *dum;		//class data which we throw away...
    if (opt_args.Mflag) {
      ntest=read_svm(argv[1], test, dum, nvar, opt_args.missing, opt_args.Uflag);
    } else {
      ntest=read_lvq(argv[1], test, dum, nvar);
    }
  } else {
    test=read_vecfile(argv[1], ntest, nvar);
  }
  if (nvar == -1) {
    fprintf(stderr, "Error reading input file: %s\n", argv[1]);
    return FILE_READ_ERROR;
  }
  if (ntest == -1) {
    fprintf(stderr, "Error reading input file: %s\n", argv[1]);
    return ALLOCATION_FAILURE;
  }
  if (test == NULL) {
    fprintf(stderr, "Unable to open file for reading: %s\n", argv[1]);
    return UNABLE_TO_OPEN_FILE_FOR_READING;
  }

  fprintf(stderr, "%d test vectors found in file %s\n", ntest, argv[1]);

  //normalization:
  if ((opt_args.uflag || opt_args.normflag) && opt_args.normfile==NULL) {
    opt_args.normfile=new char [strlen(argv[0])+5];
    sprintf(opt_args.normfile, "%s.std", argv[0]);
  }

  if (opt_args.normfile!=NULL) {
    mat=read_stats2(opt_args.normfile, ave, nvar1, nvar2);

    //errcode=classifier->ltran(mat, ave, nvar1, nvar2, opt_args.uflag);
    if (errcode!=0) exit(errcode);

    if (classifier->n_feat() != nvar) {
      fprintf(stderr, "classify_s: Dimensions of classifier (%d) do not match those of test data (%d).\n",
                classifier->n_feat(), nvar);
      exit(DIMENSION_MISMATCH);
    }
  } else {
    if (classifier->n_feat() != nvar) {
      fprintf(stderr, "classify_s: Dimension of classifier (%d) does not match dimension of test data (%d).\n",
                classifier->n_feat(), nvar);
      exit(DIMENSION_MISMATCH);
    }
  }

  outfile=new char[strlen(argv[2])+5];
  strcpy(outfile, argv[2]);
  strcat(outfile, ".cls");

  confile=new char[strlen(argv[2])+5];
  strcpy(confile, argv[2]);
  strcat(confile, ".con");

  //begin the classification scheme:
  result=new cls_ta[ntest];
  con=new real_a[ntest];
  nclass=classifier->n_class();
  prob=new real_a*[ntest];
  prob[0]=new real_a[ntest*nclass];

  for (nel_ta i=0; i<ntest; i++) {
    prob[i]=prob[0]+i*nclass;
    result[i]=classifier->classify(test[i], prob[i]);
  }

  //printf("\n");

  //write the results to a file:
  cls_ta clist[nclass];
  cls_ta ncls;
  ncls=classifier->class_list(clist);
  assert(nclass==ncls);
  if (opt_args.asciiflag && opt_args.Mflag) {
    fs=fopen(argv[2], "w");
    if (fs == NULL) {
      fprintf(stderr, "Unable to open file, %s, for writing\n", outfile);
      return UNABLE_TO_OPEN_FILE_FOR_WRITING;
    }
    fprintf(fs, "labels\n");
    for (cls_ta i=0; i<ncls; i++) fprintf(fs, " %d", clist[i]);
    fprintf(fs, "\n");
    for (nel_ta i=0; i<ntest; i++) {
      fprintf(fs, "%d", clist[result[i]]);
      for (cls_ta j=0; j<ncls; j++) fprintf(fs, " %g", prob[i][j]);
      fprintf(fs, "\n");
    }
  } else {
    cls_ta maxcls=0;		//largest value for class label
    real_a *p2;
    if (opt_args.asciiflag) {
      fs=fopen(argv[2], "w");
      if (fs == NULL) {
        fprintf(stderr, "Unable to open file, %s, for writing\n", outfile);
        return UNABLE_TO_OPEN_FILE_FOR_WRITING;
      }
    } else {
      fs=stdout;
    }
    for (cls_ta i=0; i<nclass; i++) if (clist[i]>maxcls) maxcls=clist[i];
    p2=new real_a[maxcls+1];
    fprintf(fs, "%d\n", maxcls+1);
    for (nel_ta i=0; i<ntest; i++) {
      fprintf(fs, "%d", result[i]);
      for (cls_ta j=0; j<nclass; j++) p2[j]=0;
      for (cls_ta j=0; j<nclass; j++) p2[clist[j]]=prob[i][j];
      for (cls_ta j=0; j<nclass; j++) {
        fprintf(fs, " %g", p2[j]);
      }
      fprintf(fs, "\n");
      con[i]=(nclass*p2[result[i]]-1)/(nclass-1);
    }
    delete [] p2;
    if (opt_args.asciiflag) {
      fclose(fs);
    } else {
      fs=fopen(outfile, "w");
      if (fs == NULL) {
        fprintf(stderr, "Unable to open file, %s, for writing\n", outfile);
        return UNABLE_TO_OPEN_FILE_FOR_WRITING;
      }
      fwrite(result, sizeof(cls_ta), ntest, fs);
      fclose(fs);

      //write the results to a file:
      fs=fopen(confile, "w");
      if (fs == NULL) {
        fprintf(stderr, "Unable to open file, %s, for writing\n", confile);
        return UNABLE_TO_OPEN_FILE_FOR_WRITING;
      }
      fwrite(con, sizeof(real_a), ntest, fs);
      fclose(fs);
    }
  }
  
  //clean up:
  delete [] result;
  delete [] con;
  delete [] prob[0];
  delete [] prob;
  delete [] test[0];
  delete [] test;
  delete [] outfile;
  delete [] confile;
  if (mat!=NULL) {
    delete_matrix(mat);
    delete [] ave;
  }
  if (opt_args.normfile!=NULL) delete [] opt_args.normfile;
  delete classifier;

  ran_end();

}

