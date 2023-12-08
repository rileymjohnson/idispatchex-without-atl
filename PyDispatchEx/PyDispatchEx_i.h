

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Mon Jan 18 22:14:07 2038
 */
/* Compiler settings for PyDispatchEx.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0628 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __PyDispatchEx_i_h__
#define __PyDispatchEx_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef DECLSPEC_XFGVIRT
#if defined(_CONTROL_FLOW_GUARD_XFG)
#define DECLSPEC_XFGVIRT(base, func) __declspec(xfg_virtual(base, func))
#else
#define DECLSPEC_XFGVIRT(base, func)
#endif
#endif

/* Forward Declarations */ 

#ifndef __IPyDispatchExObject_FWD_DEFINED__
#define __IPyDispatchExObject_FWD_DEFINED__
typedef interface IPyDispatchExObject IPyDispatchExObject;

#endif 	/* __IPyDispatchExObject_FWD_DEFINED__ */


#ifndef __PyDispatchExObject_FWD_DEFINED__
#define __PyDispatchExObject_FWD_DEFINED__

#ifdef __cplusplus
typedef class PyDispatchExObject PyDispatchExObject;
#else
typedef struct PyDispatchExObject PyDispatchExObject;
#endif /* __cplusplus */

#endif 	/* __PyDispatchExObject_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "dispex.h"
#include "shobjidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_PyDispatchEx_0000_0000 */
/* [local] */ 

const WORD VersionMajor_PyDispatchExLib = 1;
const WORD VersionMinor_PyDispatchExLib = 0;


extern RPC_IF_HANDLE __MIDL_itf_PyDispatchEx_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_PyDispatchEx_0000_0000_v0_0_s_ifspec;

#ifndef __IPyDispatchExObject_INTERFACE_DEFINED__
#define __IPyDispatchExObject_INTERFACE_DEFINED__

/* interface IPyDispatchExObject */
/* [unique][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IPyDispatchExObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("76d7b503-f2fa-4334-9b45-519e27e9caeb")
    IPyDispatchExObject : public IDispatchEx
    {
    public:
        virtual /* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE TestMethod1( 
            /* [in] */ VARIANT one,
            /* [in] */ VARIANT two,
            /* [in] */ VARIANT three,
            /* [in] */ VARIANT four,
            /* [in] */ VARIANT five,
            /* [retval][out] */ VARIANT *out_value) = 0;
        
        virtual /* [helpcontext][helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TestProperty1( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpcontext][helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TestProperty1( 
            /* [in] */ VARIANT newVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IPyDispatchExObjectVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPyDispatchExObject * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPyDispatchExObject * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPyDispatchExObject * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPyDispatchExObject * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPyDispatchExObject * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPyDispatchExObject * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPyDispatchExObject * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IDispatchEx, GetDispID)
        HRESULT ( STDMETHODCALLTYPE *GetDispID )( 
            IPyDispatchExObject * This,
            /* [in] */ BSTR bstrName,
            /* [in] */ DWORD grfdex,
            /* [out] */ DISPID *pid);
        
        DECLSPEC_XFGVIRT(IDispatchEx, InvokeEx)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *InvokeEx )( 
            IPyDispatchExObject * This,
            /* [annotation][in] */ 
            _In_  DISPID id,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][in] */ 
            _In_  DISPPARAMS *pdp,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pvarRes,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pei,
            /* [annotation][unique][in] */ 
            _In_opt_  IServiceProvider *pspCaller);
        
        DECLSPEC_XFGVIRT(IDispatchEx, DeleteMemberByName)
        HRESULT ( STDMETHODCALLTYPE *DeleteMemberByName )( 
            IPyDispatchExObject * This,
            /* [in] */ BSTR bstrName,
            /* [in] */ DWORD grfdex);
        
        DECLSPEC_XFGVIRT(IDispatchEx, DeleteMemberByDispID)
        HRESULT ( STDMETHODCALLTYPE *DeleteMemberByDispID )( 
            IPyDispatchExObject * This,
            /* [in] */ DISPID id);
        
        DECLSPEC_XFGVIRT(IDispatchEx, GetMemberProperties)
        HRESULT ( STDMETHODCALLTYPE *GetMemberProperties )( 
            IPyDispatchExObject * This,
            /* [in] */ DISPID id,
            /* [in] */ DWORD grfdexFetch,
            /* [out] */ DWORD *pgrfdex);
        
        DECLSPEC_XFGVIRT(IDispatchEx, GetMemberName)
        HRESULT ( STDMETHODCALLTYPE *GetMemberName )( 
            IPyDispatchExObject * This,
            /* [in] */ DISPID id,
            /* [out] */ BSTR *pbstrName);
        
        DECLSPEC_XFGVIRT(IDispatchEx, GetNextDispID)
        HRESULT ( STDMETHODCALLTYPE *GetNextDispID )( 
            IPyDispatchExObject * This,
            /* [in] */ DWORD grfdex,
            /* [in] */ DISPID id,
            /* [out] */ DISPID *pid);
        
        DECLSPEC_XFGVIRT(IDispatchEx, GetNameSpaceParent)
        HRESULT ( STDMETHODCALLTYPE *GetNameSpaceParent )( 
            IPyDispatchExObject * This,
            /* [out] */ IUnknown **ppunk);
        
        DECLSPEC_XFGVIRT(IPyDispatchExObject, TestMethod1)
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *TestMethod1 )( 
            IPyDispatchExObject * This,
            /* [in] */ VARIANT one,
            /* [in] */ VARIANT two,
            /* [in] */ VARIANT three,
            /* [in] */ VARIANT four,
            /* [in] */ VARIANT five,
            /* [retval][out] */ VARIANT *out_value);
        
        DECLSPEC_XFGVIRT(IPyDispatchExObject, get_TestProperty1)
        /* [helpcontext][helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TestProperty1 )( 
            IPyDispatchExObject * This,
            /* [retval][out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(IPyDispatchExObject, put_TestProperty1)
        /* [helpcontext][helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TestProperty1 )( 
            IPyDispatchExObject * This,
            /* [in] */ VARIANT newVal);
        
        END_INTERFACE
    } IPyDispatchExObjectVtbl;

    interface IPyDispatchExObject
    {
        CONST_VTBL struct IPyDispatchExObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPyDispatchExObject_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPyDispatchExObject_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPyDispatchExObject_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPyDispatchExObject_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IPyDispatchExObject_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IPyDispatchExObject_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IPyDispatchExObject_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IPyDispatchExObject_GetDispID(This,bstrName,grfdex,pid)	\
    ( (This)->lpVtbl -> GetDispID(This,bstrName,grfdex,pid) ) 

#define IPyDispatchExObject_InvokeEx(This,id,lcid,wFlags,pdp,pvarRes,pei,pspCaller)	\
    ( (This)->lpVtbl -> InvokeEx(This,id,lcid,wFlags,pdp,pvarRes,pei,pspCaller) ) 

#define IPyDispatchExObject_DeleteMemberByName(This,bstrName,grfdex)	\
    ( (This)->lpVtbl -> DeleteMemberByName(This,bstrName,grfdex) ) 

#define IPyDispatchExObject_DeleteMemberByDispID(This,id)	\
    ( (This)->lpVtbl -> DeleteMemberByDispID(This,id) ) 

#define IPyDispatchExObject_GetMemberProperties(This,id,grfdexFetch,pgrfdex)	\
    ( (This)->lpVtbl -> GetMemberProperties(This,id,grfdexFetch,pgrfdex) ) 

#define IPyDispatchExObject_GetMemberName(This,id,pbstrName)	\
    ( (This)->lpVtbl -> GetMemberName(This,id,pbstrName) ) 

#define IPyDispatchExObject_GetNextDispID(This,grfdex,id,pid)	\
    ( (This)->lpVtbl -> GetNextDispID(This,grfdex,id,pid) ) 

#define IPyDispatchExObject_GetNameSpaceParent(This,ppunk)	\
    ( (This)->lpVtbl -> GetNameSpaceParent(This,ppunk) ) 


#define IPyDispatchExObject_TestMethod1(This,one,two,three,four,five,out_value)	\
    ( (This)->lpVtbl -> TestMethod1(This,one,two,three,four,five,out_value) ) 

#define IPyDispatchExObject_get_TestProperty1(This,pVal)	\
    ( (This)->lpVtbl -> get_TestProperty1(This,pVal) ) 

#define IPyDispatchExObject_put_TestProperty1(This,newVal)	\
    ( (This)->lpVtbl -> put_TestProperty1(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPyDispatchExObject_INTERFACE_DEFINED__ */



#ifndef __PyDispatchExLib_LIBRARY_DEFINED__
#define __PyDispatchExLib_LIBRARY_DEFINED__

/* library PyDispatchExLib */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_PyDispatchExLib;

EXTERN_C const CLSID CLSID_PyDispatchExObject;

#ifdef __cplusplus

class DECLSPEC_UUID("bfdfa386-9555-4a4d-8d3e-5b48417b92a4")
PyDispatchExObject;
#endif
#endif /* __PyDispatchExLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

unsigned long             __RPC_USER  VARIANT_UserSize64(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal64(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal64(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree64(     unsigned long *, VARIANT * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


