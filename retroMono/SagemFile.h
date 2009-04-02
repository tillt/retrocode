/*\
 * $Log: SagemFile.h,v $
 * Revision 1.4  2009/03/05 19:55:50  lobotomat
 * *** empty log message ***
 *
 * Revision 1.3  2009/01/14 10:24:02  lobotomat
 * cleaned includes
 *
 * Revision 1.2  2008/08/08 23:04:11  lobotomat
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2007/11/27 06:35:49  lobotomat
 * initial check in
 *
 * Revision 1.1  2006/12/19 08:49:25  cvsuser
 * no message
 *
 * Revision 1.1.1.1  2006/04/17 20:31:40  Lobotomat
 * no message
 *
 * Revision 1.1  2004/06/04 03:28:01  Lobotomat
 * added EMS, Nokia, Sagem and Motorola mono ringtone formats
 * added SMAF max sample duration attribute
 *
\*/
#ifndef SAGEMFILEincluded
#define SAGEMFILEincluded

class CSagemProperty;

class CSagemFile : public CMonoContent
{
public:
	DYNOBJECT(CSagemFile)
	DYNDEFPROPERTY

	CSagemFile(void);
	virtual ~CSagemFile(void);

	int nGetLoopDelay(){return m_nLoopDelay;};
	tstring sGetName(){return m_strName;};
	
	void Read(istream &ar);

protected:
	int m_nLoopDelay;
	tstring m_strName;

	bool Decode(void);
};
#endif
