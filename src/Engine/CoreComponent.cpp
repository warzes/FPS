#include "stdafx.h"
#include "CoreComponent.h"
//=============================================================================
RTTI_DEFINITIONS(CoreComponent);
//=============================================================================
CoreComponent::CoreComponent(EngineApp& app) : m_app(&app) {}
//=============================================================================
void CoreComponent::SetEngineApp(EngineApp& app)
{
	m_app = &app;
}
//=============================================================================
bool CoreComponent::Enabled() const
{
	return m_enabled;
}
//=============================================================================
void CoreComponent::SetEnabled(bool enabled)
{
	m_enabled = enabled;
}
//=============================================================================