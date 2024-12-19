#pragma once

class CoreTime final
{
public:
	auto TotalCoreTime() const { return m_totalCoreTime; }
	void SetTotalCoreTime(double totalGameTime) { m_totalCoreTime = totalGameTime; }

	auto ElapsedCoreTime() const { return m_elapsedCoreTime; }
	void SetElapsedCoreTime(double elapsedGameTime) { m_elapsedCoreTime = elapsedGameTime; }

private:
	double m_totalCoreTime{ 0.0 };
	double m_elapsedCoreTime{ 0.0 };
};