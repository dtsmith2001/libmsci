#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>

//#include "full_util.h"
#include "bit_array.h"
#include "gsl_util.h"
#include "error_codes.h"
#include "randomize.h"
#include "tree_lg.h"

#include "agf_lib.h"

using namespace libpetey;

namespace libagf {

  //print out common control files:
  void one_against_all(FILE *fs, int ncls, const char *options) {
    for (int i=0; i<ncls; i++) {
      if (options!=NULL) {
        if (i==0) fprintf(fs, "\"%s\"", options); else fprintf(fs, "\".\"");
      } else {
        fprintf(fs, "\"\"");
      }
      fprintf(fs, " %d %c", i, PARTITION_SYMBOL);
      for (int j=0; j<i; j++) fprintf(fs, " %d", j);
      for (int j=i+1; j<ncls; j++) fprintf(fs, " %d", j);
      fprintf(fs, ";\n");
    }
  }

  void partition_adjacent(FILE *fs, int ncls, const char *options) {
    for (int i=1; i<ncls; i++) {
      if (options!=NULL) {
        if (i==0) fprintf(fs, "\"%s\"", options); else fprintf(fs, "\".\"");
      } else {
        fprintf(fs, "\"\"");
      }
      for (int j=0; j<i; j++) fprintf(fs, " %d", j);
      fprintf(fs, " %c", PARTITION_SYMBOL);
      for (int j=i; j<ncls; j++) fprintf(fs, " %d", j);
      fprintf(fs, ";\n");
    }
  }

  //need to design a version of this for larger n:
  void random_coding_matrix(FILE *fs, int ncls, int ntrial, int strictflag) {
    tree_lg<int64_t> used;
    int row[ncls];
    int64_t cur, curi;
    int mult;
    int list1[ncls];
    int list2[ncls];
    int n1, n2;

    for (int i=0; i<ntrial; i++) {
      do {
        n1=0;
        n2=0;
        mult=1;
	cur=0;
	curi=0;
        for (int j=0; j<ncls; j++) {
          if (strictflag) {
            row[j]=2*ranu();
            cur+=mult*row[j];
            curi+=mult*(1-row[j]);
            row[j]=2*row[j]-1;
            mult*=2;
          } else {
            row[j]=3*ranu();
            cur+=mult*row[j];
            curi+=mult*(2-row[j]);
            row[j]=row[j]-1;
            mult*=3;
	  }
          if (row[j]<0) {
            list1[n1]=j;
            n1++;
          } else if (row[j]>0) {
            list2[n2]=j;
            n2++;
          }
	}
      } while(used.add_member(cur)<0 || used.add_member(curi)<0 || n1<1 || n2<1);
      fprintf(fs, "\"\"");
      for (int j=0; j<n1; j++) fprintf(fs, " %d", list1[j]);
      fprintf(fs, " %c", PARTITION_SYMBOL);
      for (int j=0; j<n2; j++) fprintf(fs, " %d", list2[j]);
      fprintf(fs, ";\n");
    }
  }

  void exhaustive_coding_matrix(FILE *fs, int ncls) {
    int64_t nrow;

    nrow=pow(2, ncls-1)-1;

    for (int64_t i=0; i<nrow; i++) {
      bit_array *tobits=new bit_array((word *) &i, (sizeof(long)+1)/sizeof(word), ncls-1);
      fprintf(fs, "\"\" ");
      for (long j=0; j<ncls-1; j++) {
        if ((*tobits)[j]==0) fprintf(fs, "%d ", j+1);
      }
      fprintf(fs, "%c ", PARTITION_SYMBOL);
      for (long j=0; j<ncls-1; j++) {
        if ((*tobits)[j]) fprintf(fs, "%d ", j+1);
      }
      fprintf(fs, "0;\n");
      delete tobits;
    }
  }

  //need to design a more efficient version of this...
  void ortho_coding_matrix_brute_force(FILE *fs, int n) {
    long *trial;
    bit_array *tobits;
    int **coding_matrix;
    int nfilled;
    int nperm;

    nperm=pow(2, n);

    coding_matrix=new int *[n];
    coding_matrix[0]=new int[n*n];
    for (int i=1; i<n; i++) coding_matrix[i]=coding_matrix[0]+n*i;

    if (n>8*sizeof(long)-1) {
      fprintf(stderr, "Size must be %d or less\n", 8*sizeof(long)-1);
      exit(PARAMETER_OUT_OF_RANGE);
    }

    trial=randomize(nperm);

    nfilled=0;
    for (int i=0; i<nperm; i++) {
      int dprod;
      //printf("%d\n", trial[i]);
      tobits=new bit_array((word *) (trial+i), (sizeof(long)+1)/sizeof(word), n);
      for (long j=0; j<n; j++) {
        coding_matrix[nfilled][j]=2*(long) (*tobits)[j]-1;
        //printf("%d ", (int32_t) (*tobits)[j]);
      }
      //printf("\n");
      //for (int j=0; j<n; j++) printf("%3d", coding_matrix[nfilled][j]);
      //printf("\n");
      dprod=0;
      for (int j=0; j<nfilled; j++) {
        dprod=0;
        for (int k=0; k<n; k++) {
          dprod+=coding_matrix[nfilled][k]*coding_matrix[j][k];
        }
        if (dprod!=0) break;
      }
      if (dprod==0) {
        //for (int j=0; j<n; j++) printf("%3d", coding_matrix[nfilled][j]);
        //printf("\n");
        nfilled++;
      }
      if (nfilled>=n) break;
      delete tobits;
    }

    for (int i=0; i<nfilled; i++) {
      fprintf(fs, "\"\" ");
      for (int j=0; j<n; j++) {
        if (coding_matrix[i][j]<0) fprintf(fs, "%d ", j);
      }
      fprintf(fs, "%c", PARTITION_SYMBOL);
      for (int j=0; j<n; j++) {
        if (coding_matrix[i][j]>0) fprintf(fs, " %d", j);
      }
      fprintf(fs, ";\n");
    }

    delete [] trial;
    delete [] coding_matrix[0];
    delete [] coding_matrix;
  }

  void print_control_hier(FILE *fs, int ncls, int c0, int depth) {
    for (int i=0; i<depth; i++) fprintf(fs, "  ");
    if (ncls==1) {
      fprintf(fs, "%d\n", c0);
    } else if (ncls>1) {
      fprintf(fs, "\"\" {\n");
      print_control_hier(fs, ncls/2, c0, depth+1);
      print_control_hier(fs, ncls-ncls/2, c0+ncls/2, depth+1);
      fprintf(fs, "\n");
      for (int i=0; i<depth; i++) fprintf(fs, "  ");
      fprintf(fs, "}\n");
    }
  }

  void print_control_1vsall(FILE *fs, int ncls, const char *opt) {
    one_against_all(fs, ncls, opt);
    fprintf(fs, "{");
    for (int i=0; i<ncls; i++) fprintf(fs, " %d", i);
    fprintf(fs, "}\n");
  }

  void print_control_adj(FILE *fs, int ncls, const char *opt) {
    partition_adjacent(fs, ncls, opt);
    fprintf(fs, "{");
    for (int i=0; i<ncls; i++) fprintf(fs, " %d", i);
    fprintf(fs, "}\n");
  }

  void print_control_random(FILE *fs, int ncls, int nrow, int strictflag) {
    random_coding_matrix(fs, ncls, nrow, strictflag);
    fprintf(fs, "{");
    for (int i=0; i<ncls; i++) fprintf(fs, " %d", i);
    fprintf(fs, "}\n");
  }

  void print_control_exhaustive(FILE *fs, int ncls) {
    exhaustive_coding_matrix(fs, ncls);
    fprintf(fs, "{");
    for (int i=0; i<ncls; i++) fprintf(fs, " %d", i);
    fprintf(fs, "}\n");
  }

  void print_control_ortho(FILE *fs, int ncls) {
    ortho_coding_matrix_brute_force(fs, ncls);
    fprintf(fs, "{");
    for (int i=0; i<ncls; i++) fprintf(fs, " %d", i);
    fprintf(fs, "}\n");
  }

}
