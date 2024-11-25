#pragma once

struct WindowSystemCreateInfo final
{

};

class WindowSystem final
{
public:
	bool Create(const WindowSystemCreateInfo& createInfo);
	void Destroy();

	bool IsShouldClose() const;
	void Update();

private:
#if PLATFORM_WINDOWS

#endif
};