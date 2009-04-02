#ifndef RAWFILEincluded
#define RAWFILEincluded
class CRAWFile : public CMobileSampleContent
{
public:
	DYNOBJECT(CRAWFile)
	DYNDEFPROPERTY

	CRAWFile(void);
	virtual ~CRAWFile(void);

	virtual void Read(istream &ar);
	virtual void Write(ostream &out);
protected:
};
#endif
