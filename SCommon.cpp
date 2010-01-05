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

