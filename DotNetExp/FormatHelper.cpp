#include "pch.h"
#include "FormatHelper.h"
#include <CorHdr.h>

CString FormatHelper::TypeAttributesToString(DWORD attr) {
	static PCWSTR visiblity[] = {
		L"Private", L"Public", L"Nested Public", L"Nested Private",
		L"Nested Family", L"Nested Internal", L"Nested Protected and Internal", L"Nested Protected Internal"
	};

	CString text = visiblity[attr & tdVisibilityMask];
	text += L", ";

	if ((attr & tdLayoutMask) == tdSequentialLayout)
		text += L"Sequential, ";
	else if ((attr & tdLayoutMask) == tdExplicitLayout)
		text += L"Explicit, ";

	if ((attr & tdClassSemanticsMask) == tdInterface)
		text += L"Interface, ";

	if (attr & tdAbstract)
		text += L"Abstract, ";
	if (attr & tdSealed)
		text += L"Sealed, ";
	if (attr & tdSpecialName)
		text += L"Special Name, ";
	if (attr & tdImport)
		text += L"Import, ";
	if (attr & tdSerializable)
		text += L"Serializable, ";
	if (attr & tdWindowsRuntime)
		text += L"Windows Runtime, ";

	return text.Left(text.GetLength() - 2);
}
