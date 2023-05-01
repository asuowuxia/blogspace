#ifdef WIN32
  #include <Windows.h>
#else
  #include <dlfcn.h>
#endif

#include "DynamicLibrary.h"
#include <sstream>
#include <iostream>

DynamicLibrary::DynamicLibrary(void* handle) 
	: handle_(handle)
{
}

DynamicLibrary::~DynamicLibrary()
{
	FreeLibrary((HMODULE)handle_);
}

DynamicLibrary* DynamicLibrary::load(const std::string& name,std::string& errorString)
{
	if (name.empty())
	{
		errorString = "Empty path.";
		return nullptr;
	}

	void* handle = nullptr;

	handle = LoadLibraryA(name.c_str());
	if (handle == nullptr)
	{
		DWORD errorCode = GetLastError();
		std::stringstream ss;
		ss << std::string("LoadLibrary(") << name
			<< std::string(") Failed. errorCode: ")
			<< errorCode;
		errorString = ss.str();
	}

	return new DynamicLibrary(handle);
}

void * DynamicLibrary::getSymbol(const std::string & symbol)
{
  if (!handle_)
    return nullptr;
 
    return GetProcAddress((HMODULE)handle_, symbol.c_str());
}

