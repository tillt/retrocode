/*\
 * $Log: VOXFile.h,v $
 * Revision 1.7  2009/03/05 20:00:16  lobotomat
 * *** empty log message ***
 *
 * Revision 1.6  2008/09/04 15:54:43  lobotomat
 * minimized header to header inclusion
 * switched to safe integer types
 *
 * Revision 1.5  2007/12/19 18:32:19  lobotomat
 * changed naming convention for serializing methods
 *
 * Revision 1.4  2007/11/27 06:56:11  lobotomat
 * major restructuring
 * many little fixes here and there
 * CAUTION: very little testing done on this new version
 *
 * Revision 1.3  2007/11/08 09:02:24  lobotomat
 * fixed newly updated paths to work on unix systems
 *
 * Revision 1.2  2007/11/07 19:57:15  lobotomat
 * update include and lib paths to comply with new structure
 *
 * Revision 1.1.1.1  2007/11/07 13:59:19  lobotomat
 * initial checkin
 *
 * Revision 1.1  2006/12/19 06:18:52  cvsuser
 * no message
 *
 * Revision 1.1.1.1  2006/04/17 20:31:19  Lobotomat
 * no message
 *
\*/
#ifndef VOXFILEincluded
#define VOXFILEincluded
class CMFMSample;
class CVOXFile : public CMFMFile
{
public:
	DYNOBJECT(CVOXFile)
	DYNDEFPROPERTY

	CVOXFile(void);
	virtual ~CVOXFile(void);
	virtual void Read(istream &ar);
	virtual void Write(ostream &out);
	bool bMagicHead(std::istream &ar,uint32_t nSize);

protected:
	CMFMSample *m_pCurrentSample;
	vector<CMFMSample *> m_Samples;
};
#endif
