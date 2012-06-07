// CkObject.h: interface for the CkObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CKOBJECT_H__B4D6C6A9_5D53_4205_A809_AE3103E7DB5E__INCLUDED_)
#define AFX_CKOBJECT_H__B4D6C6A9_5D53_4205_A809_AE3103E7DB5E__INCLUDED_


#ifndef __sun__
#pragma pack (push, 8)
#endif
 

class CkObject  
{
public:
	CkObject();
	virtual ~CkObject();

#ifdef CK_COCOA
	// Scan sUtf8 and make sure it is a valid utf-8 string
	// that is assured of being handled correctly by 
	// NSString's stringWithUTF8String constructor.
	const char *objcUtf8(const char *sUtf8);
	void objcUtf8_free(void);

    private:
	char *m_utf8Str;
#endif

};

#ifndef __sun__
#pragma pack (pop)
#endif


#endif // !defined(AFX_CKOBJECT_H__B4D6C6A9_5D53_4205_A809_AE3103E7DB5E__INCLUDED_)
