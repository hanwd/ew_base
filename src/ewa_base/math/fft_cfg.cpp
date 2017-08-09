#include "ewa_base/math/fft_cfg.h"
#include "ewa_base/basic/system.h"
#include "ewa_base/basic/object_info.h"

EW_ENTER

// FFTW_BACKWARD +1
// FFTW_FORWARD -1
// FFTW_ESTIMATE 1<<6
// FFTW_UNALIGNED 1<<1
// FFTW_CONSERVE_MEMORY 1<<2


class ModuleFFtw : public DllModule, private ObjectInfo
{
public:

	ModuleFFtw(){}

	typedef void* fftw_plan;
	fftw_plan(*fftw_plan_dft_1d)(int n, const tiny_cpx<double>* src, tiny_cpx<double>* dst, int sign, unsigned flags);
	void(*fftw_execute_dft)(fftw_plan, const tiny_cpx<double>* src, tiny_cpx<double>* dst);
	void(*fftw_destroy_plan)(fftw_plan);

	bool enusre_inited()
	{
		if (!flags.get(FLAG_INITAL_OPEN)) return impl.ok();

		if (!Open("libfftw3-3.dll", FLAG_ON_SYMBOL_NOT_FOUND_CLOSE | FLAG_SEARCH_PATHS))
		{
			return false;
		}

		GetSymbolEx("fftw_plan_dft_1d", fftw_plan_dft_1d);
		GetSymbolEx("fftw_execute_dft", fftw_execute_dft);
		GetSymbolEx("fftw_destroy_plan", fftw_destroy_plan);

		return impl.ok();
	}

	virtual void DoInvoke(InvokeParam& ctx)
	{
		if (ctx.type==InvokeParam::TYPE_INIT)
		{
			LockGuard2 locker(dll_fttw.mutex);
			if (locker)
			{
				enusre_inited();
			}	
		}
		else if (ctx.type == InvokeParam::TYPE_TRY_RELEASE_MEM||ctx.type == InvokeParam::TYPE_FINI)
		{
			LockGuard2 locker(dll_fttw.mutex);
			if (locker && impl.ok())
			{
				Close();
				flags.clr(FLAG_INITAL_OPEN);
			}	
		}
	}

	AtomicSpin mutex;

}dll_fttw;




void fft(tiny_cpx<double> *dst, const tiny_cpx<double> *src, size_t n, bool inverse)
{
	LockGuard<AtomicSpin> locker(dll_fttw.mutex);

	if (!dll_fttw.enusre_inited())
	{
		Exception::XError("fftw_dll is not loaded");
	}

	ModuleFFtw::fftw_plan _plan;
	_plan = dll_fttw.fftw_plan_dft_1d(n, &src[0], &dst[0], inverse ? -1 : +1, 0x86);

	dll_fttw.fftw_execute_dft(_plan, src, dst);
	dll_fttw.fftw_destroy_plan(_plan);

}

void fft(tiny_cpx<double> *dst, const double *src, size_t n, bool inverse)
{
    arr_1t< tiny_cpx<double> > csrc(n);
    for(size_t i=0; i<n; i++)
    {
        csrc[i]=src[i];
    }
    fft( dst, &csrc[0],n,inverse );
}

EW_LEAVE
