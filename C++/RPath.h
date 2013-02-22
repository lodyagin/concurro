#pragma once

#include <string>
#include <list>
#include <locale>

namespace pallib 
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


/*
  class WindowsPath : public Path
  {
  public:
  WindowsPath (const std::wstring& _path, size_t lenLimit = MAX_PATH);
  virtual std::wstring unix_form () const;

  };
*/
} // namespace pallib

