#include "GameTImer.h"

GameTimer::GameTimer() :
	m_DeltaTime(-1), m_StopTime(0), m_BaseTime(0), m_PrevTime(0), m_CurrTime(0), m_PausedTime(0), m_isStopped(false) {
	bint CountsPerSecond;

	QueryPerformanceFrequency((LARGE_INTEGER*)&CountsPerSecond);
	m_SecondsPerCount = 1.0 / CountsPerSecond;
}

GameTimer::GameTimer(const GameTimer&) {
}

GameTimer::~GameTimer() {
}

void GameTimer::Tick() {
	bint currTime;

	if (m_isStopped) { 
		m_DeltaTime = 0.0;
		return;
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_DeltaTime = (currTime - m_PrevTime) * m_SecondsPerCount;
	m_CurrTime = m_PrevTime = currTime;


	if (m_DeltaTime < 0) {
		m_DeltaTime = 0.0;
		return;
	}
}

float GameTimer::TotalTime() const {
	double minusTime;

	if (m_isStopped) {
		minusTime = (double)m_StopTime;
	}
	else if (!m_isStopped) {
		minusTime = (double)m_CurrTime;
	}

	return (float)(((minusTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
}

float GameTimer::DeltaTime() const {
	return ((float)m_DeltaTime);
}

void GameTimer::Start() {
	if (m_isStopped) {
		bint currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		m_PausedTime += (currTime - m_StopTime);
		m_PrevTime = currTime;
		m_StopTime = 0;
		m_isStopped = false;
	}
}

void GameTimer::Stop() {
	if (!m_isStopped) {
		bint currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		m_StopTime = currTime;
		m_isStopped = true;
	}
}

void GameTimer::Reset() {
	bint currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	m_PrevTime = m_BaseTime = currTime;
	m_StopTime = 0;
	m_isStopped = false;
}