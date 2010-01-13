#ifndef __SCOMMON_H
#define __SCOMMON_H

#include <string>
#include <stdarg.h>
#include <atlbase.h>
#include <comdef.h>

#include <sstream>
#include <ostream>
#include <iomanip>

using std::string;
using std::wstring;
using std::istream;
using std::ostream;
using std::iostream;

typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned char uchar;


// cut out not more than maxCount chars that equal to passed one. If maxCount == -1 then cut out all
string trimLeft ( const string &, char = ' ', int maxCount = -1 );
string trimRight( const string &, char = ' ', int maxCount = -1 );
string trimBoth ( const string &, char = ' ', int maxCount = -1 );

const char * strnchr( const char * str, int chr, size_t maxLen );
//int strnlen( const char * str, size_t maxLen );  // returns -1 if len > maxLen

 /* Copy string src to buffer dest (of buffer size dest_size).  At most
 * dest_size-1 characters will be copied.  Always NUL terminates
 * (unless dest_size == 0).  This function does NOT allocate memory.
 * Unlike strncpy, this function doesn't pad dest (so it's often faster).
 * Returns size of attempted result, strlen(src),
 * so if retval >= dest_size, truncation occurred.
 */
size_t strlcpy (char       *dest,
                const char *src,
                size_t      dest_size);

/**
 * It is got from glib 2.0.
 *
 * @string: the return location for the newly-allocated string.
 * @format: a standard printf() format string, but notice
 *          <link linkend="string-precision">string precision pitfalls</link>.
 * @args: the list of arguments to insert in the output.
 *
 * This function allocates a 
 * string to hold the output, instead of putting the output in a buffer 
 * you allocate in advance.
 *
 * Returns: the number of bytes printed.
 **/
int 
vasprintf (char      **string,
           char const *format,
           va_list      args);

char *
strsep (char **stringp, const char *delim);

// convert windows error code to string
string sWinErrMsg (DWORD errorCode);

// formats string a la sprintf, max 10k size
string sFormat( string format, ... );

#define WSFORMAT(e) ((dynamic_cast<const std::wostringstream&>(std::wostringstream().flush() << e)).str())
#define SFORMAT(e) ((dynamic_cast<const std::ostringstream&>(std::ostringstream().flush() << e)).str())

string AmountFormat(double amt, int precision = 2);
string StripDotZeros (const string& s);
string RateFormat(double rate);	//2 or 4 fractional digits
string FixFormat(double lot, int precision = 1);	//0 or 1 fractional digit

string sFormatVa( const string & format, va_list list );

#define FORMAT_SYS_ERR(sysFun, sysErr) \
   (SFORMAT("When calling '" << sysFun << " (...)' the system error '" \
   << sWinErrMsg (sysErr) << "' has occured (#" << sysErr << ")."))

// throws std::logic_error, formating string first
__declspec(noreturn) void sThrow( const char * format, ... );

// load string with given id from resources. Maximum string length is 10k
string loadResourceStr( int id );

void checkHR( HRESULT );

BSTR toBSTR( const string & );
string fromBSTR( const WCHAR * );

wstring str2wstr( const string & );
//string wstr2str( const wstring & );


inline const char * ptr2ptr( const string & s )
{
  return s.c_str();
}

inline const char * sptr( const char * p )
{
  return p ? p : "";
}

inline const char * szptr( const char * p )
{
  return p && *p ? p : 0;
}


template<class T>
inline const char * ptr2sptr( const T & s )
{
  return sptr(ptr2ptr(s));
}

template<class T>
inline const char * ptr2szptr( const T & s )
{
  return szptr(ptr2ptr(s));
}

// append a string representation of an object
// to a string
template <class T>
void toString (const T& object, std::string & s)
{
  std::ostringstream os;
  os << object;
  s += os.str();
}


#define SMAKE_THROW_FN_DECL(name, XClass)  \
void name( const char * fmt, ... ); void name(const string& msg);

SMAKE_THROW_FN_DECL(sThrow,SException)

#define SMAKE_THROW_MEMBER_DECL(name, XClass)  \
static void name( const char * fmt, ... );


#define SMAKE_THROW_FN_IMPL(name, XClass)  \
  \
void name( const char * fmt, ... )  \
{  \
  va_list va;  \
  va_start(va, fmt);  \
  string msg(sFormatVa(fmt, va));  \
  va_end(va);  \
  throw XClass(msg);  \
}; void name(const string& msg) { throw XClass(msg); };

std::ostream& operator << (std::ostream& out, const _com_error &e);

bool operator == (const _com_error& a, const _com_error& b);

bool operator != (const _com_error& a, const _com_error& b);


//template<class X>
//SMAKE_THROW_FN_IMPL(sThrowX, X)


#endif  // __SCOMMON_H
