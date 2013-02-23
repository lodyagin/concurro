#include "StdAfx.h"
#include "SCommon.h"
#ifdef _WIN32
#include <atlstr.h>
#endif

//using namespace std;

#if 0
string AmountFormat(double amt, int precision/* = 2*/) {
	if (amt < 0)
		return "-" + AmountFormat(-amt, precision);

	string s = SFORMAT (fixed << setprecision(precision) << amt);	//format the number
	string::size_type pt = s.find(".");	//find decimal point
	if (pt == string::npos)
		pt = s.length();						//or end-of-line

	while (pt > 3) {	//insert some commas
		pt -= 3; //skip 3 digits left
		s = s.substr(0, pt) + "," + s.substr(pt);	// insert comma
	}
	return s;
}

string StripDotZeros (const string& s0) {
	string s = s0;
	while (!s.empty() && s[s.length()-1] == '0')	//strip zero or more 0s
		s = s.substr(0,s.length()-1);
	if (!s.empty() && s[s.length()-1] == '.')		//strip single . from the end
		s = s.substr(0,s.length()-1);
	return s;
}

string RateFormat(double rate) {
	string s = SFORMAT (fixed << setprecision(4) << rate);
	return (s.length() > 6) ? s.substr(0,6) : s;
}

string FixFormat(double lot, int precision) {
	if (lot == (int) lot)
		return SFORMAT(lot);
	else
		return SFORMAT(fixed<<setprecision(precision)<<lot);
}
#endif

std::string trimLeft( const std::string & s, char ch, int maxCount )
{
  const char * b = ptr2ptr(s);
  const char * p = b;
  while ( *p != 0 && *p == ch && (maxCount == -1 || p - b < maxCount ) ) ++p;
  return std::string(p);
}

std::string trimRight( const std::string & s, char ch, int maxCount )
{
  const char * b = ptr2ptr(s);
  const char * e = strchr(b, 0);
  const char * p = e - 1;
  while ( p > b && *p == ch && (maxCount == -1 || e - p + 1 < maxCount ) ) --p;
  return std::string(b, p - b + 1);
}

std::string trimBoth( const std::string & s, char ch, int maxCount )
{
  return trimLeft(trimRight(s, ch, maxCount), ch, maxCount);
}

const char * strnchr( const char * str, int chr, size_t maxLen )
{
  for ( const char * p = str; p - str < int(maxLen) && *p; ++p )
    if ( *p == chr ) return p;
  return 0;
}

/*int strnlen( const char * str, size_t maxLen )
{
  for ( const char * p = str; p - str < int(maxLen); ++p )
    if ( *p == '\0' ) return p - str;
  return -1;
}*/

/* It is got from glib 2.0.
 *
 * Copy string src to buffer dest (of buffer size dest_size).  At most
 * dest_size-1 characters will be copied.  Always NUL terminates
 * (unless dest_size == 0).  This function does NOT allocate memory.
 * Unlike strncpy, this function doesn't pad dest (so it's often faster).
 * Returns size of attempted result, strlen(src),
 * so if retval >= dest_size, truncation occurred.
 */
size_t
strlcpy (char       *dest,
         const char *src,
         size_t      dest_size)
{
  register char *d = dest;
  register const char *s = src;
  register size_t n = dest_size;
  
  if (dest == NULL || src == NULL)
    return 0;
  
  /* Copy as many bytes as will fit */
  if (n != 0 && --n != 0)
    do
      {
        register char c = *s++;
        
        *d++ = c;
        if (c == 0)
          break;
      }
    while (--n != 0);
  
  /* If not enough room in dest, add NUL and traverse rest of src */
  if (n == 0)
    {
      if (dest_size != 0)
        *d = 0;
      while (*s++)
        ;
    }
  
  return s - src - 1;  /* count does not include NUL */
}

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
         size_t      dest_size)
{
  register char *d = dest;
  register const char *s = src;
  register size_t bytes_left = dest_size;
  size_t dlength;  /* Logically, MIN (strlen (d), dest_size) */
  
  if (dest == NULL || src == NULL)
    return 0;
  
  /* Find the end of dst and adjust bytes left but don't go past end */
  while (*d != 0 && bytes_left-- != 0)
    d++;
  dlength = d - dest;
  bytes_left = dest_size - dlength;
  
  if (bytes_left == 0)
    return dlength + strlen (s);
  
  while (*s != 0)
    {
      if (bytes_left != 1)
        {
          *d++ = *s;
          bytes_left--;
        }
      s++;
    }
  *d = 0;
  
  return dlength + (s - src);  /* count does not include NUL */
}

#define G_VA_COPY(ap1, ap2)   (*(ap1) = *(ap2))

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
           va_list      args)
{
  if (string == 0) return -1;

  va_list args2;
  int len;

  G_VA_COPY (args2, args);

  const size_t bufSize = ::_vscprintf (format, args) + 1;
  *string = reinterpret_cast <char*>
    (::malloc (bufSize));

  if (*string != 0)
    len = ::vsprintf_s (*string, bufSize, format, args2);
  else 
    len = -1;

  va_end (args2);

  return len;
}

// From gnu libc 2.7
char *
strsep (char **stringp, const char *delim)
{
  char *begin, *end;

  begin = *stringp;
  if (begin == NULL)
    return NULL;

  /* A frequent case is when the delimiter string contains only one
     character.  Here we don't need to call the expensive `strpbrk'
     function and instead work using `strchr'.  */
  if (delim[0] == '\0' || delim[1] == '\0')
    {
      char ch = delim[0];

      if (ch == '\0')
        end = NULL;
      else
        {
          if (*begin == ch)
            end = begin;
          else if (*begin == '\0')
            end = NULL;
          else
            end = strchr (begin + 1, ch);
        }
    }
  else
    /* Find the end of the token.  */
    end = strpbrk (begin, delim);

  if (end)
    {
      /* Terminate the token and set *STRINGP past NUL character.  */
      *end++ = '\0';
      *stringp = end;
    }
  else
    /* No more delimiters; this is the last token.  */
    *stringp = NULL;

  return begin;
}
#endif

#ifdef _WIN32
std::wstring sFormat( std::wstring format, ... )
{
  va_list list;
  va_start(list, format);
  std::wstring s(sFormatVa(format, list));
  va_end(list);
  return s;
}

std::wstring sFormatVa( const std::wstring & format, va_list list )
{
	CString buf;
	buf.FormatV(format.c_str(), list);
	return buf.GetString();
}

std::string sFormatVaA( const std::string & format, va_list list )
{
	CStringA buf;
	buf.FormatV(format.c_str(), list);
	return buf.GetString();
}
#endif

#ifdef _WIN32
SMAKE_THROW_FN_IMPL(sThrow, SException)
#endif


/*void stripEndChar( char * buf, char ch )
{
  int len = strlen(buf);
  if ( buf[len - 1] == ch ) buf[len - 1] = '\0';
}*/

#ifdef _WIN32
std::wstring sWinErrMsg (DWORD errorCode)
{
   std::std::wstring res;
   
   LPVOID lpMsgBuf = 0;;
   bool messageFound = false;
   DWORD err2;

   messageFound = FormatMessage 
         (FORMAT_MESSAGE_ALLOCATE_BUFFER 
          | FORMAT_MESSAGE_IGNORE_INSERTS 
          | FORMAT_MESSAGE_FROM_SYSTEM,
          NULL,
          errorCode,
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          (LPTSTR) &lpMsgBuf,
          100,
          NULL
          ) != 0;
//FIXME WSAEINVAL wrapping 
   if (!messageFound) {  //TODO add printing of error code
     // with message
      err2 = GetLastError (); //FIXME for socket!
      if (err2 == ERROR_MR_MID_NOT_FOUND) {
         messageFound = FormatMessage 
               (FORMAT_MESSAGE_ALLOCATE_BUFFER 
               | FORMAT_MESSAGE_IGNORE_INSERTS 
               | FORMAT_MESSAGE_FROM_HMODULE,
               GetModuleHandle(L"wininet.dll"),
               errorCode,
               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
               (LPTSTR) &lpMsgBuf,
               100,
               NULL
               ) != 0;
      }
   }

   if (messageFound) {
      if (lpMsgBuf) {
         /*stripEndChar(lpMsgBuf, '\n');
         stripEndChar(lpMsgBuf, '\r');
         stripEndChar(lpMsgBuf, '.');*/
		   res = (LPCTSTR)lpMsgBuf;
		   LocalFree( lpMsgBuf );
      }
   }
   else {
     return SFORMAT ("Unable to format error message " << errorCode
        << ", error " << err2);
   }

	return res;
}

/*
std::string sWinErrMsg( unsigned long err )
{
  char buf[10 * 1024];
  char *_buf = buf ;

  buf[sizeof buf-1] = '\0' ;
  if ( FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    _buf, sizeof(buf) - 1, NULL) != 0 )
  {
    stripEndChar(buf, '\n');
    stripEndChar(buf, '\r');
    stripEndChar(buf, '.');
    return std::string(buf);
  }
  else return std::string();
}
*/

std::string loadResourceStr( int id )
{
  char buf[1024 * 10];
  LoadStringA(0, id, buf, sizeof(buf));
  return std::string(buf);
}

void checkHR( HRESULT r )
{
  if ( !SUCCEEDED(r) )
    sThrow(sWinErrMsg(r));
}

std::wstring str2wstr( const std::string & str )
{
  int slen = strlen(str.c_str());
  int wlen = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slen, 0, 0);
  wchar_t* buf = new wchar_t [wlen + 1];
  MultiByteToWideChar(CP_ACP, 0, str.c_str(), slen, buf, wlen);
  std::wstring wstr(buf, wlen);
  delete [] buf;
  return wstr;
}

std::string wstr2str( const std::wstring & wstr )
{
  int slen = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.length(), 0, 0, 0, 0);
  char * buf = new char [slen + 1];
  WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.length(), buf, slen, 0, 0);
  buf[slen] = 0;
  std::string str(buf);
  delete [] buf;
  return str;
}

std::string toUTF8 (const std::wstring& wstr)
{
  int slen = ::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), 0, 0, 0, 0);
  char * buf = new char [slen + 1];
  WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), buf, slen, 0, 0);
  buf[slen] = 0;
  std::string str(buf);
  delete [] buf;
  return str;
}

std::wstring fromUTF8 (const std::string& str)
{
  int slen = strlen(str.c_str());
  int wlen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), slen, 0, 0);
  wchar_t* buf = new wchar_t [wlen + 1];
  MultiByteToWideChar(CP_UTF8, 0, str.c_str(), slen, buf, wlen);
  std::wstring wstr(buf, wlen);
  delete [] buf;
  return wstr;
}
#endif

/*std::string uni2ascii (const std::wstring & str)
{
  std::string res (str.length (), ' ');

  for (
    std::wstring::const_iterator cit = str.begin (),
    std::string::iterator it = res.begin ()
    cit != str.end ();
    cit++, it++
      )
    {
      wchar_t wch = *cit;
      if (wch <= 127)
        *it = (char) wch;
      else
        *it = '?';
    }
}

std::wstring ascii2uni (const std::string & str)
{
  std::wstring res (str.length (), L' ');

  for (
    std::string::const_iterator cit = str.begin (),
    std::wstring::iterator it = res.begin ()
    cit != str.end ();
    cit++, it++
      )
    {
      char ch = *cit;
      if (ch <= 127)
        *it = (wchar_t) wch;
      else
        *it = '?';
    }
}*/

#ifdef _WIN32
#define TIME_ADJUST_32_64 116444736000000000I64

// Got from MSDN
FILETIME TimetToFileTime (time_t t)
{
    FILETIME ft;

    LONGLONG ll = Int32x32To64(t, 10000000) 
      + TIME_ADJUST_32_64;
    ft.dwLowDateTime = (DWORD) ll;
    ft.dwHighDateTime = ll >>32;

    return ft;
}

time_t FileTimeToTimet (FILETIME ft)
{
    LARGE_INTEGER li;
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;

    return (li.QuadPart - TIME_ADJUST_32_64) 
      / 10000000;
}
#endif

/**
 * Allocate a new char* string by the std::string arg.
 */
char* string2char_ptr (const std::string& str)
{
  char *cstr = new char[str.length() + 1];
  strcpy(cstr, str.c_str());
  return cstr;
}

