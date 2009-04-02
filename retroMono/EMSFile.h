#ifndef EMSFILEincluded
#define EMSFILEincluded
class CEMSProperty;
class CEMSFile : public CMonoContent
{
public:
	DYNOBJECT(CEMSFile)
	DYNDEFPROPERTY

	CEMSFile(void);
	virtual ~CEMSFile(void);
	
	void Read(istream &ar);

	bool bMagicHead(std::istream &ar,uint32_t nSize);
	
	tstring sGetName(void) {return m_strName;};
	bool bUsesIMelodyHead(void) {return m_bIMelodyHead;};
	bool bUsesTempo(void) {return m_bUsingTempo;};
	bool bUsesName(void) {return m_bUsingName;};

protected:
	tstring m_strName;
	
	bool m_bIMelodyHead;
	bool m_bEMSHead;
	bool m_bUsingTempo;
	bool m_bUsingName;

	bool Decode();
};
#endif
