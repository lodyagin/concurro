/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
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

#ifndex CONCURRO_PATH_H_
#define CONCURRO_PATH_H_

#include <string>
#include <list>
#include <locale>

namespace curr
{

  class Path
  {
	 friend Path operator+ (const Path&, const Path&);
	 friend bool operator== (const Path&, const Path&);

  public:
	 class PathException : public PalExceptionEx
	 {
	 public:
		PathException (const std::wstring& msg) : PalExceptionEx (msg)
		{};
	 };

	 class InvalidPath : public PathException
	 {
	 public:
	 InvalidPath( const std::wstring& path, const std::wstring& reason) : PathException
		  ( L"Invalid path: [" + path + L"]: " + reason ) {}
	 };

	 class PathExceedsMaxPath : public PathException
	 {
	 public:
	 PathExceedsMaxPath(const std::wstring& path) : PathException
		  ( L"The path exceeds maxPath: [" + path + L"]" )
		{}
	 };

	 Path( size_t lenLimit );
	 Path( const std::wstring& _path);

	 // Used locale
	 static const std::locale loc;

	 enum PathType : int
	 {
		path_unknown = 0,
		  path_windows = 1,
		  path_linux   = 2,
        };

	 // can contain drive letter only in a case of absolute path
	 std::wstring to_string () const;
	 std::wstring to_string (bool endWithSlash) const;

	 //bool is_valid_on_windows(LPCWSTR lpcPath);
	 bool is_relative () const { return isRelative; }
	 bool is_root_dir () const;
  
	 // this path is below p2
	 bool is_below (const Path& p2) const;

	 // number of directory in the path
	 unsigned int nDirs () const { return path.size (); }

	 // Return a new path constructed from
	 // the first n dirs of this path
	 // n <= nDirs ()
	 Path n_first_dirs (unsigned n) const;

	 // remove '.' and '..'
	 // return false if some '..' are still
	 // present (only the case it designates
	 // a relative directory above the start point
	 bool normalize ();

	 void make_relative ()
	 {
		isRelative = true;
		isFromRootFolder = false;
	 }
        
	 static bool is_valid_path(const std::wstring& _path)
	 {
		return Path(_T("")).checkPathType(_path) == path_linux; 
	 }

  protected:

	 Path::PathType checkPathType(const std::wstring& _path);

	 typedef std::list<std::wstring> List;

	 // In this form child will call init() by himself
	 Path()
	 {
	 };

	 Path( const List& _path, bool _isRelative, bool _isFromRootFolder )
	 {
		path = _path;
		isRelative = _isRelative;
		isFromRootFolder = _isFromRootFolder;
	 };

	 std::wstring to_string_generic(wchar_t separator) const;

	 void init (const std::wstring& pathStr);
	 void parse_path (const std::wstring& pathStr);

  protected:
	 bool isRelative;
	 bool isFromRootFolder;

	 List path;

	 size_t maxPath;
  };

  // The suffix mast be relative path
  Path operator+ (const Path& prefix, const Path& suffix);

  bool operator== (const Path&, const Path&);
  bool operator!= (const Path&, const Path&);

} // namespace curr

#endif CONCURRO_PATH_H_
