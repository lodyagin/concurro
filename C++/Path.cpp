/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Cohors LLC 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU Lesser General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public License
  for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "Path.h"
#include <portable.h>
#include "path.h"
#include <locale>
#include <sstream>
#include <algorithm>
#include <assert.h>
#include "windef.h"

#include "utf_converter.h"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

namespace curr {

  // Use the default locale for case conversion
  const std::locale Path::loc;

  Path::Path (size_t lenLimit)
	 : maxPath(lenLimit)
  {
  }


  Path::Path (const std::wstring& _path)
	 :maxPath(MAX_PATH)
  {

	 int type = checkPathType(_path);
	 switch(type)
	 {
	 case Path::PathType::path_linux:
		init ( _path );
		break;

	 default:
		throw InvalidPath (_path, L" : This path not in linux format");
		break;
	 }

  }

  // êîäû âîçâðàòà
  //todo íåîáõîäèìî ïåðåðàáîòàòü ôóíêöèþ, òàê êàê ïîñëå îòëàäêè òåñòîâ áîëüøàÿ å¸ ÷àñòü óäàëåíà
  Path::PathType Path::checkPathType(const std::wstring& _path)
  {
	 if(_path.length() == 0) return Path::PathType::path_linux;

	 //length too large or empty
	 if ( _path.length() > maxPath -1 ) throw(Path::PathExceedsMaxPath(_path));

	 //for special symbol
	 bool flagSpecOnce(false);
	 Path::PathType _res = Path::PathType::path_linux;
	 int index = 0;
	 int temp;

	 for (std::wstring::const_iterator iter = _path.begin(); iter != _path.end(); iter++)
	 {
		if (*iter == L'/')
		{
		  if (flagSpecOnce || _res == Path::PathType::path_windows) {_res = Path::PathType::path_unknown; break;}
		  else {flagSpecOnce = true; _res = Path::PathType::path_linux;}
		}
		else
		  /*if (*iter == L'\\')
			 {
			 if (flagSpecOnce || _res == Path::PathType::path_linux) {_res = Path::PathType::path_unknown; break;}
			 else {flagSpecOnce = true; _res = Path::PathType::path_windows;}
			 }
			 else*/
		  if (*iter == L':' )
		  {
			 if (index == 1)
			 {_res = Path::PathType::path_windows; break;}
		  }
		  else
		  {
			 /*if (*iter == L'<' || *iter == L'>' || *iter == L'"' || *iter == L'|')
				{_res = Path::PathType::path_unknown; break;}
				else*/
			 flagSpecOnce =false;
		  }
		index++;
	 }

	 return _res;
  }

  // common part of constructors
  void Path::init (const std::wstring& pathStrIn)
  {
	 //----------------------------------------------------//
	 // ïðîâåðêà äàííûõ íà âàëèäíîñòü

	 // åñëè äëèíà áîëüøå âîçìîæíîé - ãåíåðèðóåì èñêëþ÷åíèå
	 if (pathStrIn.length() > maxPath - 1)
		throw PathExceedsMaxPath (pathStrIn);

	 if( pathStrIn.length () > maxPath )
		throw InvalidPath (pathStrIn, L" too long");
	 // UT for dir creation maxPath - 12 (8.3 is leaved for files)

	 // åñëè íà÷èíàåòñÿ ñî ñëåøà, òî ïóòü ÍÅ ðåëàòèâíûé, èíà÷å ðåëàòèâíûé
	 isRelative = !(pathStrIn[0] == L'/');
	 isFromRootFolder = ! isRelative; // << êàê ïîä ëèíóêñîì óçíàòü ?

	 parse_path (pathStrIn);
  }

  // from string to list of directories
  void Path::parse_path (const std::wstring& pathStr)
  {
	 std::wstring::size_type lastPos = 0;

	 while( lastPos < pathStr.length () )
	 {
		std::wstring::size_type currPos = pathStr.find_first_of (L"/", lastPos);
		std::wstring::size_type nPos = std::wstring::npos;

		if (currPos == nPos)
		{
		  std::wstring nextDir = pathStr.substr (lastPos, pathStr.length () - lastPos);
		  path.push_back( nextDir );
		  return;
		}

		if (lastPos == currPos)
		{
		  lastPos++;
		  continue;
		}

		if (currPos > 0)
		{
		  std::wstring nextDir = pathStr.substr (lastPos, currPos - lastPos);
		  path.push_back (nextDir);
		  lastPos = currPos + 1;
		}
	 }
  }

  //---------------------------------------------------

  std::wstring Path::to_string () const
  {
	 return to_string_generic (L'/');
  }

  std::wstring Path::to_string (bool endWithSlash) const
  {
	 if( endWithSlash )
	 {
		if( isRelative && path.size () == 0 )
		  throw InvalidPath( to_string (), L"unable to append / to the end");
	 }
	 else
	 {
		if( !isRelative && path.size () == 0 )
		  return L"/";//root directory
	 }

	 std::wstring s = to_string ();
	 if (!endWithSlash)
		s = (s[s.length () - 1] == L'/') ? s : s + L'/';

	 return s;
  }

  std::wstring Path::to_string_generic (wchar_t separator) const
  {
	 std::wstring strm;
	 List::const_iterator cit = path.begin ();
	 bool first = true;
	 for (; cit != path.end (); cit++)
	 {
		if (!first) strm += separator;
		strm += (*cit);
		first = false;
	 }

	 std::wstring s = strm;
	 if (isFromRootFolder)
		s = separator + strm;

	 return s;
  }

  //---------------------------------------------------


  bool Path::is_root_dir () const 
  { 
	 return !isRelative && path.size () == 0; 
  }

  bool Path::normalize ()
  {
	 const std::wstring point (L".");
	 const std::wstring point2 (L"..");

	 List res;
	 bool normalized = true;

	 List::const_iterator cit = path.begin ();
	 while (cit != path.end ())
	 {
		if (*cit == point)
		  ; // just skip
		else if (*cit == point2)
		{
		  if (res.size () > 0 && res.back () != point2)
			 res.pop_back ();
		  else if (res.size ())
		  {
			 res.push_back (point2);
			 normalized = false;
		  }
		}
		else res.push_back (*cit);
		cit++;
	 }
  
	 path = res; // replace the current path 
	 return normalized;
  }

/*bool Path::is_valid_on_windows(LPCWSTR lpcPath)
  {
  if(!lpcPath)
  return FALSE;
	
  size_t i = 0;
  while(lpcPath[i] && i <= maxPath){
  wchar_t wc = lpcPath[i];
	
  if(wc < 32)
  return false;
		
  if(wcschr(L"<>\"/|?*", wc))
  return false;

  if(L':' == wc && i != 1)
  return false;

  i++;
  }
	
  if(i == 0 || i >= maxPath)
  return false;
		
  return true;
  }*/

  Path Path::n_first_dirs (unsigned int n) const
  {
	 if (n > nDirs ())
		throw pallib::PalExceptionEx (L"Path::n_first_dirs overflow");

	 List res;
	 List::const_iterator cit = path.begin ();
	 for (List::size_type i = 0; i < n; i++)
		res.push_back (*cit++);

	 return Path (res, isRelative, isFromRootFolder);
  }

  Path operator+ (const Path& prefix, const Path& suffix)
  {
	 if( !suffix.is_relative () || suffix.isFromRootFolder )
		throw Path::InvalidPath(suffix.to_string(), L" as a suffix" );

	 Path res(
		prefix.path,
		prefix.is_relative (),
		prefix.isFromRootFolder
		);
  
	 res.path.insert(
		res.path.end (),
		suffix.path.begin (),
		suffix.path.end ()
		);

	 res.normalize();

	 return res;
  }

  static bool
  char_equal_ignore_case (wchar_t a, wchar_t b) 
  {
	 return std::tolower (a, Path::loc)
		== std::tolower (b, Path::loc);
  }

  static bool 
  is_equal_ignore_case (const std::wstring& a, const std::wstring& b) 
  {
	 return a.length () == b.length ()
		&& std::equal 
      (a.begin (), a.end (),
       b.begin (), 
       char_equal_ignore_case
		  );
  }

  // it ignore case
  bool operator== (const Path& a, const Path& b)
  {
	 if( a.isRelative == b.isRelative )
	 {
		return ( a.path.size () == b.path.size () &&
					std::equal(a.path.begin (), a.path.end (), b.path.begin (), is_equal_ignore_case) );
	 }

	 return false;
  }

  bool operator!= (const Path& a, const Path& b)
  {
	 return !(a == b);
  }

/*
  WindowsPath::WindowsPath (const std::wstring& _path, size_t lenLimit)
  try  :Path(_path, lenLimit, false)
  {
  } catch (PathException &) { throw; }

  std::wstring WindowsPath::unix_form () const
  {
  std::wstring res;
  WindowsPath tmpPath = *this;
  tmpPath.drive = L'?';

  if (tmpPath.isRelative)
  res = tmpPath.to_string_generic (L'/');
  else
  {
  if (drive == L'c')
  {
  struct passwd *pw = getpwuid(getuid());
  if (!pw) 
  {
  errnomap ();
  throw PathException (L"");
  }

  const char *homedir = pw->pw_dir;
  Narrow2WideConverter conv(homedir);

  res = conv.getWstr();
  }
  res += tmpPath.to_string_generic (L'/');
  }

  return res;
  }
*/

}
