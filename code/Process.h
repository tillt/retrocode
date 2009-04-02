class CMobileSampleContent;

void ConvertTo16Bit(CMobileSampleContent *pSource);
void Normalize(CMobileSampleContent *pSource);
void RmsNormalize(CMobileSampleContent *pSource,converting_parameters *parm);
void ConvertChannels(CMobileSampleContent *pSource,converting_parameters *parm);
void ConvertSampleRate(CMobileSampleContent *pSource,converting_parameters *parm);
void GainControl(CMobileSampleContent *pSource,converting_parameters *parm);
void Crop(CMobileSampleContent *pSource,converting_parameters *parm);
void FadeIn(CMobileSampleContent *pSource,converting_parameters *parm);
void FadeOut(CMobileSampleContent *pSource,converting_parameters *parm);
void AutoAdapt(CMobileSampleContent *pDest,CMobileSampleContent *pSource,converting_parameters *parm);
void ButterworthFilter(CMobileSampleContent *pSource,converting_parameters *parm);
void Loop(CMobileSampleContent *pSource,converting_parameters *parm);
void RingbackMixer(CMobileSampleContent *pSource,converting_parameters *parm);