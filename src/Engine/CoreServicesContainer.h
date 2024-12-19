#pragma once

class CoreServicesContainer final
{
public:
	void AddService(uint32_t typeID, void* service);
	void RemoveService(uint32_t typeID);
	void* FindService(uint32_t typeID) const;

private:
	std::map<uint32_t, void*> m_services;
};