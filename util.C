#ifdef MAIN
#include <stdio.h>
#include <petscvec.h>
#endif

PetscScalar dotScalar(PetscInt d, const PetscScalar x[], const PetscScalar y[]);

class BlockIt {
  public:
  BlockIt(int d, const int *dim) : d(d) {
    int s=1;
    this->dim = new int[d]; stride = new int[d]; ind = new int[d];
    for (int j=d-1; j>=0; j--) {
      this->dim[j] = dim[j];
      ind[j] = 0;
      stride[j] = s;
      s *= dim[j];
    }
    i = 0;
    done = s == 0;
  }
  BlockIt(const BlockIt &it) : done(it.done), i(it.i), d(it.d) {
    dim = new int[d]; stride = new int[d]; ind = new int[d];
    for (int i=0; i < d; i++) {
      dim[i]    = it.dim[i];
      stride[i] = it.stride[i];
      ind[i]    = it.ind[i];
    }
  }
  ~BlockIt() {
    delete [] dim;
    delete [] stride;
    delete [] ind;
  }
  void next() {
    int carry = 1;
    i = 0;
    for (int j = d-1; j >= 0; j--) {
      ind[j] += carry;
      carry = 0;
      if (ind[j] == dim[j]) {
        ind[j] = 0;
        carry = 1;
      }
      i += ind[j] * stride[j];
    }
    done = (bool)carry;
  }
  int shift(int j, int s) const {
    const int is = ind[j] + s;
    if (is < 0 || is >= dim[j]) return -1;
    return i + s * stride[j];
  }
  int shift2(int j0, int s0, int j1, int s1) const {
    const int is0 = ind[j0] + s0;
    if (is0 < 0 || is0 >= dim[j0]) return -1;
    const int is1 = ind[j1] + s1 + (j0 == j1) ? s0 : 0;
    if (is1 < 0 || is1 >= dim[j1]) return -1;
    return i + s0 * stride[j0] + s1 * stride[j1];
  }
  int plus(int *offset) const {
    int I = 0;
    for (int j=d-1; j >= 0; j--) {
      const PetscInt k = ind[j] + offset[j];
      if (k < 0 || k >= dim[j]) return -1;
      I += k * stride[j];
    }
    return I;
  }
  bool normal(PetscReal *n) const {
    PetscReal n2;
    for (int j=0; j < d; j++) { // Compute normal vector, it 'seems' backwards because of the Chebyshev ordering.
      if      (ind[j] == 0)          { n[j] =  1.0; }
      else if (ind[j] == dim[j] - 1) { n[j] = -1.0; }
      else                           { n[j] =  0.0; }
    }
    n2 = dotScalar(d, n, n);
    if (n2 > 1e-10) {
      for (int j=0; j < d; j++) n[j] /= sqrt(n2); // normalize n
    }
    return (n2 > 1e-10);
  }
  int numDims() const { return d; }
  bool done;
  int i, *ind;
  private:
  int d, *dim, *stride;
};

PetscScalar dotScalar(PetscInt d, const PetscScalar x[], const PetscScalar y[]) {
  PetscScalar dot = 0.0;
  for (PetscInt i=0; i<d; i++) {
    dot += x[i] * y[i];
  }
  return dot;
}

PetscInt indexMaxAbs(PetscInt d, const PetscScalar x[]) {
  PetscInt j=0;
  PetscReal max=0.0;
  for (int i=0; i < d; i++) {
    const PetscReal tmp = PetscAbs(x[i]);
    if (max < tmp) {
      max = tmp;
      j = i;
    }
  }
  return j;
}

int sumInt(int d, const int dim[]) {
  int z = 0;
  for (int i=0; i<d; i++) z += dim[i];
  return z;
}

int productInt(int d, const int dim[]) {
  int z = 1;
  for (int i=0; i<d; i++) z *= dim[i];
  return z;
}

void zeroInt(int d, int v[]) {
  for (int i=0; i < d; i++) v[i] = 0;
}

#undef __FUNCT__
#define __FUNCT__ "polyInterp"
PetscErrorCode polyInterp(const PetscInt n, const PetscReal *x, PetscScalar *w, const PetscReal x0, const PetscReal x1, PetscScalar *f0, PetscScalar *f1)
{
  PetscInt o=0, e;

  PetscFunctionBegin;
  for (int di=1; di < n; di++) { // offset (column of table)
    o = di % 2; e = (o+1) % 2;
    for (int i=0; i < n-di; i++) { // nodes
      w[i*4+2*o]   = ((x0-x[i+di])*w[i*4+2*e  ] + (x[i]-x0)*w[(i+1)*4+2*e  ]) / (x[i] - x[i+di]);
      w[i*4+2*o+1] = ((x1-x[i+di])*w[i*4+2*e+1] + (x[i]-x1)*w[(i+1)*4+2*e+1]) / (x[i] - x[i+di]);
    }
  }
  *f0 = w[2*o];
  *f1 = w[2*o+1];
  PetscFunctionReturn(0);
}

#ifdef MAIN
#define N 20
PetscScalar func(PetscReal x) {
  //return pow(x,6) + 3.1*pow(x,4) + 2.7*pow(x,3);
  return cos(x);
}

#undef __FUNCT__
#define __FUNCT__ "main"
int main(int argc, char *argv[])
{
  PetscReal x[N], w[N*4], x0, x1, f0, f1;
  PetscErrorCode ierr;

  for (int order = 2; order < N; order++) {
    for (int i=0; i < order; i++) {
      x[i] = 1.0 + 1.0 * i;
      w[i*4] = func(x[i]);
      w[i*4+1] = w[i*4];
    }
    x0 = 1.43; x1 = 3.1;
    ierr = polyInterp(order, x, w, x0, x1, &f0, &f1);CHKERRQ(ierr);
    printf("[%d], %f ~ %f    %f ~ %f\n", order, f0, func(x0), f1, func(x1));
  }
  return 0;
}

#endif
