#include "stdafx.h"
#include "SCommon.h"
#include <atlstr.h>

using namespace std;

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

string trimLeft( const string & s, char ch, int maxCount )
{
  const char * b = ptr2ptr(s);
  const char * p = b;
  while ( *p != 0 && *p == ch && (maxCount == -1 || p - b < maxCount ) ) ++p;
  return string(p);
}

string trimRight( const string & s, char ch, int maxCount )
{
  const char * b = ptr2ptr(s);
  const char * e = strchr(b, 0);
  const char * p = e - 1;
  while ( p > b && *p == ch && (maxCount == -1 || e - p + 1 < maxCount ) ) --p;
  return string(b, p - b + 1);
}

string trimBoth( const string & s, char ch, int maxCount )
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


string sFormat( string format, ... )
{
  va_list list;
  va_start(list, format);
  string s(sFormatVa(format, list));
  va_end(list);
  return s;
}

string sFormatVa( const string & format, va_list list )
{
	CString buf;
	buf.FormatV(format.c_str(), list);
	return buf.GetString();
}

SMAKE_THROW_FN_IMPL(sThrow, SException)


/*void stripEndChar( char * buf, char ch )
{
  int len = strlen(buf);
  if ( buf[len - 1] == ch ) buf[len - 1] = '\0';
}*/

std::string sWinErrMsg (DWORD errorCode)
{
   std::string res;
   
   LPVOID lpMsgBuf = 0;;
   bool messageFound = false;
   DWORD err2;

   messageFound = FormatMessageA 
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
         messageFound = FormatMessageA 
               (FORMAT_MESSAGE_ALLOCATE_BUFFER 
               | FORMAT_MESSAGE_IGNORE_INSERTS 
               | FORMAT_MESSAGE_FROM_HMODULE,
               GetModuleHandle("wininet.dll"),
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
     LOG4CXX_WARN (Logging::Root (), 
        SFORMAT ("Unable to format error message " << errorCode
        << ", error " << err2));
   }

	return res;
}

/*
string sWinErrMsg( unsigned long err )
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
    return string(buf);
  }
  else return string();
}
*/

string loadResourceStr( int id )
{
  char buf[1024 * 10];
  LoadStringA(0, id, buf, sizeof(buf));
  return string(buf);
}

void checkHR( HRESULT r )
{
  if ( !SUCCEEDED(r) )
    sThrow(sWinErrMsg(r));
}


/*BSTR toBSTR( const string & str )
{
  int slen = strlen(str.c_str());
  int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slen, NULL, 0);
  BSTR result = SysAllocStringLen(NULL,len);
  MultiByteToWideChar(CP_ACP, 0, str.c_str(), slen, result, len);
//  result[len - 1] = 0;
  return result;
}*/

/*string fromBSTR( const WCHAR * wsz )
{
  return wsz ? wstr2str(wstring(wsz)) : string();
}*/


/*wstring str2wstr( const string & str )
{
  int slen = strlen(str.c_str());
  int wlen = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slen, 0, 0);
  BSTR bstr = SysAllocStringLen(0, wlen);
  MultiByteToWideChar(CP_ACP, 0, str.c_str(), slen, bstr, wlen);
  wstring wstr(bstr, wlen);
  SysFreeString(bstr);
  return wstr;
}*/

/*string wstr2str( const wstring & wstr )
{
  int slen = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.length(), 0, 0, 0, 0);
  char * buf = new char [slen + 1];
  WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.length(), buf, slen, 0, 0);
  buf[slen] = 0;
  string str(buf);
  delete [] buf;
  return str;
}*/

std::ostream& operator << (std::ostream& out, const _com_error &e)
{
   out << "_com_error (Code => " << e.Error ()
      << ", Code meaning => " << e.ErrorMessage ()
      << ", Source => " << e.Source ()
      << ", Description => " << e.Description () << ")";
   return out;
}

bool operator == (const _com_error& a, const _com_error& b)
{
   return a.Error () == b.Error ()
      && strcmp (a.ErrorMessage (), b.ErrorMessage ()) == 0
      && a.Source () == b.Source ()
      && a.Description () == b.Description ();
}

bool operator != (const _com_error& a, const _com_error& b)
{
   return a.Error () != b.Error ()
      || a.Description () != b.Description ()
      || a.Source () != b.Source ()
      || strcmp (a.ErrorMessage (), b.ErrorMessage ()) != 0;
}

