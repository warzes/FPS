#pragma once

#include "RTTI.h"

class EngineApp;
class CoreTime;

class CoreComponent : public RTTI
{
	RTTI_DECLARATIONS(CoreComponent, RTTI)
public:
	CoreComponent() = default;
	CoreComponent(EngineApp& app);
	virtual ~CoreComponent() = default;

	auto GetEngineApp() { return m_app; }
	void SetEngineApp(EngineApp& app);
	bool Enabled() const;
	void SetEnabled(bool enabled);

	virtual bool Initialize() { return true; }
	virtual void Update([[maybe_unused]] const CoreTime& gameTime) {}

protected:
	EngineApp* m_app{ nullptr };
	bool       m_enabled{ true };

private:
	CoreComponent(const CoreComponent&) = delete;
	CoreComponent& operator=(const CoreComponent&) = delete;
};