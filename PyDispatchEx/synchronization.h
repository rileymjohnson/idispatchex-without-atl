#pragma once
#include "pch.h"

class ComCriticalSection
{
public:
	ComCriticalSection() throw()
	{
		std::memset(&m_sec, 0, sizeof(CRITICAL_SECTION));
	}
	~ComCriticalSection() {}
	HRESULT Lock() throw()
	{
		EnterCriticalSection(&m_sec);
		return S_OK;
	}
	HRESULT Unlock() throw()
	{
		LeaveCriticalSection(&m_sec);
		return S_OK;
	}
	HRESULT Init() throw()
	{
		HRESULT hRes = S_OK;
		if (!InitializeCriticalSectionEx(&m_sec, 0, 0))
		{
			hRes = winrt::impl::hresult_from_win32(WINRT_IMPL_GetLastError());
		}

		return hRes;
	}

	HRESULT Term() throw()
	{
		DeleteCriticalSection(&m_sec);
		return S_OK;
	}
	CRITICAL_SECTION m_sec;
};

class ComAutoCriticalSection :
	public ComCriticalSection
{
public:
	ComAutoCriticalSection()
	{
		HRESULT hr = ComCriticalSection::Init();
		winrt::check_hresult(hr);
	}
	~ComAutoCriticalSection() throw()
	{
		ComCriticalSection::Term();
	}
private:
	HRESULT Init(); // Not implemented. ComAutoCriticalSection::Init should never be called
	HRESULT Term(); // Not implemented. ComAutoCriticalSection::Term should never be called
};

class ComSafeDeleteCriticalSection :
	public ComCriticalSection
{
public:
	ComSafeDeleteCriticalSection() : m_bInitialized(false) {}

	~ComSafeDeleteCriticalSection() throw()
	{
		if (!m_bInitialized)
		{
			return;
		}
		m_bInitialized = false;
		ComCriticalSection::Term();
	}

	HRESULT Init() throw()
	{
		WINRT_ASSERT(!m_bInitialized);
		const HRESULT hr = ComCriticalSection::Init();
		if (SUCCEEDED(hr))
		{
			m_bInitialized = true;
		}
		return hr;
	}

	HRESULT Term() throw()
	{
		if (!m_bInitialized)
		{
			return S_OK;
		}
		m_bInitialized = false;
		return ComCriticalSection::Term();
	}

	HRESULT Lock()
	{
		WINRT_ASSERT(m_bInitialized);
		return ComCriticalSection::Lock();
	}

private:
	bool m_bInitialized;
};

class ComAutoDeleteCriticalSection : public ComSafeDeleteCriticalSection
{
	HRESULT Term() throw();
};

class ComFakeCriticalSection
{
public:
	HRESULT Lock() throw()
	{
		return S_OK;
	}
	HRESULT Unlock() throw()
	{
		return S_OK;
	}
	HRESULT Init() throw()
	{
		return S_OK;
	}
	HRESULT Term() throw()
	{
		return S_OK;
	}
};

class ComMultiThreadModelNoCS
{
public:
	static ULONG WINAPI Increment(_Inout_ LPLONG p) throw()
	{
		return ::InterlockedIncrement(p);
	}
	static ULONG WINAPI Decrement(_Inout_ LPLONG p) throw()
	{
		return ::InterlockedDecrement(p);
	}
	typedef ComFakeCriticalSection AutoCriticalSection;
	typedef ComFakeCriticalSection AutoDeleteCriticalSection;
	typedef ComFakeCriticalSection CriticalSection;
	typedef ComMultiThreadModelNoCS ThreadModelNoCS;
};

class ComMultiThreadModel
{
public:
	static ULONG WINAPI Increment(_Inout_ LPLONG p) throw()
	{
		return ::InterlockedIncrement(p);
	}
	static ULONG WINAPI Decrement(_Inout_ LPLONG p) throw()
	{
		return ::InterlockedDecrement(p);
	}
	typedef ComAutoCriticalSection AutoCriticalSection;
	typedef ComAutoDeleteCriticalSection AutoDeleteCriticalSection;
	typedef ComCriticalSection CriticalSection;
	typedef ComMultiThreadModelNoCS ThreadModelNoCS;
};

template< class TLock >
class ComCritSecLock
{
public:
		ComCritSecLock(TLock& cs, bool bInitialLock = true) :
			m_cs(cs),
			m_bLocked(false)
		{
			if (bInitialLock)
			{
				HRESULT hr;

				hr = Lock();
				if (FAILED(hr))
				{
					AtlThrow(hr);
				}
			}
		}
		~ComCritSecLock() throw()
		{
			if (m_bLocked)
			{
				Unlock();
			}
		}
		HRESULT Lock() throw()
		{
			HRESULT hr;

			ATLASSERT(!m_bLocked);
			ATLASSUME(!m_bLocked);
			hr = m_cs.Lock();
			if (FAILED(hr))
			{
				return(hr);
			}
			m_bLocked = true;

			return(S_OK);
		}
		void Unlock() throw()
		{
			ATLASSUME(m_bLocked);
			m_cs.Unlock();
			m_bLocked = false;
		}
private:
	TLock& m_cs;
	bool m_bLocked;
};

