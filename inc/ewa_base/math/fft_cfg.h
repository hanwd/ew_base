#include "ewa_base/math/tiny_cpx.h"

EW_ENTER


void fft(tiny_cpx<double> *dst,const double *src,size_t n,bool inverse=false);
void fft(tiny_cpx<double> *dst,const tiny_cpx<double> *src,size_t n,bool inverse=false);

EW_LEAVE

