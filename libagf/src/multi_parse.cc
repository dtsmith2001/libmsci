#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>

#include "full_util.h"
#include "peteys_tmpl_lib.h"

#include "agf_lib.h"
#include "multi_parse.h"

using namespace std;
using namespace libpetey;

namespace libagf {

  int scan_whitespace(FILE *fs, int &lineno) {
    int c1;
    char c2;
    do {
      c1=fgetc(fs);
      if (c1==EOF) break;
      c2=(char) c1;
      //printf("%c\n", c2);
      if (c2=='\n') lineno++;
    } while (isspace(c2)==0);

    return c1;
  }

  int scan_nowhitespace(FILE *fs, int &lineno) {
    int c1;
    char c2;
    do {
      c1=fgetc(fs);
      if (c1==EOF) break;
      c2=(char) c1;
      //printf("%c\n", c2);
      if (c2=='\n') lineno++;
    } while (isspace(c2));

    return c1;
  }

  int scan_tochar(FILE *fs, const char *charlist, int &lineno) {
    int c1;
    char c2;
    int flag=1;
    do {
      c1=fgetc(fs);
      if (c1==EOF) break;
      c2=(char) c1;
      //printf("%c\n", c2);
      if (c2=='\n') lineno++;
      for (int i=0; charlist[i]!='\0'; i++) {
        if (charlist[i]==c2) {
          flag=0;
	  break;
	}
      }
    } while (flag);

    return c1;
  }

  int scan_tofunc(FILE *fs, int (* func)(int), int &lineno) {
    int c1;
    char c2;
    do {
      c1=fgetc(fs);
      if (c1==EOF) break;
      c2=(char) c1;
      if (c2=='\n') lineno++;
    } while ((*func)(c2)==0);

    return c1;
  }

  int scan_nofunc(FILE *fs, int (* func)(int), int &lineno) {
    int c1;
    char c2;
    do {
      c1=fgetc(fs);
      if (c1==EOF) break;
      c2=(char) c1;
      if (c2=='\n') lineno++;
    } while ((*func)(c2));

    return c1;
  }

  //scan for a file name:
  char * scan_class_model1(FILE *fs, int &lineno) {
    int c1;
    char c2;
    char *fname;
    int pos1;
    int namelen;
    int numflag=0;

    //printf("scan_class_model1--part 1\n");
    c1=scan_nowhitespace(fs, lineno);
    if (c1==EOF) return NULL;
    c2=(char) c1;
    if (isdigit(c2)) {
      numflag=1;
    } else if (isalpha(c2)==0) {
      return NULL;
    }

    //scan until we reach the end of the file name:
    //printf("scan_class_model1--part 2\n");
    pos1=ftell(fs);
    c1=scan_whitespace(fs, lineno);
    if (c1==EOF) return NULL;
    namelen=ftell(fs)-pos1;

    //put the filename in a string (whether or not we need it...):
    fname=new char[namelen+1];
    fseek(fs, -namelen-1, SEEK_CUR);
    for (int i=0; i<namelen; i++) fname[i]=fgetc(fs);
    fname[namelen]='\0';

    //printf("%s\n", fname);

    return fname;
  }

  //scan for a set of options enclosed in quotation marks:
  char * scan_class_model2(FILE *fs, int &lineno) {
    int c1;
    char c2;
    char *fname;
    int namelen;
    int pos1;
    int numflag=0;

    //printf("scan_class_model2--part 1\n");
    c1=scan_nowhitespace(fs, lineno);
    if (c1==EOF) return NULL;
    c2=(char) c1;
    if (isdigit(c2)) {
      numflag=1;
    } else if (c2!='"') {
      return NULL;
    }

    //printf("scan_class_model2--part 2\n");
    pos1=ftell(fs);
    if (numflag){
      c1=scan_whitespace(fs, lineno);
      if (c1==EOF) return NULL;
      namelen=ftell(fs)-pos1;
    } else {
      c1=scan_tochar(fs, "\"", lineno);
      if (c1==EOF) return NULL;
      namelen=ftell(fs)-pos1-1;
    }
    //printf("namelen=%d\n", namelen);

    //put the filename in a string (whether or not we need it...):
    fname=new char[namelen+1];
    fseek(fs, -namelen-1, SEEK_CUR);
    for (int i=0; i<namelen; i++) fname[i]=fgetc(fs);
    fname[namelen]='\0';
    if (numflag==0) c1=fgetc(fs);

    //printf("scan_class_model2: %s\n", fname);

    return fname;
  }

  //scan specifically for a class label: 
  //- if one is found, return it as a string
  //- if not, rewind to the first non-whitespace character
  char * scan_class_label(FILE *fs, int &lineno) {
    int c1;
    char c2;
    char *fname;
    int namelen;
    int pos1;

    //printf("scan_class_label--part 1\n");
    c1=scan_nowhitespace(fs, lineno);
    if (c1==EOF) return NULL;
    c2=(char) c1;
    if (isdigit(c2)==0) {
      fseek(fs, -1, SEEK_CUR);
      return NULL;
    }

    //printf("scan_class_label--part 2\n");
    pos1=ftell(fs);
    c1=scan_nofunc(fs, &isdigit, lineno);
    if (c1==EOF) return NULL;
    namelen=ftell(fs)-pos1;
    //printf("namelen=%d\n", namelen);

    //put the class label in a string:
    fname=new char[namelen+1];
    fseek(fs, -namelen-1, SEEK_CUR);
    for (int i=0; i<namelen; i++) fname[i]=fgetc(fs);
    fname[namelen]='\0';

    //printf("scan_class_label: label=%s\n", fname);

    return fname;
  }


  template <class cls_t>
  int parse_multi_partitions(multi_parse_param *param, char **model, cls_t **partition, int maxn) {
    int npart=0;
    int nread;
    int c1;
    char c2;
    char cold;
    cls_t ncls;
    long place;		//file place holder

    for (int i=0; i<maxn; i++) {
      if (param->trainflag) {
        model[npart]=scan_class_model2(param->infs, param->lineno);
      } else {
        model[npart]=scan_class_model1(param->infs, param->lineno);
      }

      if (model[npart]==NULL) {
        fseek(param->infs, -1, SEEK_CUR);
        break;
      }

      if (isdigit(model[npart][0])) {
        fprintf(stderr, "parse_multi_partitions: syntax error, line %d at %s\n", param->lineno, model[npart]);
        exit(FILE_READ_ERROR);
      }

      if (param->trainflag) {
        //if options are a special case, pull them from the stack
        //add new options to the stack
        if (param->optstack[param->stackptr]!=NULL &&
			strcmp(model[npart], ".")==0) {
          delete [] model[npart];
          model[npart]=new char [strlen(param->optstack[param->stackptr])+1];
          strcpy(model[npart], param->optstack[param->stackptr]);
        } else {
          if (strlen(model[npart])==0) {
            delete [] model[npart];
            model[npart]=new char[strlen(param->optstack[0])+1];
            strcpy(model[npart], param->optstack[0]);
          } else if (strcmp(model[npart], ".")==0) {
            delete [] model[npart];
            model[npart]=new char[strlen(param->optstack[param->stackptr-1])+1];
            strcpy(model[npart], param->optstack[param->stackptr-1]);
          }
          if (param->optstack[param->stackptr]!=NULL) {
            delete [] param->optstack[param->stackptr];
          }
          param->optstack[param->stackptr]=new char [strlen(model[npart])+1];
          strcpy(param->optstack[param->stackptr], model[npart]);
        }
      }

      nread=0;
      cold=' ';
      ncls=0;
      do {
        c1=fgetc(param->infs);
        nread++;
        if (c1==EOF) {
          fprintf(stderr, "parse_multi_partitions: %d, unexpected end of file(1).\n", param->lineno);
          exit(FILE_READ_ERROR);
        }
        c2=(char) c1;
        //printf("3.(%c)\n", c2);
        if (isdigit(c2)) {
          if (isdigit(cold)==0) ncls++;
        } else if (c2==PARTITION_SYMBOL) {
          place=ftell(param->infs);
          break;
        } else if (c2=='\n') {
          param->lineno++;
        } else if (c2!=' ' && c2!='\t') {
          fprintf(stderr, "parse_multi_partitions: %d, syntax error in control file(2).\n", param->lineno);
          exit(FILE_READ_ERROR);
        }
        cold=c2;
      } while (1);
      //printf("ncls=%d\n", ncls);
      fseek(param->infs, -nread, SEEK_CUR);
      partition[2*npart]=new cls_t[ncls+1];
      for (int i=0; i<ncls; i++) {
        fscanf(param->infs, "%d", partition[2*npart]+i);
        //printf("%d\n", partition[2*npart][i]);
      }
      partition[2*npart][ncls]=-1;

      fseek(param->infs, place, SEEK_SET);

      nread=0;
      cold=' ';
      ncls=0;
      do {
        c1=fgetc(param->infs);
        nread++;
        if (c1==EOF) {
          fprintf(stderr, "parse_multi_partitions: %d, unexpected end of file(2).\n", param->lineno);
          exit(FILE_READ_ERROR);
        }
        c2=(char) c1;
        //printf("4.(%c)\n", c2);
        //if (c2<='9' && c2>='0') {
        if (isdigit(c2)) {
          if (isdigit(cold)==0) ncls++;
        } else if (c2==';') {
          place=ftell(param->infs);
          break;
        } else if (c2=='\n') {
          param->lineno++;
        } else if (c2!=' ' && c2!='\t') {
          fprintf(stderr, "parse_multi_partitions: %d, syntax error in control file(3).\n", param->lineno);
          exit(FILE_READ_ERROR);
        }
        cold=c2;
      } while (1);
      //printf("ncls=%d\n", ncls);

      fseek(param->infs, -nread, SEEK_CUR);
      partition[2*npart+1]=new cls_t[ncls+1];
      partition[2*npart+1][ncls]=-1;
      for (cls_t i=0; i<ncls; i++) {
        fscanf(param->infs, "%d", partition[2*npart+1]+i);
        //printf("%d\n", partition[2*npart+1][i]);
      }

      fseek(param->infs, place, SEEK_SET);
    
      if (npart < maxn) {
        npart++; 
      } else {
        fprintf(stderr, "parse_multi_partitions: Maximum of %d partitions allowed.\n", MAXNPART);
        fprintf(stderr, "  Re-run after increasing MAXNPART macro.  Sorry.\n");
        exit(PARAMETER_OUT_OF_RANGE);
      }
    }

    if (npart==0) return 0;

    //augment stack pointer:
    if (param->trainflag) {
      if (param->stackptr<param->maxnstack) {
        param->stackptr++;
      } else {
        fprintf(stderr, "parse_multi_partitions: option stack exausted (%d levels)\n", param->stackptr);
        exit(PARAMETER_OUT_OF_RANGE);
      }
    }

    return npart;
  }

  //checke the integrity of the partitions:
  //1  = "non-strict" partitioning
  //-1 = error
  template <class cls_t>
  int multi_partition_strict(char **model, cls_t **partition, int npart) {
    //check them over:
    //1. each partition must have the same number of classes
    //2. there must be one of each class present between 0 and ncls-1
    cls_t j, k;
    cls_t *tbl;
    cls_t *tbl_ttl;
    cls_t ncls;
    int err=0;

    //count the number of classes:
    ncls=0;
    for (int i=0; i<npart; i++) {
      //printf("partition: %d\n", npart);
      for (j=0; partition[2*i][j]>=0; j++) {
        if (partition[2*i][j]>=ncls) ncls=partition[2*i][j]+1;
        //printf("%d ", partition[0][ncls]);
      }
      //printf("| ");
      for (j=0; partition[2*i+1][j]>=0; j++) {
        if (partition[2*i+1][j]>=ncls) ncls=partition[2*i+1][j]+1;
        //printf("%d\n", partition[1][j]);
      }
    }
    //printf("\n");
    //printf("ncls=%d\n", ncls);

    tbl=new cls_t[ncls];
    tbl_ttl=new cls_t[ncls];
    for (j=0; j<ncls; j++) tbl_ttl[j]=0;
    for (cls_t i=0; i<npart; i++) {
      for (j=0; j<ncls; j++) tbl[j]=0;
      for (k=0; partition[2*i][k]>=0; k++) {
        tbl[partition[2*i][k]]+=1;
      }
      for (k=0; partition[2*i+1][k]>=0; k++) {
        tbl[partition[2*i+1][k]]+=1;
      }
      for (j=0; j<ncls; j++) {
        if (tbl[j]==0) {
          //fprintf(stderr, "parse_multi_partitions: missing or duplicate class in partition.\n");
	  err=1;
	} else if (tbl[j]>1) {
          fprintf(stderr, "multi_partition_strict: duplicate class in partition %d.\n", i);
	  err=-1;
        }
	//printf("%d: tbl[%d]=%d\n", i, j, tbl[j]);
	tbl_ttl[j]+=tbl[j];
      }
    }
    for (j=0; j<ncls; j++) {
      if (tbl_ttl[j]==0) {
        fprintf(stderr, "multi_partitions_strict: missing class %d.\n", j);
	//GIGO
        //err=-1;
      } 
    }
    delete [] tbl;
    delete [] tbl_ttl;

    return err;

  }

  char * parse_multi_start(multi_parse_param *param, 	//all the parameters globbed into one
		int &flag, 		//0=regular 2-class model, 1=multi-class model, 2=single class label
		char *&extra)		//options for direct classification
  {

    char *fname;
    int pos1;
    int c1;
    char c2;

    flag=0;
    extra=NULL;		//up to calling routine to delete this

    pos1=ftell(param->infs);

    //scan for the file containing the model:
    if (param->trainflag) {
      fname=scan_class_model2(param->infs, param->lineno);
      //in case model is a multi-partition model, rewind extra 2 characters because of quotes:
    } else {
      fname=scan_class_model1(param->infs, param->lineno);
      //filenames for final model not enclosed by quotes:
    }

    if (fname==NULL) {
      c1=fgetc(param->infs);
      if (c1==EOF) {
        fprintf(stderr, "parse_multi_start: %d, syntax error at end of file(1)\n", param->lineno);
      } else {
        c2=char(c1);
        if (c2=='\n') {
          fprintf(stderr, "parse_multi_start: %d, syntax error in control file(2) at end of line\n", param->lineno);
        } else {
          fprintf(stderr, "parse_multi_start: %d, syntax error in control file(3) at \"%c\"\n", param->lineno, c2);
        }
      }
      exit(FILE_READ_ERROR);
    }

    //if we just have a number, return with it in a string:
    if (fname[0]>='0' && fname[0]<='9') {
      flag=2;
      return fname;
    }

    //scan forward until we find a character that's not whitespace:
    c1=scan_nowhitespace(param->infs, param->lineno);
    if (c1==EOF) {
      fprintf(stderr, "parse_multi_start: %d, unexpected end of file(3).\n", param->lineno);
      exit(FILE_READ_ERROR);
    }
    c2=(char) c1;
    if (isdigit(c2)) {
      fseek(param->infs, pos1, SEEK_SET);
      flag=1;
    } else {
      if (c2=='A' || c2=='K' || c2=='G') {
        flag=c1;
        if (param->trainflag) {
          c1=scan_nowhitespace(param->infs, param->lineno);
          c2=(char) c1;
	} else {
          //we need the options/commandname to go with the model:
	  //fprintf(stderr, "parse_multi_start: scanning for required options\n");
	  extra=scan_class_model2(param->infs, param->lineno);
          c1=scan_nowhitespace(param->infs, param->lineno);
          c2=(char) c1;
	}
      }
      if (c2!='{') {
        fprintf(stderr, "parse_multi_start: %d, syntax error in control file(4) at \"%c\".\n", param->lineno, c2);
        exit(FILE_READ_ERROR);
      }
    }

    //param->depth++;

    return fname;

  }

  void print_opt_stack(multi_parse_param *param) {
    for (int i=0; i<param->stackptr; i++) printf("%s\n", param->optstack[i]);
  }

  template <class cls_t>
  void print_partition_list(FILE *fs, cls_t **part, cls_t npart) {
    for (cls_t i=0; i<npart; i++) {
      for (cls_t j=0; part[0][j]>=0; j++) {
        fprintf(fs, "%d ", part[i*2][j]);
      }
      fprintf(fs, "%c", PARTITION_SYMBOL);
      for (cls_t j=0; part[1][j]>=0; j++) {
        fprintf(fs, " %d", part[i*2+1][j]);
      }
      fprintf(fs, "\n");
    }
  }

  template int parse_multi_partitions<cls_ta>(multi_parse_param *, char **, cls_ta **, int );
  template int multi_partition_strict<cls_ta>(char **, cls_ta **, int);

}
