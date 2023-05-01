
#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <vector>
#include <map>
#include <apr-1/apr.h>
#include <boost/shared_ptr.hpp>
#include "plugin_framework/plugin.h"
#include <string>

class DynamicLibrary;
struct IObjectAdapter;

class PluginManager
{
  typedef std::map<std::string, boost::shared_ptr<DynamicLibrary> > DynamicLibraryMap; 
  typedef std::vector<PF_ExitFunc> ExitFuncVec;  
  typedef std::vector<PF_RegisterParams> RegistrationVec; 

public:   
  typedef std::map<std::string, PF_RegisterParams> RegistrationMap;

  static PluginManager & getInstance();
  static int initializePlugin(PF_InitFunc initFunc);
  int loadAll(const std::string & pluginDirectory, PF_InvokeServiceFunc func = nullptr);
  int loadByPath(const std::string & path);

  void * createObject(const std::string & objectType, IObjectAdapter & adapter);

  int shutdown();  
  static int registerObject(const char * nodeType, 
                                    const PF_RegisterParams * params);
  const RegistrationMap & getRegistrationMap();
  PF_PlatformServices & getPlatformServices();

private:
  ~PluginManager();    
  PluginManager();
  PluginManager(const PluginManager &);
  
  DynamicLibrary * loadLibrary(const std::string & path, std::string & errorString);
private:
  bool                inInitializePlugin_;
  PF_PlatformServices platformServices_;
  
  // map<path,dynamicLibrary>
  DynamicLibraryMap   dynamicLibraryMap_;

  ExitFuncVec         exitFuncVec_;

  // PF_ExitFunc exitFunc = initFunc(&pm.platformServices_);
  RegistrationMap     tempExactMatchMap_;   // register exact-match object types 
  RegistrationVec     tempWildCardVec_;     // wild card ('*') object types

  RegistrationMap     exactMatchMap_;   // register exact-match object types 
  RegistrationVec     wildCardVec_;     // wild card ('*') object types
};


#endif
