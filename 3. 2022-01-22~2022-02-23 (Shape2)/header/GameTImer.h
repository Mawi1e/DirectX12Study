#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

using bint = __int64;

class GameTimer {
public:
	GameTimer();
	GameTimer(const GameTimer&);
	~GameTimer();

	void Tick();
	double TotalTime() const;
	double DeltaTime() const;

	void Start();
	void Stop();
	void Reset();

private:
	double m_DeltaTime, m_SecondsPerCount;
	bint m_StopTime, m_BaseTime, m_PrevTime, m_CurrTime, m_PausedTime;
	bool m_isStopped;

};