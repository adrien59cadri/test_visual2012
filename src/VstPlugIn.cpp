#include "VstPlugIn.h"
#include <assert.h>
#include <string>


#include <fftw3.h>

void process_fft(double *in, fftw_complex *out, int framesnb)
{



    fftw_complex *intmp, *outtmp;
    fftw_plan p;
         
    intmp = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * framesnb);
    outtmp = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * framesnb);
    p = fftw_plan_dft_1d(framesnb, intmp, outtmp, FFTW_FORWARD, FFTW_ESTIMATE);
         
    for(int i=0;i<framesnb;i++)
        intmp[i][0] = in[i];

    fftw_execute(p); /* repeat as needed */
         
    memcpy(out,outtmp,sizeof(fftw_complex) * framesnb);

    fftw_destroy_plan(p);
    fftw_free(in); fftw_free(out);
}

void process_fft(float *in, float **out, int framesnb)
{
    
    double *ind = new double[framesnb];
    fftw_complex*outd = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * framesnb);

    
    for(int i=0;i<framesnb;i++)
        ind[i]= in[i];

    process_fft(ind, outd, framesnb);

    
    for(int i=0;i<framesnb;i++)
    {
        out[0][i]= outd[i][0];
        out[1][i]= outd[i][1];
    }
}

AudioEffect* createEffectInstance (audioMasterCallback audioMaster){
	VstPlugIn * ptr = new VstPlugIn(audioMaster);
	return ptr;
}



void VstPlugIn::processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames)  ///< Process 32 bit (single precision) floats (always in a resume state)
{
	//for (auto i = 0; i < sampleFrames;i++)
	//{
	//	process_fft(*inputs,outputs,sampleFrames);
	//}
}

VstPlugIn::VstPlugIn (audioMasterCallback audioMaster)
: AudioEffectX (audioMaster, 1, 1)	// 1 program, 1 parameter only
{
	setNumInputs (2);		// stereo in
	setNumOutputs (2);		// stereo out
	setUniqueID ('test');	// identify
	canProcessReplacing (true);	// supports replacing output
	canDoubleReplacing (false);	// supports double precision processing

	InitParams();
	vst_strncpy (programName, "Default", kVstMaxProgNameLen);	// default program name
}


void VstPlugIn::InitParams()
{
	memset(mParams,0,sizeof(mParams));
	for (int i=0;i<kParamNb;i++)
	{
		mParamLabels[i] = new char[kVstMaxParamStrLen];
		mParamNames[i] = new char[kVstMaxParamStrLen];
	}
}
void VstPlugIn::CleanParams()
{
	for (int i=0;i<kParamNb;i++)
	{
		delete [] mParamLabels[i];
		delete [] mParamNames[i];
	}
}

//-------------------------------------------------------------------------------------------------------
VstPlugIn::~VstPlugIn ()
{
	CleanParams();
	// nothing to do here
}

//-------------------------------------------------------------------------------------------------------
void VstPlugIn::setProgramName (char* name)
{
	vst_strncpy (programName, name, kVstMaxProgNameLen);
}

//-----------------------------------------------------------------------------------------
void VstPlugIn::getProgramName (char* name)
{
	vst_strncpy (name, programName, kVstMaxProgNameLen);
}

//-----------------------------------------------------------------------------------------
void VstPlugIn::setParameter (VstInt32 index, float value)
{
	assert(index<kParamNb);
	mParams[index] = value;
}

//-----------------------------------------------------------------------------------------
float VstPlugIn::getParameter (VstInt32 index)
{
	
	assert(index<kParamNb);
	return mParams[index];
}

//-----------------------------------------------------------------------------------------
void VstPlugIn::getParameterName (VstInt32 index, char* label)
{

	vst_strncpy (label, mParamNames[index], kVstMaxParamStrLen);
}

//-----------------------------------------------------------------------------------------
void VstPlugIn::getParameterDisplay (VstInt32 index, char* text)
{
	float2string (mParams[index], text, kVstMaxParamStrLen);
}

//-----------------------------------------------------------------------------------------
void VstPlugIn::getParameterLabel (VstInt32 index, char* label)
{
	vst_strncpy (label,mParamLabels[index], kVstMaxParamStrLen);
}

//------------------------------------------------------------------------
bool VstPlugIn::getEffectName (char* name)
{
	vst_strncpy (name, "myplug", kVstMaxEffectNameLen);
	return true;
}

//------------------------------------------------------------------------
bool VstPlugIn::getProductString (char* text)
{
	vst_strncpy (text, "fx", kVstMaxProductStrLen);
	return true;
}

//------------------------------------------------------------------------
bool VstPlugIn::getVendorString (char* text)
{
	vst_strncpy (text, "Dzada", kVstMaxVendorStrLen);
	return true;
}

//-----------------------------------------------------------------------------------------
VstInt32 VstPlugIn::getVendorVersion ()
{ 
	return 1000; 
}

//-----------------------------------------------------------------------------------------
//void VstPlugIn::processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames)
