#include "stdafx.h"
#include "Log.h"
//=============================================================================
LogSystem* thisLogSystem = nullptr;
void RequestExit();
//=============================================================================
void Print(const std::string& msg)
{
	assert(thisLogSystem);
	thisLogSystem->Print(msg);
}
//=============================================================================
void Warning(const std::string& msg)
{
	assert(thisLogSystem);
	thisLogSystem->Warning(msg);
}
//=============================================================================
void Error(const std::string& msg)
{
	assert(thisLogSystem);
	thisLogSystem->Error(msg);
}
//=============================================================================
void Fatal(const std::string& msg)
{
	assert(thisLogSystem);
	thisLogSystem->Fatal(msg);
}
//=============================================================================
LogSystem::~LogSystem()
{
	assert(!thisLogSystem);
}
//=============================================================================
bool LogSystem::Create(const LogSystemCreateInfo& createInfo)
{
	assert(!thisLogSystem);

	thisLogSystem = this;
	return true;
}
//=============================================================================
void LogSystem::Destroy()
{
	thisLogSystem = nullptr;
}
//=============================================================================
void LogSystem::Print(const std::string& msg)
{
	assert(thisLogSystem);
	puts(msg.data());
}
//=============================================================================
void LogSystem::Warning(const std::string& msg)
{
	Print("[WARNING] " + msg);
}
//=============================================================================
void LogSystem::Error(const std::string& msg)
{
	Print("[ERROR] " + msg);
}
//=============================================================================
void LogSystem::Fatal(const std::string& msg)
{
	Print("[FATAL] " + msg);
	RequestExit();
}
//=============================================================================