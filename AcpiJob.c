#include "EfiLoader.h"

UINT8 ExploreAcpiTables(void)
{
	UINT8 rsdpVersion = 0;
	EFI_STATUS status;
	status = LibGetSystemConfigurationTable(&Global.AcpiTableGuid, &Global.AcpiTable);
	if (!EFI_ERROR(status))
	{
		Print(L"SUCCESS      | Acpi table is at address: %X\r\n", Global.AcpiTable);
	}
	status = LibGetSystemConfigurationTable(&SMBIOSTableGuid, &Global.SmBiosTable);
	if (!EFI_ERROR(status))
	{
		Print(L"SUCCESS      | SmBios table is at address: %X\r\n", Global.SmBiosTable);
	}
	status = LibGetSystemConfigurationTable(&SalSystemTableGuid, &Global.SalSystemTable);
	if (!EFI_ERROR(status))
	{
		Print(L"SUCCESS      | Sal system table is at address: %X\r\n", Global.SalSystemTable);
	}
	status = LibGetSystemConfigurationTable(&MpsTableGuid, &Global.MpsTable);
	if (!EFI_ERROR(status))
	{
		Print(L"SUCCESS      | Mps table is at address: %X\r\n", Global.MpsTable);
	}
	Global.Ect = Global.SystemTable->ConfigurationTable;
	for (UINTN index = 0; index != Global.SystemTable->NumberOfTableEntries; index += 1)
	{
		if ((CompareGuid(&(Global.SystemTable->ConfigurationTable[Global.Index].VendorGuid), &Global.Acpi2TableGuid)))
		{
			ConvertGuidToString(Global.GuidString, &(Global.SystemTable->ConfigurationTable[Global.Index].VendorGuid));
			Global.Rsdp = (Acpi20RootSystemDescriptionPointer*)Global.Ect->VendorTable;
			Print(L"SUCCESS      | Rsdp v2.0 is at address: %X. Length = %u\r\n", Global.Rsdp, Global.Rsdp->Length);
			rsdpVersion = 2;
			break;
		}
		if ((CompareGuid(&(Global.SystemTable->ConfigurationTable[Global.Index].VendorGuid), &Global.AcpiTableGuid)))
		{
			Print(L"SUCCESS      | Rsdp v1.0 is at address: %X. Length = %u\r\n", Global.Rsdp, Global.Rsdp->Length);
			rsdpVersion = 1;
			break;
		}
	}
	if (Global.Rsdp == NULL)
	{
		rsdpVersion = 0;
	}
	return rsdpVersion;
}

void* ParseRsdp(Acpi20RootSystemDescriptionPointer* rsdp, CHAR16* guidString)
{
	AcpiXsdtHeader* xsdt, * entry, * madt;
	CHAR16 oemString[20];
	UINT32 entryCount;
	UINT64* entryPointer;
	int index;
	Print(L"NOTIFICATION | Acpi guid: %s\r\n", guidString);
	ConvertAsciiStringToUnicodeString((CHAR8*)(rsdp->OemId), oemString, 6);
	Print(L"SUCCESS		 | Rsdp at %X. Version: %d. OEM ID: %s\r\n", (void*)rsdp, (int)(rsdp->Revision), oemString);
	if (rsdp->Revision >= Acpi20RootSystemDescriptionPointerRevision)
	{
		xsdt = (AcpiXsdtHeader*)(rsdp->XsdtAddress);
	}
	else
	{
		Print(L"NOTIFICATION | No xsdt table found, line %llu\r\n", __LINE__);
		return NULL;
	}
	if (CompareAsciiStrings("XSDT", (CHAR8*)(VOID*)(xsdt->Signature), 4))
	{
		Print(L"NOTIFICATION | Invalid xsdt table found, line %llu\r\n", __LINE__);
		return NULL;
	}
	ConvertAsciiStringToUnicodeString((CHAR8*)(xsdt->OemId), oemString, 6);
	entryCount = (xsdt->Length - sizeof(AcpiXsdtHeader)) / sizeof(UINT64);
	Print(L"SUCCESS		 | Xsdt at %X. OEM ID: %s. Entry count: %d\r\n", (void*)xsdt, oemString, entryCount);
	entryPointer = (UINT64*)(xsdt + 1);
	for (index = 0; index < entryCount; index += 1, entryPointer += 1)
	{
		entry = (AcpiXsdtHeader*)((UINTN)(*entryPointer));
		if (entry->Signature == "APIC")
		{
			madt = entry;
		}
	}
	return madt;
}
