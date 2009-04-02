#if !defined(AFX_MIDIFILEDOC_H__9C0DC046_E28D_11D4_90DC_B433E57F1167__INCLUDED_)
#define AFX_MIDIFILEDOC_H__9C0DC046_E28D_11D4_90DC_B433E57F1167__INCLUDED_
class CMIDIFileDoc : public CMobileContent, public CMIDIFileLoader, public CMIDIFileWriter
{
public:
	DYNOBJECT(CMIDIFileDoc)
	DYNDEFPROPERTY

	CMIDIFileDoc();
	virtual ~CMIDIFileDoc();

	virtual void Read(std::istream &ar);
	virtual void Write(std::ostream &ar);

	virtual bool Load(const char *szPath);
	virtual bool Save(const char *szFile);
};

#endif
