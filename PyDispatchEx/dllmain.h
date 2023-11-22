// dllmain.h : Declaration of module class.

class CPyDispatchExModule : public ATL::CAtlDllModuleT< CPyDispatchExModule >
{
public :
	DECLARE_LIBID(LIBID_PyDispatchExLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_PYDISPATCHEX, "{fe3f3474-e93a-4f8b-b2ee-fb03359e6c04}")
};

extern class CPyDispatchExModule _AtlModule;
