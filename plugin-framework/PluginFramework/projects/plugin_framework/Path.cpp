#include "base.h"
#include "Path.h"
#include "Directory.h"
#include <boost/tokenizer.hpp>
#include <iterator>

#ifdef WIN32
  #include <windows.h>

  const char * Path::sep = "\\";
#else
  #include <sys/stat.h>
  #include <fstream>

  const char * Path::sep = "/";
#endif

Path::Path(const std::string & path) : path_(path)
{
}

static apr_status_t getInfo(const std::string & path, apr_int32_t wanted, apr_finfo_t & info)
{
  CHECK(!path.empty()) << "Can't get the info of an empty path";

  apr_status_t res;
  apr_pool_t * pool = NULL;
  
//#ifdef WIN32 
//  res = ::apr_pool_create(&pool, NULL);
//#endif
//  
//  res = ::apr_stat(&info, path.c_str(), wanted, pool);
//  
//#ifdef WIN32
//  ::apr_pool_destroy(pool);
//#endif
  
  return res;
} 

bool Path::exists(const std::string & path)
{
  if (path.empty())
    return false;

  // TO DO.使用C++17文件系统实现判断路径是否存在
  
  //apr_finfo_t st;      
  //apr_status_t res  = getInfo(path, APR_FINFO_TYPE, st);
  //return res == APR_SUCCESS;
}

static apr_filetype_e getType(const std::string & path)
{
  apr_finfo_t st;
  apr_status_t res = getInfo(path, APR_FINFO_TYPE, st);
  CHECK(res == APR_SUCCESS) 
    << "Can't get info for '" << path << "', " << base::getErrorMessage();
  
  return st.filetype;
}


bool Path::isFile(const std::string & path)
{
  return getType(path) == APR_REG;
}

bool Path::isDirectory(const std::string & path)
{
    // TO DO. 用C++17实现判断 path是否为文件目录
    return false;

  /*return getType(path) == APR_DIR;*/
}

bool Path::isSymbolicLink(const std::string & path)
{
  return getType(path) == APR_LNK;
}
  
bool Path::isAbsolute(const std::string & path)
{
//  CHECK(!path.empty()) << "Empty path is invalid";
//#ifdef WIN32
//  if (path.size() < 2)
//    return false;
//  else
//  return path[1] == ':';
//#else
//  return path[0] == '/';
//#endif

    return false;
}

bool Path::areEquivalent(const std::string & path1, const std::string & path2)
{
#if 0
  apr_finfo_t st1;
  apr_finfo_t st2;
  apr_int32_t wanted = APR_FINFO_IDENT;
  getInfo(path1.c_str(), wanted, st1);
  getInfo(path2.c_str(), wanted, st2);
  bool res = true;
  res &= st1.device == st2.device;
  res &= st1.inode == st2.inode;
  res &= std::string(st1.fname) == std::string(st2.fname);
  
  return res;

#endif
  return "";
}

std::string Path::getParent(const std::string & path)
{
#if 0
  Path::StringVec sv;
  Path::split(path, sv);
  std::string root;
  if (path[0] == '/')
    root = "/";
  return root + Path::join(sv.begin(), sv.end()-1);

#endif

  return "";
}

std::string Path::getBasename(const std::string & path)
{
#if 0
  std::string::size_type index = path.find_last_of(Path::sep);
  
  if (index == std::string::npos)
    return path;
  
  return std::string(path.c_str() + index + 1, index);
#endif

  return "";
}

std::string Path::getExtension(const std::string & path)
{
#if 0
  std::string filename = Path::getBasename(path);
  std::string::size_type index = filename.find_last_of('.');
  
  // If its a  regular or hidden filenames with no extension
  // return an empty string
  if (index == std::string::npos ||  // regular filename with no ext 
      index == 0                 ||  // hidden file (starts with a '.')
      index == path.size() -1)       // filename ends with a dot
    return "";
  
  // Don't include the dot, just the extension itself (unlike Python)
  return filename.substr(index + 1);
#endif

  return "";
}

apr_size_t Path::getFileSize(const std::string & path)
{
#if 0
  apr_finfo_t st;
  apr_int32_t wanted = APR_FINFO_TYPE | APR_FINFO_SIZE;
  getInfo(path.c_str(), wanted, st);
  CHECK(st.filetype == APR_REG) << "Can't get the size of a non-file object";
  
  return (apr_size_t)st.size;

#endif 
  return 0;
}

std::string Path::normalize(const std::string & path)
{
  return path;
}

std::string Path::makeAbsolute(const std::string & path)
{
    // 这里使用绝对路径
    // TO DO. 可以使用C++17的文件系统，实现该功能后续改写
  if (Path::isAbsolute(path))
    return path;
  
  return std::string("");
}

void Path::split(const std::string & path, StringVec & parts)
{
#if 0
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

  tokenizer tokens(path, boost::char_separator<char>(Path::sep));
  
  std::copy(tokens.begin(), tokens.end(), std::inserter(parts, parts.begin()));
#endif

}

std::string Path::join(StringVec::iterator begin, StringVec::iterator end)
{
#if 0
  // Need to get rid of redundant separators

  if (begin == end)
    return "";
    
  std::string path(*begin++);
  
  while (begin != end)
  {
    path += Path::sep;
    path += *begin++;
  };
  
  return path;

#endif
  return "";
}


void Path::copy(const std::string & source, const std::string & destination)
{

#if 0
  CHECK(!source.empty()) 
    << "Can't copy from an empty source";

  CHECK(!destination.empty()) 
    << "Can't copy to an empty destination";

  CHECK(source != destination)
    << "Source and destination must be different";
    
  if (isDirectory(source))
  {
    Directory::copyTree(source, destination);
    return;
  } 

#ifdef WIN32
  // This will overwrite quitely destination file if exist
  BOOL res = ::CopyFile(LPTSTR(source.c_str()), LPTSTR(destination.c_str()), FALSE);
  CHECK(res != FALSE) << base::getErrorMessage();
#else

  try
  {
    std::ifstream  in(source.c_str());
    std::ofstream  out(destination.c_str()); 
    in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    out.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    out << in.rdbuf();
  }
  catch (...)
  {
    // Should I do it? maybe just let the standard exception propagate on its own?
    THROW << "Path::copy() failed. " << base::getErrorMessage();
  }
#endif

#endif

}

void Path::remove(const std::string & path)
{
#if 0
  CHECK(!path.empty()) 
    << "Can't remove an empty path";

  // Just return if it doesn't exist already
  if (!Path::exists(path))
    return;
    
  if (isDirectory(path))
  {
    Directory::removeTree(path);
    return;
  } 

#ifdef WIN32
  BOOL res = ::DeleteFile(LPTSTR(path.c_str()));
  CHECK(res != FALSE) << base::getErrorMessage();
#else
  int res = ::remove(path.c_str());
  CHECK(res == 0) << base::getErrorMessage();
#endif

#endif

}
  
void Path::rename(const std::string & oldPath, const std::string & newPath)
{
#if 0

  CHECK(!oldPath.empty() && !newPath.empty()) 
    << "Can't rename to/from empty path";
#ifdef WIN32
  BOOL res = ::MoveFile(LPTSTR(oldPath.c_str()), LPTSTR(newPath.c_str()));
  CHECK(res != FALSE) << base::getErrorMessage();
#else
  int res = ::rename(oldPath.c_str(), newPath.c_str());
  CHECK(res != -1) << base::getErrorMessage();
#endif

#endif

}

Path::operator const char *() const
{
  return path_.c_str();
}

Path & Path::operator+=(const Path & path)
{
#if 0
  Path::StringVec sv;
  sv.push_back(std::string(path_));
  sv.push_back(std::string(path.path_));
  path_ = Path::join(sv.begin(), sv.end());
  return *this;

#endif
  return *this;
}

Path Path::getParent() const
{
  return Path::getParent(path_);
}

Path Path::getBasename() const
{
  return Path::getBasename(path_);
}

Path Path::getExtension() const
{
  return Path::getExtension(path_); 
}

apr_size_t Path::getFileSize() const
{
  return Path::getFileSize(path_); 
}
  
Path & Path::normalize()
{
  //path_ = Path::normalize(path_);
  return *this;
}

Path & Path::makeAbsolute()
{
  if (!isAbsolute())
    path_ = Path::makeAbsolute(path_);

  return *this;
}
  
void Path::split(StringVec & parts) const
{
  Path::split(path_, parts);
}

void Path::remove() const
{
  Path::remove(path_);
}
  
void Path::rename(const std::string & newPath)
{
  //Path::rename(path_, newPath);
  //path_ = newPath;
}

bool Path::isDirectory() const
{
  return Path::isDirectory(path_);
}

bool Path::isFile() const
{
  return Path::isFile(path_);
}

bool Path::isSymbolicLink() const
{
  return Path::isSymbolicLink(path_);
}  
bool Path::isAbsolute() const
{
  return Path::isAbsolute(path_);
}

bool Path::exists() const
{
  return Path::exists(path_);
}

bool Path::isEmpty() const
{
  return path_.empty();
}

Path operator+(const Path & p1, const Path & p2)
{
  Path::StringVec sv;
  sv.push_back(std::string(p1));
  sv.push_back(std::string(p2));
  return Path::join(sv.begin(), sv.end());
}


