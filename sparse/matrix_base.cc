#include <typeinfo>
#include "matrix_base.h"

#include "error_codes.h"

#include "full_matrix.h"
#include "full_util.h"
#include "sparse_array.h"

#include "read_ascii_all.h"

#include "sparse_calc_defs.h"

namespace libpetey {
  namespace libsparse {

    //initialization routines:
    template <class index_t, class scalar>
    matrix_base<index_t, scalar>::matrix_base() {
      m=0;
      n=0;
    }

    //initialization routines:
    template <class index_t, class scalar>
    matrix_base<index_t, scalar>::matrix_base(index_t m1, index_t n1) {
      m=m1;
      n=n1;
    }

    template <class index_t, class scalar>
    matrix_base<index_t, scalar>::matrix_base(matrix_base<index_t, scalar> *other) {
      other->dimensions(m, n);
    }

    //destructor:
    template <class index_t, class scalar>
    matrix_base<index_t, scalar>::~matrix_base() {
    }

    template <class index_t, class scalar>
    scalar matrix_base<index_t, scalar>::operator() (index_t i, index_t j) {
      return 0;
    }

    template <class index_t, class scalar>
    scalar *matrix_base<index_t, scalar>::operator() (index_t i) {
      //scalar *row;
      //row=new scalar[n];
      return NULL;
    }

    template <class index_t, class scalar>
    void matrix_base<index_t, scalar>::get_row(index_t i, scalar *row) {
    }

    template <class index_t, class scalar>
    long matrix_base<index_t, scalar>::cel (scalar val, index_t i, index_t j) {
      return i*m+j;
    }

    //multiply with any matrix (doesn't really matter--doesn't do anything anyway):
    template <class index_t, class scalar>
    matrix_base<index_t, scalar> * matrix_base<index_t, scalar>::mat_mult(matrix_base<index_t, scalar> *cand) {
      matrix_base<index_t, scalar> *result;
      index_t m1, n1;
      cand->dimensions(m1, n1);

      if (n!=m1) {
        fprintf(stderr, "Matrix inner dimensions must match ([%dx%d]*[%dx%d]\n", 
		    m, n, cand->m, cand->n);
        return NULL;
      }
      result=new matrix_base<index_t, scalar>(m, n1);
      return result;
    } 

    //matrix addition:
    template <class index_t, class scalar>
    matrix_base<index_t, scalar> * matrix_base<index_t, scalar>::add(matrix_base<index_t, scalar> *b) {
      matrix_base<index_t, scalar> *result;
      index_t m1, n1;
      b->dimensions(m1, n1);
      if (n!=n1 || m1!=m) {
        fprintf(stderr, "Matrix dimensions must match ([%dx%d]+[%dx%d]\n", 
		    m, n, b->m, b->n);
        return NULL;
      }
      result=new matrix_base<index_t, scalar>(m, n);
      return result;
    }

    //multiply with a vector:
    template <class index_t, class scalar>
    scalar * matrix_base<index_t, scalar>::vect_mult(scalar *cand) {
      //scalar *result;
      //result=new scalar[n];
      return NULL;
    } 

    template <class index_t, class scalar>
    scalar * matrix_base<index_t, scalar>::left_mult(scalar *cor) {
      //scalar *result;
      //result=new scalar[m];
      return NULL;
    } 

    //multiply with a vector:
    template <class index_t, class scalar>
    void matrix_base<index_t, scalar>::vect_mult(scalar *cand, scalar *result) {
    } 

    template <class index_t, class scalar>
    void matrix_base<index_t, scalar>::left_mult(scalar *cor, scalar *result) {
    } 

    //copy constructor:
    template <class index_t, class scalar>
    matrix_base<index_t, scalar>::matrix_base(matrix_base<index_t, scalar> &other) {
      m=other.m;
      n=other.n;
    }

    //type conversion:
    template <class index_t, class scalar>
    matrix_base<index_t, scalar> & matrix_base<index_t, scalar>::operator = 
		(full_matrix<index_t, scalar> &other) {
      other.dimensions(m, n);
      return *this;
    }

    //type conversion:
    template <class index_t, class scalar>
    matrix_base<index_t, scalar> & matrix_base<index_t, scalar>::operator = 
		(sparse<index_t, scalar> &other) {
      other.dimensions(m, n);
      return *this;
    }

    template <class index_t, class scalar>
    matrix_base<index_t, scalar> & matrix_base<index_t, scalar>::operator = 
  		(sparse_array<index_t, scalar> &other) {
      other.dimensions(m, n);
      return *this;
    }

    template <class index_t, class scalar>
    void matrix_base<index_t, scalar>::dimensions(index_t &mout, index_t &nout) const {
      mout=m;
      nout=n;
    }

    //should probably return a new matrix:
    template <class index_t, class scalar>
    void matrix_base<index_t, scalar>::transpose() {
      index_t sw;
      sw=n;
      n=m;
      m=sw;
    }

    //should probably return a new matrix:
    template <class index_t, class scalar>
    void matrix_base<index_t, scalar>::scal_mult(scalar cand) {
    }
	
    template <class index_t, class scalar>
    int matrix_base<index_t, scalar>::scan(FILE *fs) {
      char *line;
      int err=0;

      line=fget_line(fs);

      if (line==NULL) return FILE_READ_ERROR;
      if (sscanf(line, "%d %d\n", &m, &n)!=2) err=FILE_READ_ERROR;
      delete [] line;
 
      return 0;
    }

    template <class index_t, class scalar>
    size_t matrix_base<index_t, scalar>::read(FILE *fs, int mattype) {
      size_t nread=0;
      if (mattype==FULL_T) {
        nread=fread(&n, sizeof(m), 1, fs);
        fseek(fs, 0, SEEK_END);
        m=(ftell(fs)-sizeof(n))/sizeof(scalar)/n;
        fseek(fs, sizeof(n), SEEK_SET);
      } else if (mattype==SPARSE_ARRAY_T || mattype==SPARSE_T) {
        nread=fread(&m, sizeof(m), 1, fs);
        nread+=fread(&n, sizeof(n), 1, fs);
      } else {
        m=0; n=0;
      }

      return nread;
    }

    template <class index_t, class scalar>
    size_t matrix_base<index_t, scalar>::read(FILE *fs) {
      read(fs, SPARSE_T);
    }

    template <class index_t, class scalar>
    size_t matrix_base<index_t, scalar>::write(FILE *fs) {
      return 0;
    }

    template <class index_t, class scalar>
    void matrix_base<index_t, scalar>::print(FILE *fs) {
      fprintf(fs, "%d %d\n", m, n);
    }

    template <class index_t, class scalar>
    matrix_base<index_t, scalar> * matrix_base<index_t, scalar>::clone() {
      matrix_base<index_t, scalar> *result;
      result=new matrix_base<index_t, scalar>(m, n);
      return result;
    }

    template <class index_t, class scalar>
    scalar matrix_base<index_t, scalar>::norm() {
      return 0;
    }

    template <class index_t, class scalar>
    matrix_base<index_t, scalar> & matrix_base<index_t, scalar>::operator = (matrix_base<index_t, scalar> &other) {
      if (typeid(other) == typeid(sparse<index_t, scalar>)) {
        return *this=*((sparse<index_t, scalar> *) &other);
      } else if (typeid(other) == typeid(sparse_array<index_t, scalar>)) {
        return *this=*(sparse_array<index_t, scalar> *) &other;
      } else if (typeid(other) == typeid(full_matrix<index_t, scalar>)) {
        return *this=*(full_matrix<index_t, scalar> *) &other;
      }
    };

    //to avoid creating a "clone" method--the designers of C++ really should
    //add polymorphic copying to the standard...
    template <class index_t, class scalar>
    matrix_base<index_t, scalar> * copy_matrix(matrix_base<index_t, scalar> *other) {
      matrix_base<index_t, scalar> *result;
      if (typeid(*other) == typeid(sparse<index_t, scalar>)) {
        result=new sparse<index_t, scalar> ();
        *result=*other;
      } else if (typeid(*other) == typeid(sparse_array<index_t, scalar>)) {
        result=new sparse_array<index_t, scalar> ();
        *result=*other;
      } else if (typeid(*other) == typeid(full_matrix<index_t, scalar>)) {
        result=new full_matrix<index_t, scalar> ();
        *result=*other;
      } else {
        result=NULL;
      }

      return result;
    }

    template class matrix_base<int32_t, float>;
    template class matrix_base<int32_t, double>;

  } //end namespace libsparse
} //end namespace libpetey

