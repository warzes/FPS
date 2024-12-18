﻿#include "stdafx.h"
#include "Log.h"
#include "LogSystem.h"
//=============================================================================
extern LogSystem* thisLogSystem;
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