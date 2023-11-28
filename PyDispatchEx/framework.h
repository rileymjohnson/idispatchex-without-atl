#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#define _ATL_APARTMENT_THREADED

#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit


#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW

#include <wil/cppwinrt.h>
#include <wil/resource.h>
#include <wil/com.h>
#include <wil/win32_helpers.h>
#include <wil/registry.h>
#include <wil/registry_helpers.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>

#include "resource.h"

#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>

#include <string>

#include <oleauto.h>
