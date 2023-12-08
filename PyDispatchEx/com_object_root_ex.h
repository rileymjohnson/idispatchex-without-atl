#pragma once
#include "pch.h"

using namespace ATL;

template <class ThreadModel>
class ComObjectRootEx :
	public CComObjectRootBase
{
public:
	typedef ThreadModel _ThreadModel;
	typedef typename _ThreadModel::AutoCriticalSection _CritSec;
	typedef typename _ThreadModel::AutoDeleteCriticalSection _AutoDelCritSec;

	~ComObjectRootEx()
	{
	}

	ULONG InternalAddRef()
	{
		ATLASSUME(m_dwRef != -1L);
		return _ThreadModel::Increment(&m_dwRef);
	}
	ULONG InternalRelease()
	{
#ifdef _DEBUG
		LONG nRef = _ThreadModel::Decrement(&m_dwRef);
		if (nRef < -(LONG_MAX / 2))
		{
			ATLASSERT(0 && _T("Release called on a pointer that has already been released"));
		}
		return nRef;
#else
		return _ThreadModel::Decrement(&m_dwRef);
#endif
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

template <>
class ComObjectRootEx<CComSingleThreadModel> :
	public CComObjectRootBase
{
public:
	typedef CComSingleThreadModel _ThreadModel;
	typedef _ThreadModel::AutoCriticalSection _CritSec;
	typedef _ThreadModel::AutoDeleteCriticalSection _AutoDelCritSec;

	~ComObjectRootEx() {}

	ULONG InternalAddRef()
	{
		ATLASSUME(m_dwRef != -1L);
		return _ThreadModel::Increment(&m_dwRef);
	}
	ULONG InternalRelease()
	{
#ifdef _DEBUG
		long nRef = _ThreadModel::Decrement(&m_dwRef);
		if (nRef < -(LONG_MAX / 2))
		{
			ATLASSERT(0 && _T("Release called on a pointer that has already been released"));
		}
		return nRef;
#else
		return _ThreadModel::Decrement(&m_dwRef);
#endif
	}

	HRESULT _AtlInitialConstruct()
	{
		return S_OK;
	}

	void Lock()
	{
	}
	void Unlock()
	{
	}
};

