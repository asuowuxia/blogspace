
#include <string>

#include <apr-1/apr_network_io.h> // for APR_PATH_MAX

#include "Directory.h"
#include "Path.h"
#include "PluginManager.h"
#include "DynamicLibrary.h"
#include "ObjectAdapter.h"

#if defined(PF_PLATFORM_MAC)
  static std::string dynamicLibraryExtension("dylib");
#elif defined(PF_PLATFORM_LINUX)
  static std::string dynamicLibraryExtension("so");
#elif defined(PF_PLATFORM_WINDOWS)
  static std::string dynamicLibraryExtension("dll");
#endif


// The registration params may be received from an external plugin so it is 
// crucial to validate it, because it was never subjected to our tests.
static bool isValid(const char * objectType, const PF_RegisterParams * params)
{
  if (!objectType || !(*objectType))
     return false;
  if (!params ||!params->createFunc || !params->destroyFunc)
    return false;
  
  return true;
}

// ---------------------------------------------------------------

int PluginManager::registerObject(const char* objectType, const PF_RegisterParams* params)
{
	// Check parameters
	if (!isValid(objectType, params))
		return -1;

	PluginManager& pm = PluginManager::getInstance();

	// Verify that versions match
	PF_PluginAPI_Version v = pm.platformServices_.version;
	if (v.major != params->version.major)
		return -1;

	std::string key((const char*)objectType);
	// If it's a wild card registration just add it
	if (key == std::string("*"))
	{
		pm.wildCardVec_.push_back(*params);
		return 0;
	}

	// If item already exists in eactMatc fail (only one can handle)
	if (pm.exactMatchMap_.find(key) != pm.exactMatchMap_.end())
		return -1;

	pm.exactMatchMap_[key] = *params;


	return 0;
}

// ---------------------------------------------------------------

PluginManager & PluginManager::getInstance()
{
  static PluginManager instance;
  
  return instance;
}


int PluginManager::loadAll(const std::string& pluginDirectory, PF_InvokeServiceFunc func)
{
	if (pluginDirectory.empty()) // Check that the path is non-empty.
		return -1;

	platformServices_.invokeService = func;

	Path dir_path(pluginDirectory);
	if (!dir_path.exists() || !dir_path.isDirectory())
		return -1;

    // TO DO. 用C++17 遍历文件夹下的所有文件，用于加载 .dll文件

	Directory::Entry e;
	Directory::Iterator di(dir_path);
	while (di.next(e))
	{
		Path full_path(dir_path + Path(e.path));

		// Skip directories
		if (full_path.isDirectory())
			continue;

		// Skip files with the wrong extension
		std::string ext = std::string(full_path.getExtension());
		if (ext != dynamicLibraryExtension)
			continue;

		// Ignore return value
		loadByPath(std::string(full_path));
	}

	return 0;
}

apr_int32_t PluginManager::initializePlugin(PF_InitFunc initFunc)
{
  PluginManager & pm = PluginManager::getInstance();

  PF_ExitFunc exitFunc = initFunc(&pm.platformServices_);
  if (!exitFunc)
    return -1;
  
  // Store the exit func so it can be called when unloading this plugin
  pm.exitFuncVec_.push_back(exitFunc);
  return 0;
}
 
PluginManager::PluginManager() : inInitializePlugin_(false)
{
  platformServices_.version.major = 1;
  platformServices_.version.minor = 0;
  platformServices_.invokeService = nullptr; // can be populated during loadAll()
  platformServices_.registerObject = registerObject;
}

PluginManager::~PluginManager()
{
  // Just in case it wasn't called earlier
  shutdown();
}

apr_int32_t PluginManager::shutdown()
{
#if 0
  apr_int32_t result = 0;
  for (ExitFuncVec::iterator func = exitFuncVec_.begin(); func != exitFuncVec_.end(); ++func)
  {
    try
    {
      result = (*func)();
    }
    catch (...)
    {
      result = -1;
    }
  }
  
  
  dynamicLibraryMap_.clear();
  exactMatchMap_.clear();
  wildCardVec_.clear();
  exitFuncVec_.clear();
  
  return result;

#endif
  return 0;
}

int PluginManager::loadByPath(const std::string & pluginPath)
{
    Path path(pluginPath);
    
    // Resolve symbolic links. linux要考虑软连接
    #ifndef WIN32 
    if (path.isSymbolicLink())
    {
      char buff[APR_PATH_MAX+1];
      int length = ::readlink(path, buff, APR_PATH_MAX);
      if (length < 0)
        return -1;
        
      path = std::string(buff, length);
    }
    #endif
             
    // Don't load the same dynamic library twice
    if (dynamicLibraryMap_.find(std::string(path)) != dynamicLibraryMap_.end())
      return -1;

    path.makeAbsolute();

    std::string errorString;
    DynamicLibrary * d = loadLibrary(std::string(path), errorString);
    if (!d) // not a dynamic library? 
      return -1;
    //                
    //// Get the NTA_initPlugin() function
    PF_InitFunc initFunc = (PF_InitFunc)(d->getSymbol("PF_initPlugin"));
    if (!initFunc) // dynamic library missing entry point?
      return -1;
    
    int res = initializePlugin(initFunc);

    if (res < 0) // failed to initalize?
      return res;

    return 0;
}

void* PluginManager::createObject(const std::string& objectType, IObjectAdapter& adapter)
{
	// "*" is not a valid object type
	if (objectType == std::string("*"))
		return nullptr;

	// Prepare object params
	PF_ObjectParams np;
	np.objectType = (const char*)objectType.c_str(); // string
	np.platformServices = &platformServices_;

	// Exact match found
	if (exactMatchMap_.find(objectType) != exactMatchMap_.end())
	{
		PF_RegisterParams& rp = exactMatchMap_[objectType];
		void* object = rp.createFunc(&np);
		if (object) // great, there is an exact match
		{
			// Adapt if necessary (wrap C objects using an adapter)
			if (rp.programmingLanguage == PF_ProgrammingLanguage_C)
				// ObjectAdapter<ActorAdapter, C_Actor>
				// return new T((U *)object, df);
				// to do: new ActorAdapter((C_Actor*) object,df)
				object = adapter.adapt(object, rp.destroyFunc);

			return object;
		}
	}

	// Try to find a wild card match
	for (int i = 0; i < wildCardVec_.size(); ++i)
	{
		PF_RegisterParams& rp = wildCardVec_[i];
		void* object = rp.createFunc(&np);
		if (object) // great, it worked
		{
			// Adapt if necessary (wrap C objects using an adapter)
			if (rp.programmingLanguage == PF_ProgrammingLanguage_C)
				object = adapter.adapt(object, rp.destroyFunc);

			// promote registration to exactMatc_ 
			// (but keep also the wild card registration for other object types)
			int res = registerObject(np.objectType, &rp);
			if (res < 0)
			{
				// Serious framework should report or log it              
				rp.destroyFunc(object);
				return nullptr;
			}
			return object;
		}
	}

	// Too bad no one can create this objectType
	return nullptr;
}

DynamicLibrary * PluginManager::loadLibrary(const std::string &  path, std::string & errorString)
{
	DynamicLibrary* d = DynamicLibrary::load(path, errorString);
	if (!d)  // not a dynamic library? 
		return nullptr;

	// Add library to map, so it can be unloaded
	Path::makeAbsolute(path);

	dynamicLibraryMap_[Path::makeAbsolute(path)] = boost::shared_ptr<DynamicLibrary>(d);

	return d;
}

const PluginManager::RegistrationMap & PluginManager::getRegistrationMap()
{
  return exactMatchMap_;
}

PF_PlatformServices & PluginManager::getPlatformServices()
{
  return platformServices_;
}
