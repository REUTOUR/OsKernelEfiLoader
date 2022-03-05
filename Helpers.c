#include "EfiLoader.h"

UINTN CompareAsciiStrings(CHAR8* string1, CHAR8* string2, UINTN length)
{
	while (*string1 && length)
	{
		if (*string1 != *string2)
		{
			break;
		}
		string1 += 1;
		string2 += 1;
		length -= 1;
	}
	return length ? *string1 - *string2 : 0;
}

UINT8 CompareRawData(const void* left, const void* right, UINT64 length)
{
	const UINT8* l = left, * r = right;
	for (UINT64 i = 0; i != length; i += 1)
	{
		(l[i] != r[i]) ? return 0 : continue;
	}
	return 1;
}

void ConvertAsciiStringToUnicodeString(CHAR8* asciiString, CHAR16* unicodeString, UINT8 length)
{
	while (*asciiString != '\0' && length > 0)
	{
		*(unicodeString++) = (CHAR16) * (String++);
		length -= 1;
	}
	*unicodeString = '\0';
}

void ConvertGuidToString(CHAR16* buffer, EFI_GUID* guid)
{
	SPrint(buffer, 0, L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", guid->Data1, guid->Data2, guid->Data3, guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3], guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
}

UINT8 VerifyZeroMem(UINTN bytesCount, EFI_PHYSICAL_ADDRESS baseAddress)
{
	for (UINTN i = 0; i != bytesCount; i += 1)
	{
		(*(((UINT8*)baseAddress) + i) != 0) ? return 1 : continue;
	}
	return 0;
}
