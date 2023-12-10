#pragma once
#include "pch.h"

using namespace ATL;

class ComObjectRootEx :
	public CComObjectRootBase
{
public:
	typedef CComMultiThreadModel _ThreadModel;
	typedef _ThreadModel::AutoCriticalSection _CritSec;
	typedef _ThreadModel::AutoDeleteCriticalSection _AutoDelCritSec;

	~ComObjectRootEx()
	{
	}

	ULONG InternalAddRef()
	{
		WINRT_ASSERT(m_dwRef != -1L);
		return _ThreadModel::Increment(&m_dwRef);
	}
	ULONG InternalRelease()
	{
		return _ThreadModel::Decrement(&m_dwRef);
	}

	HRESULT _AtlInitialConstruct()
	{
		return m_critsec.Init();
	}
	void Lock()
	{
		m_critsec.Lock();
	}
	void Unlock()
	{
		m_critsec.Unlock();
	}
private:
	_AutoDelCritSec m_critsec;
};

