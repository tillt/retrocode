#ifndef MOTOROLAFILEincluded
#define MOTOROLAFILEincluded
class CMotorolaProperty;
class CMotorolaFile : public CMonoContent
{
public:
	DYNOBJECT(CMotorolaFile)
	DYNDEFPROPERTY

	CMotorolaFile(void);
	virtual ~CMotorolaFile(void);
	
	void Read(istream &ar);

protected:
	bool Decode(void);
};
#endif
