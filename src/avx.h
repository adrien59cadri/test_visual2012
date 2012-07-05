#include <Windows.h>
#include "WinBase.h"
#include <immintrin.h>
#include <chrono>
#include <memory>
#include <iostream>
#include <assert.h>

uint64_t getHighResolutionTimeCounter(){
	LARGE_INTEGER t1;
	BOOL test = QueryPerformanceCounter(&t1);
	assert(test);
	return t1.QuadPart;

}

double getTime(uint64_t t)
{
	LARGE_INTEGER f;
	BOOL test = QueryPerformanceCounter(&f);
	assert(test);
	return static_cast<double>(t)/static_cast<double>(f.QuadPart);

}


void foo(){
	auto len =  8 * 1024;
	auto size = sizeof (float) * len;
	auto v1sse2 =  static_cast<__m128*>(_mm_malloc(size,32));
	auto v2sse2 =  static_cast<__m128*>(_mm_malloc(size,32));
	auto v1float = reinterpret_cast<float*>(v1sse2);
	auto v2float = reinterpret_cast<float*>(v2sse2);
	auto v1avx = reinterpret_cast<__m256*>(v1sse2);
	auto v2avx = reinterpret_cast<__m256*>(v2sse2);
	auto ressse2 =  static_cast<__m128*>(_mm_malloc(size,32));
	auto resavx =  static_cast<__m256*>(_mm_malloc(size,32));
	auto resfloat =  static_cast<float*>(_mm_malloc(size,32));


	auto t0 = getHighResolutionTimeCounter();
    
	for(auto i=0;i<len;i++)
	{
		resfloat[i] = v1float[i]+v2float[i];
	}
    
	auto t1 = getHighResolutionTimeCounter();
	for(auto i=0;i<len/4;i++)
	{
		ressse2[i] = _mm_add_ps(v1sse2[i],v2sse2[i]);
	}

	
	
	auto t2 = getHighResolutionTimeCounter();
 //   BOOL WINAPI IsProcessorFeaturePresent();

	//for(auto i=0;i<len/8;i++)
	//{

	//	//si avx dispo
	//	resavx[i] = _mm256_add_ps(v1avx[i],v2avx[i]);
	//}
	//
	//auto t3 = getHighResolutionTimeCounter();
	std::cout<<"time float : "<<getTime(t1-t0)<<"time sse2 : "<<getTime(t2-t1)/*<<"time avx : "<<getTime(t3-t2)*/<<std::endl;

	_mm_free(ressse2);
	_mm_free(v1sse2);
	_mm_free(v2sse2);
	_mm_free(resavx);
	_mm_free(resfloat);
}