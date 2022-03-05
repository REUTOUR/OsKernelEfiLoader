#include "EfiLoader.h"

void WriteOnlyMessage(void* message)
{
	Print((CHAR16*)message);
}

void WriteParametricMessage(void* message, UINT64 parameter1)
{
	Print((CHAR16*)message);
	Print(L"%llu;", parameter1);
}

void WriteDoubleParametricMessage(void* message, UINT64 parameter1, UINT64 parameter2)
{
	Print((CHAR16*)message);
	Print(L"%llu, %llu;", parameter1, parameter2);
}

void ExitByError(CHAR16* errorMessage)
{
	UINT16 inputBuffer;
	Print(errorMessage);
	Input(NULL, inputBuffer, 1);
	BS->Exit(Global.ImageHandle, EFI_SUCCESS, 0, NULL);
}

EFI_STATUS Main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
{
	InitializeLib(imageHandle, systemTable);
	Global.ImageHandle = imageHandle;
	Global.SystemTable = systemTable;
	Print(L"NOTIFICATION | Firmware Vendor: %s\r\n", ST->FirmwareVendor);
	Print(L"NOTIFICATION | Firmware Revision : 0x%08x\r\n", ST->FirmwareRevision);
	Global.OperationStatus = RT->GetTime(&Global.Time, NULL);
	if (EFI_ERROR(Global.OperationStatus))
	{
		ExitByError(L"ERROR        | Error getting time phase, line %llu\r\n", __LINE__);
	}
	Print(L"NOTIFICATION | It is %02hhu:%02hhu:%02hhu of %02hhu-%02hhu-%04hu\r\n", Global.Time.Hour, Global.Time.Minute, Global.Time.Second, Global.Time.Day, Global.Time.Month, Global.Time.Year);
	OpenKernelFile();
	ParseKernelFile();
	switch (Global.KernelImageFormatType)
	{
		case PEIMAGE:
		{
			LoadPeKernelImage();
			break;
		}
		case ELFIMAGE:
		{
			ExitByError(L"ERROR        | ELF images not supported, line %llu\r\n", __LINE__);
			break;
		}
		default:
		{
			ExitByError(L"ERROR        | Determined incorrect image format, line %llu\r\n", __LINE__);
			break;
		}
	}
	switch (ExploreAcpiTables())
	{
		case 0:
		{
			Print(L"NOTIFICATION | System has no RSDP\r\n");
			break;
		}
		case 1:
		{
			Print(L"NOTIFICATION | System has RSDP v1.0. No parsing supported\r\n");
			break;
		}
		case 1:
		{
			Print(L"NOTIFICATION | System has RSDP v2.0. Parsing is started\r\n");
			Global.Madt = ParseRsdp(Global.Rsdp, Global.GuidString);
			break;
		}
		default:
		{
			ExitByError(L"ERROR        | Incorrect result of RSDP looking up, line %llu\r\n", __LINE__);
			break; 
		}
	}
	ST->ConOut->ClearScreen(ST->ConOut);
	Base.Megabytes = 4096;
	Base.wom = (void*)WriteOnlyMessage;
	Base.wpm = (void*)WriteParametricMessage;
	Base.wdpm = (void*)WriteDoubleParametricMessage;
	Base.Architecture = 0ULL;
	typedef void (*EntryPointFunction)(void* base);
	EntryPointFunction EntryPointPlaceholder = (EntryPointFunction)(Global.MzHeaderMemory);
	EntryPointPlaceholder((void*)&Base);
	Global.OperationStatus = EFI_SUCCESS;
	return Global.OperationStatus;
}
