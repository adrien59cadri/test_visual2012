#include "VstPlugIn.h"
#include <assert.h>
#include <string>

AudioEffect* createEffectInstance (audioMasterCallback audioMaster){
	VstPlugIn * ptr = new VstPlugIn(audioMaster);
	return ptr;
}



void VstPlugIn::processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames)  ///< Process 32 bit (single precision) floats (always in a resume state)
{
	for (auto i = 0; i < sampleFrames;i++)
	{
		
	}
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
	vst_strncpy (text, "Gain", kVstMaxProductStrLen);
	return true;
}

//------------------------------------------------------------------------
bool VstPlugIn::getVendorString (char* text)
{
	vst_strncpy (text, "Steinberg Media Technologies", kVstMaxVendorStrLen);
	return true;
}

//-----------------------------------------------------------------------------------------
VstInt32 VstPlugIn::getVendorVersion ()
{ 
	return 1000; 
}

//-----------------------------------------------------------------------------------------
//void VstPlugIn::processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames)
