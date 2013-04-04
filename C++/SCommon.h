#ifndef SCOMMON_H_
#define SCOMMON_H_

#include <string>
#include <stdarg.h>
#ifdef _WIN32
#include <atlbase.h>
#else
#include <errno.h>
#define __declspec(x)
#define __stdcall
#define INVALID_SOCKET (-1)
#define BOOL bool
#define SOCKET int
#endif

#include <sstream>
#include <ostream>
#include <iomanip>
#include <string.h>

//#define WIN32_LEAN_AND_MEAN 
//#include <windows.h>


#ifdef _WIN32
#  define _T L
#else
#  define _T
#endif

typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned char uchar;


// cut out not more than maxCount chars that equal to passed one. If maxCount == -1 then cut out all
std::string trimLeft ( const std::string &, char = ' ', int maxCount = -1 );
std::string trimRight( const std::string &, char = ' ', int maxCount = -1 );
std::string trimBoth ( const std::string &, char = ' ', int maxCount = -1 );

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

/* It is got from glib 2.0.
 *
 * Appends string src to buffer dest (of buffer size dest_size).
 * At most dest_size-1 characters will be copied.
 * Unlike strncat, dest_size is the full size of dest, not the space left over.
 * This function does NOT allocate memory.
 * This always NUL terminates (unless siz == 0 or there were no NUL characters
 * in the dest_size characters of dest to start with).
 * Returns size of attempted result, which is
 * MIN (dest_size, strlen (original dest)) + strlen (src),
 * so if retval >= dest_size, truncation occurred.
 */
size_t
strlcat (char       *dest,
         const char *src,
         size_t      dest_size);

#ifdef _WIN32
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
#endif

char *
strsep (char **stringp, const char *delim);

#ifdef _WIN32
// convert windows error code to string
std::wstring sWinErrMsg (DWORD errorCode);
#endif

// formats string a la sprintf, max 10k size
std::wstring sFormat( std::wstring format, ... );

#define WSFORMAT(e) ((dynamic_cast<const std::wostringstream&>(std::wostringstream().flush() << e)).str())
#ifndef UNICODE
#define SFORMAT(e) ((dynamic_cast<const std::ostringstream&>(std::ostringstream().flush() << e)).str())
#else
#define SFORMAT WSFORMAT
#endif

std::string AmountFormat(double amt, int precision = 2);
std::string StripDotZeros (const std::string& s);
std::string RateFormat(double rate);	//2 or 4 fractional digits
std::string FixFormat(double lot, int precision = 1);	//0 or 1 fractional digit

std::wstring sFormatVa( const std::wstring & format, va_list list );

// ansi version
std::string sFormatVaA( const std::string & format, va_list list );

//string uni2ascii (const std::wstring & str);
//std::wstring ascii2uni (const string & str);

#define FORMAT_SYS_ERR(sysFun, sysErr) \
   (SFORMAT("When calling '" << sysFun << " (...)' the system error '" \
   << sWinErrMsg (sysErr) << "' has occured (#" << sysErr << ")."))

// throws std::logic_error, formating string first
__declspec(noreturn) void sThrow( const wchar_t * format, ... );

// load string with given id from resources. Maximum string length is 10k
std::string loadResourceStr( int id );

#ifdef _WIN32
void checkHR( HRESULT );
#endif

std::wstring str2wstr( const std::string & );
std::string wstr2str( const std::wstring & );

std::string toUTF8 (const std::wstring&);
std::wstring fromUTF8 (const std::string&);

inline const char * ptr2ptr( const std::string & s )
{
  return s.c_str();
}

inline const wchar_t * ptr2ptr( const std::wstring & s )
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

template <class T>
std::string&& toString (const T& object)
{
  std::ostringstream os;
  os << object;
  return std::move(os.str());
}

// FIXME raise exception when the string is not a number
template <class T>
T fromString (const std::string& s)
{
  T object;
  std::istringstream is (s);
  is >> object;
  return object;
}

#define SMAKE_THROW_FN_DECL(name, XClass)  \
void name( const wchar_t * fmt, ... ); void name(const std::wstring& msg); 
//void name( const char * fmt, ... ); void name(const std::string& msg); 

SMAKE_THROW_FN_DECL(sThrow,SException)

#define SMAKE_THROW_MEMBER_DECL(name, XClass)  \
static void name( const wchar_t * fmt, ... );
//static void name( const char * fmt, ... ); 


#define SMAKE_THROW_FN_IMPL(name, XClass)  \
  \
void name( const wchar_t * fmt, ... )  \
{  \
  va_list va;  \
  va_start(va, fmt);  \
  std::wstring msg(sFormatVa(fmt, va));  \
  va_end(va);  \
  throw XClass(msg);  \
}; void name(const std::wstring& msg) { throw XClass(msg); };

//template<class X>
//SMAKE_THROW_FN_IMPL(sThrowX, X)

#ifdef _WIN32
FILETIME TimetToFileTime (time_t t);
time_t FileTimeToTimet (FILETIME ft);
#endif

// copy if (see Stroustrup 3rd ed, 18.6.1)

template<class In, class Out, class Pred>
Out copy_if (In first, In last, Out res, Pred p)
{
  while (first != last)
  {
    if (p (*first))
      *res++ = *first;
    ++first;
  }
  return res;
}

/**
 * Allocate a new char* string by the std::string arg.
 */
char* string2char_ptr (const std::string& str);

#endif
