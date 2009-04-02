#include "stdafx.h"

#include <iostream>
#include <string>

#include "wruntime_error.h"

#ifdef WIN32
#ifdef _UNICODE


wruntime_error::wruntime_error( const wstring &errorMsg ) : runtime_error(toNarrowString(errorMsg)), mErrorMsg(errorMsg)
{
    // NOTE: We give the runtime_error base the narrow version of the 
    //  error message. This is what will get shown if what() is called.
    //  The wruntime_error inserter or errorMsg() should be used to get 
    //  the wide version.
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */

wruntime_error::wruntime_error( const wruntime_error& rhs ) : runtime_error(toNarrowString(rhs.errorMsg())) , mErrorMsg(rhs.errorMsg())
{
}

wruntime_error& wruntime_error::operator=( const wruntime_error& rhs )
{
    // copy the wruntime_error
    runtime_error::operator=( rhs ) ; 
    mErrorMsg = rhs.mErrorMsg ; 
    return *this ; 
}

wruntime_error::~wruntime_error()
{
}

const wstring& wruntime_error::errorMsg() const 
{ 
	return mErrorMsg ; 
}
#endif
#endif

#ifndef UNICODE
assert_error::assert_error(const tstring &fileName, const int nLine, const tstring &functionName,const tstring &errorMsg ) : runtime_error(errorMsg), mErrorMsg(errorMsg),mFunctionName(functionName),mFileName(fileName),nLine(nLine)
{
    // NOTE: We give the runtime_error base the narrow version of the 
    //  error message. This is what will get shown if what() is called.
    //  The wruntime_error inserter or errorMsg() should be used to get 
    //  the wide version.
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */

assert_error::assert_error( const assert_error& rhs ) : runtime_error(rhs.errorMsg()) , mErrorMsg(rhs.errorMsg())
{
}

assert_error::~assert_error() throw()
{ 
}

assert_error& assert_error::operator=( const assert_error& rhs )
{
    // copy the wruntime_error
    runtime_error::operator=( rhs ) ; 
    mErrorMsg = rhs.mErrorMsg ; 
	mFunctionName=rhs.mFunctionName;
	mFileName=rhs.mFileName;
	nLine=rhs.nLine;
    return *this ; 
}


const tstring &assert_error::errorMsg() const 
{ 
	return mErrorMsg ; 
}

const tstring &assert_error::fileName() const 
{ 
	return mFileName; 
}

const tstring &assert_error::functionName() const 
{ 
	return mFunctionName; 
}

const int &assert_error::nLineNumber() const
{
	return nLine;
}
#endif
