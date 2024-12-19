#include "stdafx.h"
#include "CoreServicesContainer.h"
//=============================================================================
void CoreServicesContainer::AddService(UINT typeID, void* service)
{
	m_services.insert(std::pair<UINT, void*>(typeID, service));
}
//=============================================================================
void CoreServicesContainer::RemoveService(UINT typeID)
{
	m_services.erase(typeID);
}
//=============================================================================
void* CoreServicesContainer::FindService(UINT typeID) const
{
	auto it = m_services.find(typeID);
	return (it != m_services.end() ? it->second : nullptr);
}
//=============================================================================