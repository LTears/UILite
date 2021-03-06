#ifndef _H_UISCRIPT_H_
#define _H_UISCRIPT_H_

#include "UIcom.h"
#include <activscp.h>

class UIAxScriptDispatch : public IDispatch
{
protected:
	ULONG	m_nRef;
public:
	UIAxScriptDispatch():m_nRef(0) {}
	virtual ~UIAxScriptDispatch() 
	{
		ASSERT(m_nRef==0); 
	}

public:
	STDMETHODIMP QueryInterface(REFIID riid, void ** ppv)
	{
		*ppv = NULL;
		if (IID_IDispatch == riid) {
			*ppv = this;
		} else if (IID_IUnknown==riid) {
			*ppv = this;
		}
		if (NULL != *ppv) {
			((LPUNKNOWN)*ppv)->AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	STDMETHODIMP_(ULONG) AddRef(void)
	{
		return ++m_nRef;
	}
	STDMETHODIMP_(ULONG) Release(void)
	{
		if(--m_nRef==0) {
			delete this;
			return 0;
		}
		return m_nRef;
	}

	//IDispatch
	STDMETHODIMP GetTypeInfoCount(UINT* pctinfo)
	{
		return E_NOTIMPL;
	}
	STDMETHODIMP GetTypeInfo(/* [in] */ UINT iTInfo,
		/* [in] */ LCID lcid,
		/* [out] */ ITypeInfo** ppTInfo)
	{
		return E_NOTIMPL;
	}
	STDMETHODIMP GetIDsOfNames(
		/* [in] */ REFIID riid,
		/* [size_is][in] */ LPOLESTR *rgszNames,
		/* [in] */ UINT cNames,
		/* [in] */ LCID lcid,
		/* [size_is][out] */ DISPID *rgDispId)
	{
		for (UINT i = 0; i < cNames; i++) {
			rgDispId[i] = DISPID_UNKNOWN;
		}
		return ResultFromScode(DISP_E_UNKNOWNNAME);
	}
	STDMETHODIMP Invoke(
		/* [in] */ DISPID dispIdMember,
		/* [in] */ REFIID riid,
		/* [in] */ LCID lcid,
		/* [in] */ WORD wFlags,
		/* [out][in] */ DISPPARAMS  *pDispParams,
		/* [out] */ VARIANT  *pVarResult,
		/* [out] */ EXCEPINFO *pExcepInfo,
		/* [out] */ UINT *puArgErr)
	{
		return S_OK;
	}
};

class UIAxActiveScriptSiteWindow : public IActiveScriptSiteWindow
{
protected:
	DWORD m_nRef;
	HWND m_hWnd;
public:
	UIAxActiveScriptSiteWindow(HWND hWnd = NULL) : m_nRef(0), m_hWnd(hWnd)
	{}
	virtual ~UIAxActiveScriptSiteWindow() 
	{
		ASSERT(m_nRef == 0);
	}

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		*ppv = NULL;
		if(riid == IID_IActiveScriptSiteWindow) {
			*ppv = this;
		} else if (IID_IUnknown==riid) {
			*ppv = this;
		}
		if (*ppv) {
			((LPUNKNOWN)*ppv)->AddRef();
			return S_OK; 
		}
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(ULONG) AddRef(void)
	{
		return ++m_nRef;
	}

	STDMETHODIMP_(ULONG) Release(void)
	{
		if(--m_nRef == 0) {
			delete this;
			return 0;
		}
		return m_nRef;
	}

	//IActiveScriptSiteWindow
	STDMETHOD(GetWindow)(HWND *phwnd)
	{
		*phwnd = m_hWnd;
		return S_OK;
	}

	STDMETHOD(EnableModeless)(BOOL fEnable)
	{
		return S_OK;
	}
};

class UIAxActiveScriptSite : public IActiveScriptSite
{
protected:
	ULONG m_nRef;
	CComQIPtr<IActiveScriptSiteWindow> m_spWindow;
	typedef std::map<CString,CComQIPtr<IDispatch>,CStringNoCaseLess> Name2spIDispatch;
	Name2spIDispatch m_name2spDisp;
public:
	UIAxActiveScriptSite() : m_nRef(0) { }
	virtual ~UIAxActiveScriptSite() 
	{ 
		ASSERT(m_nRef==0); 
	}

	//
	HRESULT SetWindow(HWND hWnd)
	{
		m_spWindow = new UIAxActiveScriptSiteWindow(hWnd);
		return S_OK;
	}

	HRESULT AddName(LPCOLESTR lpszName, IDispatch* pDisp)
	{
		m_name2spDisp[lpszName] = pDisp;
		return S_OK;
	}

	HRESULT RemoveName(LPCOLESTR lpszName, IDispatch** ppDisp)
	{
		Name2spIDispatch::iterator it = m_name2spDisp.find(lpszName);
		if(it != m_name2spDisp.end()) {
			*ppDisp = it->second;
			return S_OK;
		}
		return E_FAIL;
	}

	// IUNKnown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		*ppv = NULL;
		if(riid == IID_IActiveScriptSite) {
			*ppv=this;
		} else if (riid == IID_IActiveScriptSiteWindow) {
			*ppv = (IActiveScriptSiteWindow*)m_spWindow;
		} else if (riid == IID_IUnknown) {
			*ppv=this;
		} 
		if (*ppv) {
			((LPUNKNOWN)*ppv)->AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	STDMETHODIMP_(ULONG) AddRef(void)
	{
		return ++m_nRef;
	}
	STDMETHODIMP_(ULONG) Release(void) 
	{
		if(--m_nRef==0) {
			delete this;
			return 0;
		}
		return m_nRef;
	}

	// IActiveScriptSite
	STDMETHOD(GetLCID)(LCID* /*plcid*/) 
	{
		return E_NOTIMPL;
	}

	STDMETHOD(GetItemInfo)(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown** ppiunkItem, ITypeInfo** ppti) 
	{
		if( (dwReturnMask & SCRIPTINFO_ITYPEINFO)!=0 ) {
			*ppti = NULL;
			return E_FAIL;
		}
		if( (dwReturnMask & SCRIPTINFO_IUNKNOWN)==0 ) 
			return E_FAIL;
		if( ppiunkItem==NULL ) 
			return E_POINTER;
		*ppiunkItem = NULL;
		Name2spIDispatch::iterator it = m_name2spDisp.find(pstrName);
		if(it != m_name2spDisp.end()) {
			IDispatch* pDisp = it->second;
			pDisp->AddRef();
			*ppiunkItem = it->second;
			return S_OK;
		}
		return E_FAIL;
	}

	STDMETHOD(GetDocVersionString)(BSTR* pbstrVersion) 
	{
		if( pbstrVersion==NULL ) 
			return E_POINTER;
		*pbstrVersion = ::SysAllocString(OLESTR("UI Script 1.0"));
		return S_OK;
	}

	STDMETHOD(OnScriptTerminate)( const VARIANT* /*pvarResult*/, const EXCEPINFO* /*pexcepinfo*/) 
	{
		return S_OK;
	}

	STDMETHOD(OnStateChange)(SCRIPTSTATE /*ssScriptState*/) 
	{
		return S_OK;
	}

	STDMETHOD(OnScriptError)(IActiveScriptError* pScriptError) 
	{
		EXCEPINFO e;
		DWORD dwContext;
		ULONG ulLine;
		LONG lPos;
		pScriptError->GetExceptionInfo(&e);
		pScriptError->GetSourcePosition(&dwContext, &ulLine, &lPos);
		TCHAR szFormat[] = _T("An error occured while parsing script: Source: %ws Error: %08X Description: %ws Line: %d");
		TCHAR szMsg[1024];
		::_stprintf(szMsg, szFormat,
			e.bstrSource,
			e.scode,
			e.bstrDescription,
			ulLine+1);
		::MessageBox(::GetActiveWindow(), szMsg, _T("Compile Error"), MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
		return S_OK;
	}

	STDMETHOD(OnEnterScript)() 
	{
		return S_OK;
	}

	STDMETHOD(OnLeaveScript)() 
	{
		return S_OK;
	}
};

template<class T>
class UIAxScriptHostT 
{
protected:
	class UIAxInnerScriptSiteWindow : public IActiveScriptSiteWindow
	{
	protected:
		ULONG m_nRef;
		T* m_pT;
	public:
		UIAxInnerScriptSiteWindow(T* pT) : m_nRef(0), m_pT(pT) { }
		virtual ~UIAxInnerScriptSiteWindow() 
		{
			ASSERT(m_nRef==0);
		}

		// IUnknown
		STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
		{
			*ppv = NULL;
			if(riid == IID_IActiveScriptSiteWindow) {
				*ppv = this;
			} else if(SUCCEEDED(m_pT->QueryInterface(riid, ppv))) {
				return S_OK;
			} else if (IID_IUnknown==riid) {
				*ppv = this;
			}
			if (*ppv) {
				((LPUNKNOWN)*ppv)->AddRef();
				return S_OK; 
			}
			return E_NOINTERFACE;
		}
		STDMETHODIMP_(ULONG) AddRef(void)
		{
			return ++m_nRef;
		}
		STDMETHODIMP_(ULONG) Release(void) 
		{
			if(--m_nRef==0) {
				delete this;
				return 0;
			}
			return m_nRef;
		}

		//IActiveScriptSiteWindow
		STDMETHOD(GetWindow)(HWND *phwnd)
		{
			*phwnd = m_pT->GetWindow();
			return S_OK;
		}

		STDMETHOD(EnableModeless)(BOOL fEnable)
		{
			return S_OK;
		}
	};
	class UIAxInnerScriptSite : public IActiveScriptSite
	{
	protected:
		ULONG m_nRef;
		typedef std::map<CString,CComQIPtr<IDispatch>,CStringNoCaseLess> Name2spIDispatch;
		Name2spIDispatch m_name2spDisp;
		T* m_pT;
	public:
		UIAxInnerScriptSite(T* pT) : m_nRef(0), m_pT(pT) { }
		virtual ~UIAxInnerScriptSite()
		{
			ASSERT(m_nRef==0);
		}

		HRESULT AddName(LPCOLESTR lpszName, IDispatch* pDisp)
		{
			m_name2spDisp[lpszName] = pDisp;
			return S_OK;
		}

		HRESULT RemoveName(LPCOLESTR lpszName, IDispatch** ppDisp)
		{
			Name2spIDispatch::iterator it = m_name2spDisp.find(lpszName);
			if(it != m_name2spDisp.end()) {
				*ppDisp = it->second;
				return S_OK;
			}
			return E_FAIL;
		}

		// IUNKnown
		STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
		{
			*ppv = NULL;
			if(riid == IID_IActiveScriptSite) {
				*ppv=this;
			} else if(SUCCEEDED(m_pT->QueryInterface(riid, ppv))) {
				return S_OK;
			} else if (riid == IID_IUnknown) {
				*ppv=this;
			} 
			if (*ppv) {
				((LPUNKNOWN)*ppv)->AddRef();
				return S_OK;
			}
			return E_NOINTERFACE;
		}
		STDMETHODIMP_(ULONG) AddRef(void)
		{
			return ++m_nRef;
		}
		STDMETHODIMP_(ULONG) Release(void) 
		{
			if(--m_nRef==0) {
				delete this;
				return 0;
			}
			return m_nRef;
		}

		// IActiveScriptSite
		STDMETHOD(GetLCID)(LCID* /*plcid*/) 
		{
			return E_NOTIMPL;
		}

		STDMETHOD(GetItemInfo)(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown** ppiunkItem, ITypeInfo** ppti) 
		{
			if( (dwReturnMask & SCRIPTINFO_ITYPEINFO)!=0 ) {
				*ppti = NULL;
				return E_FAIL;
			}
			if( (dwReturnMask & SCRIPTINFO_IUNKNOWN)==0 ) 
				return E_FAIL;
			if( ppiunkItem==NULL ) 
				return E_POINTER;
			*ppiunkItem = NULL;
			Name2spIDispatch::iterator it = m_name2spDisp.find(pstrName);
			if(it != m_name2spDisp.end()) {
				IDispatch* pDisp = it->second;
				pDisp->AddRef();
				*ppiunkItem = it->second;
				return S_OK;
			}
			return E_FAIL;
		}

		STDMETHOD(GetDocVersionString)(BSTR* pbstrVersion) 
		{
			if( pbstrVersion==NULL ) 
				return E_POINTER;
			*pbstrVersion = ::SysAllocString(OLESTR("UI Script 1.0"));
			return S_OK;
		}

		STDMETHOD(OnScriptTerminate)( const VARIANT* /*pvarResult*/, const EXCEPINFO* /*pexcepinfo*/) 
		{
			return S_OK;
		}

		STDMETHOD(OnStateChange)(SCRIPTSTATE /*ssScriptState*/) 
		{
			return S_OK;
		}

		STDMETHOD(OnScriptError)(IActiveScriptError* pScriptError) 
		{
			EXCEPINFO e;
			DWORD dwContext;
			ULONG ulLine;
			LONG lPos;
			pScriptError->GetExceptionInfo(&e);
			pScriptError->GetSourcePosition(&dwContext, &ulLine, &lPos);
			TCHAR szFormat[] = _T("An error occured while parsing script: Source: %ws Error: %08X Description: %ws Line: %d");
			TCHAR szMsg[1024];
			::_stprintf(szMsg, szFormat,
				e.bstrSource,
				e.scode,
				e.bstrDescription,
				ulLine+1);
			m_pT->MessageBox(szMsg, _T("Compile Error"), MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
			return S_OK;
		}

		STDMETHOD(OnEnterScript)() 
		{
			return S_OK;
		}

		STDMETHOD(OnLeaveScript)() 
		{
			return S_OK;
		}
	};
	BOOL m_bInited;
	CComQIPtr<IActiveScript> m_spAS;
	CComQIPtr<IActiveScriptSite> m_spASS;
	CComQIPtr<IActiveScriptSiteWindow> m_spASSW;
public:
	UIAxScriptHostT():m_bInited(FALSE) 
	{

	}
	~UIAxScriptHostT() 
	{ 
		if (m_bInited) {
			Term();
		}
	}

	HRESULT Init(BSTR code, BSTR language = L"JScript")
	{
		//JScript or VBScript
		T* pT = static_cast<T*>(this);
		ASSERT(!m_bInited);
		if (m_bInited) {
			Term();
		}
		m_bInited = TRUE;
		HRESULT hr = E_FAIL;
		hr = m_spAS.CoCreateInstance(language);
		if(m_spAS) {
			hr = E_FAIL;
			hr = pT->OnPrepareScript();
			CComQIPtr<IActiveScriptParse> spASP(m_spAS);
			if(spASP) {
				hr = spASP->InitNew();
				hr = spASP->ParseScriptText(
					code,
					NULL,
					NULL,
					NULL,
					0,
					0,
					0,
					NULL,
					NULL);
				if (SUCCEEDED(hr)) {
					//运行脚本，设置脚本引擎为SCRIPTSTATE_CONNECTED
					hr = m_spAS->SetScriptState(SCRIPTSTATE_CONNECTED);
				}
			}
		}
		return hr;
	}

	HRESULT Term()
	{
		m_bInited = FALSE;
		HRESULT hr = S_OK;
		if(m_spAS) {
			//hr = m_spAS->SetScriptState(SCRIPTSTATE_DISCONNECTED);
			hr = m_spAS->Close();
			m_spAS.Release();
		}
		if(m_spASS) {
			m_spASS.Release();
		}
		if(m_spASSW) {
			m_spASSW.Release();
		}
		return hr;
	}

	//
	IActiveScript* GetActiveScript() 
	{
		ASSERT(m_spAS);
		return m_spAS;
	}

	IActiveScriptSite* GetActiveScriptSite() 
	{
		ASSERT(m_spASS);
		return m_spASS;
	}

	IActiveScriptSiteWindow* GetActiveScriptSiteWindow()
	{
		ASSERT(m_spASSW);
		return m_spASSW;
	}

	HRESULT AddName(LPCOLESTR lpszName, IDispatch* pDisp, DWORD dwFlags = SCRIPTITEM_ISVISIBLE)
	{
		((UIAxInnerScriptSite*)(IActiveScriptSite*)m_spASS)->AddName(lpszName, pDisp);
		m_spAS->AddNamedItem(lpszName, dwFlags);
		return S_OK;
	}

	HRESULT RemoveName(LPCOLESTR lpszName, IDispatch** ppDisp)
	{
		((UIAxInnerScriptSite*)(IActiveScriptSite*)m_spASS)->RemoveName(lpszName,ppDisp);
		return S_OK;
	}

	HRESULT QueryInterface(REFIID riid, void **ppv)
	{
		T* pT = static_cast<T*>(this);
		*ppv = NULL;
		if(riid == IID_IActiveScript) {
			*ppv = pT->GetActiveScript();
		} else if(riid == IID_IActiveScriptSite) {
			*ppv = pT->GetActiveScriptSite();
		} else if (riid == IID_IActiveScriptSiteWindow) {
			*ppv = pT->GetActiveScriptSiteWindow();
		}
		if (*ppv) {
			((LPUNKNOWN)*ppv)->AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	//HRESULT ExecuteScript(BSTR code, VARIANT *ret = NULL, BSTR language = L"JScript")
	//{
	//	//JScript or VBScript
	//	T* pT = static_cast<T*>(this);
	//	HRESULT hr = E_FAIL;
	//	CComQIPtr<IActiveScript> spAS;
	//	hr = spAS.CoCreateInstance(language);
	//	if(spAS) {
	//		hr = E_FAIL;
	//		CComQIPtr<IActiveScriptParse> spASP(spAS);
	//		if(spASP) {
	//			hr = spASP->InitNew();
	//			hr = pT->OnPrepareScript(spAS);
	//			hr = spASP->ParseScriptText(
	//				code,
	//				NULL,
	//				NULL,
	//				NULL,
	//				0,
	//				0,
	//				0,
	//				NULL,
	//				NULL);
	//			hr = spAS->SetScriptState(SCRIPTSTATE_STARTED);
	//			hr = spAS->Close();
	//		}
	//	}
	//	return hr;
	//}

	HRESULT CallFunc(BSTR name, BSTR func, DISPPARAMS* params, VARIANT *pValue = NULL)
	{
		T* pT = static_cast<T*>(this);
		HRESULT hr = E_FAIL;
		//调用脚本函数
		//获得name对象,NULL表示全局对象
		CComPtr<IDispatch> spName;
		hr = m_spAS->GetScriptDispatch(name, &spName);
		//查询函数
		DISPID dispId = -1;
		hr = spName->GetIDsOfNames(IID_NULL, &func, 1, LOCALE_SYSTEM_DEFAULT, &dispId);
		//异常
		EXCEPINFO excepInfo = {0}; 
		UINT nArgErr;
		//调用函数
		hr = spName->Invoke(dispId, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, params, pValue, &excepInfo, &nArgErr);
		return hr;
	}

	HRESULT GetMemberValue(BSTR name, BSTR member, VARIANT& value)
	{
		HRESULT hr;
		//获得name对象,NULL表示全局对象
		CComPtr<IDispatch> spName;
		hr = m_spAS->GetScriptDispatch(name, &spName);
		DISPID dispId;
		hr = spName->GetIDsOfNames(IID_NULL, &member, 1, LOCALE_SYSTEM_DEFAULT, &dispId);
		if(SUCCEEDED(hr)) {
			DISPPARAMS dispparams = {NULL, NULL, 0, 0};
			EXCEPINFO excepInfo = {0}; 
			UINT nArgErr = (UINT)-1; // initialize to invalid arg
			hr = spName->Invoke(dispId, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &dispparams, &value, &excepInfo, &nArgErr);
		}
		return hr;
	}

	HRESULT SetMemberValue(BSTR name, BSTR member, VARIANT& value)
	{
		HRESULT hr;
		//获得name对象,NULL表示全局对象
		CComPtr<IDispatch> spName;
		hr = m_spAS->GetScriptDispatch(name, &spName);
		DISPID dispId;
		hr = spName->GetIDsOfNames(IID_NULL, &member, 1, LOCALE_SYSTEM_DEFAULT, &dispId);
		if(SUCCEEDED(hr)) {
			DISPID dispidPut = DISPID_PROPERTYPUT; // put操作
			VARIANTARG varg[1] = {0};
			DISPPARAMS dispparams = {0}; 
			dispparams.rgvarg = varg;
			dispparams.rgvarg[0] = value;	// 数据
			dispparams.cArgs = 1;			// 参数数量
			dispparams.cNamedArgs = 1;		// 参数名称
			dispparams.rgdispidNamedArgs = &dispidPut;	// 操作DispId，表明本参数适用于put操作
			EXCEPINFO excepInfo = {0};
			UINT nArgErr = (UINT)-1; // initialize to invalid arg
			hr = spName->Invoke(dispId, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &dispparams, NULL, &excepInfo, &nArgErr);
			//hr = spName->Invoke(dispId, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYPUT, &dispparams, &value, &excepInfo, &nArgErr);
		}
		return hr;
	}

	HRESULT GetArrayLength(IDispatch* pDisp, int & nLength)
	{
		HRESULT hr;
		BSTR varName = L"length";
		DISPID dispId;
		hr = pDisp->GetIDsOfNames(IID_NULL, &varName, 1, LOCALE_USER_DEFAULT, &dispId);
		if(SUCCEEDED(hr)) {
			DISPPARAMS dispparams = {NULL, NULL, 0, 0};
			VARIANT value;
			hr = pDisp->Invoke(dispId, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &dispparams, &value, NULL, NULL);
			if(SUCCEEDED(hr)) {
				nLength = value.intVal;
			}
		}
		return hr;
	}
	
	HRESULT GetArrayValueByIndex(IDispatch* pDisp, int index, VARIANT& value)
	{
		HRESULT hr;
		CComVariant varName(index, VT_I4); // 数组下标
		varName.ChangeType(VT_BSTR); // 将数组下标转为数字型，以进行GetIDsOfNames
		DISPID dispId;
		//
		// 获取通过下标访问数组的过程，将过程名保存在dispId中
		//
		hr = pDisp->GetIDsOfNames(IID_NULL, &varName.bstrVal, 1, LOCALE_USER_DEFAULT, &dispId);
		if(SUCCEEDED(hr)) {
			//
			// 调用COM过程，访问指定下标数组元素，根据dispId 将元素值保存在varValue中
			//
			DISPPARAMS dispparams = {NULL, NULL, 0, 0};
			hr = pDisp->Invoke(dispId, IID_NULL,LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &dispparams, &value, NULL,NULL);
		}
		return hr;
	}

	HRESULT SetArrayValueByIndex(IDispatch* pDisp, int index, VARIANT& value)
	{
		HRESULT hr;
		CComVariant varName(index, VT_I4);
		varName.ChangeType(VT_BSTR); // 将数组下标转为数字型，以进行GetIDsOfNames
		DISPID dispId = 0;
		hr = pDisp->GetIDsOfNames(IID_NULL, &varName.bstrVal, 1, LOCALE_USER_DEFAULT, &dispId);
		if (SUCCEEDED(hr)) {
			DISPID dispidPut = DISPID_PROPERTYPUT; // put操作
			VARIANTARG varg[1] = {0};
			DISPPARAMS dispparams = {0}; 
			dispparams.rgvarg = varg;
			dispparams.rgvarg[0] = value;	// 数据
			dispparams.cArgs = 1;			// 参数数量
			dispparams.cNamedArgs = 1;		// 参数名称
			dispparams.rgdispidNamedArgs = &dispidPut;	// 操作DispId，表明本参数适用于put操作
			hr = pDisp->Invoke(dispId, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &dispparams, NULL, NULL, NULL);
		}
		return hr;
	}

	/*HRESULT GetArrayValue(BSTR name, VARIANT lstCode, VARIANT lstValue, int iType, VARIANT_BOOL*pVal)
	{
		*pVal = VARIANT_FALSE;
		HRESULT hr;
		if(lstCode.vt == VT_DISPATCH && lstValue.vt == VT_DISPATCH ) //处理JS数组
		{
			//MessageBox(L"BeginToSetNo1");
			int iCodeLen,iValueLen;
			GetArrayLength(lstCode.pdispVal,iCodeLen);
			GetArrayLength(lstValue.pdispVal,iValueLen);
			VARIANT vCode,vValue;
			for(int i=0; i<iCodeLen; i++)
			{
				GetArrayValueByIndex(lstCode.pdispVal,i,vCode);
				GetArrayValueByIndex(lstValue.pdispVal,i,vValue);
				//vParam[1] =CComVariant(vCode.bstrVal);
				//vParam[0] =CComVariant(vValue.bstrVal);
			?}
			?*pVal = VARIANT_TRUE;
		} else { //处理VB数组等
			if((lstCode.vt^VT_ARRAY) <1 || (lstCode.vt^VT_ARRAY)>73)//用户传入数组类型不正确直接返回
				return S_OK;
			if((lstValue.vt^VT_ARRAY) <1 || (lstValue.vt^VT_ARRAY)>73)//用户传入数组类型不正确直接返回
				return S_OK;
			try {
				long dim1 = SafeArrayGetDim(lstCode.parray);
				long dim2 = SafeArrayGetDim(lstValue.parray);
				long ubound;
				long lbound;
				
				SafeArrayGetUBound(lstCode.parray,dim1,&ubound);
				SafeArrayGetLBound(lstCode.parray,dim1,&lbound);
																	
				BSTR* buf1,*buf2;
				SafeArrayAccessData(lstCode.parray,(void**)&buf1);
				SafeArrayAccessData(lstValue.parray,(void**)&buf2);
				for(int i=lbound,j=ubound-lbound+1; i<j; i++)
				{
					//vParam[1] =CComVariant(buf1[i]);
					//vParam[0] =CComVariant(buf2[i]);	
				}
				SafeArrayUnaccessData(lstCode.parray); // slove?the delphi problem "variant or safe array is locked"
				SafeArrayUnaccessData(lstValue.parray);

				*pVal =VARIANT_TRUE;
			}
			catch(...)
			{
				*pVal =VARIANT_FALSE;
			}
		}
		return S_OK;
	}*/

	HRESULT OnPrepareScript()
	{
		T* pT = static_cast<T*>(this);
		if (!m_spASSW) {
			m_spASSW = new UIAxInnerScriptSiteWindow(pT);
		}
		if (!m_spASS) {
			m_spASS = new UIAxInnerScriptSite(pT);
		}
		return m_spAS->SetScriptSite(m_spASS);
		return S_OK;
	}
};

#endif//_H_UISCRIPT_H_