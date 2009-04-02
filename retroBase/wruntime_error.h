#ifndef WRUNTIMEincluded
#define WRUNTIMEincluded
#include "Basics.h"
#include "MyString.h"
#include <string>
#include <stdexcept>

using namespace std;

#ifndef RETROBASE_EXPORTS
#undef DllExport
#define DllExport 
#else
#undef DllExport
#define DllExport   __declspec( dllexport )
#endif

#ifdef WIN32
#ifdef _UNICODE
    #define truntime_error wruntime_error
#else 
    #define truntime_error std::runtime_error
#endif
#else
    #define truntime_error std::runtime_error
#endif
/*
#ifdef _UNICODE
class wruntime_error : public std::runtime_error
{

public:                 // --- PUBLIC INTERFACE ---
    DllExport wruntime_error( const std::wstring& errorMsg ) ;
	DllExport wruntime_error( const wruntime_error& rhs ) ;
    DllExport wruntime_error&     operator=( const wruntime_error& rhs ) ;
    DllExport virtual ~wruntime_error() ;

    DllExport const std::wstring& errorMsg() const ;

private:                // --- DATA MEMBERS ---
    std::wstring        mErrorMsg ; ///< Exception error message.
    
};

#else
*/
class assert_error : public truntime_error
{
public:                 // --- PUBLIC INTERFACE ---
    DllExport assert_error( const string &fileName, const int nLine, const string &functionName,const string &errorMsg );
	DllExport assert_error( const assert_error &rhs);
    DllExport assert_error& operator=( const assert_error& rhs );
	DllExport virtual ~assert_error() throw();

	DllExport const string &errorMsg() const ;
	DllExport const string &fileName() const ;
	DllExport const string &functionName() const ;
	DllExport const int &nLineNumber() const ;

private:                // --- DATA MEMBERS ---
    tstring        mErrorMsg ; ///< Exception error message.
	tstring        mFunctionName; 
	tstring        mFileName; 
	int			   nLine; 
    
};
//#endif

#endif	//WRUNTIMEincluded
