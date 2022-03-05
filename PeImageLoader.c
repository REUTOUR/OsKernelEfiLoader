#include "EfiLoader.h"

void LoadPeKernelImage()
{
	UINT64 i;
	EFI_STATUS status;
	UINTN size;
	UINT64 virtualSize = 0;
	UINT64 sectionsCount = (UINT64)Global.PeHeader.FileHeader.NumberOfSections;
	size = IMAGE_SIZEOF_SECTION_HEADER * sectionsCount;
	IMAGE_SECTION_HEADER* sectionHeadersTable;
	IMAGE_SECTION_HEADER* thisSectionHeader;
	UINTN rawDataSize;
	EFI_PHYSICAL_ADDRESS sectionAddress;
	EFI_PHYSICAL_ADDRESS allocatedMemory = 0x100000;
	UINTN headerSize = (UINTN)Global.PeHeader.OptionalHeader.SizeOfHeaders;
	UINT64 pages = EFI_SIZE_TO_PAGES(virtualSize);
	status = ST->BootServices->AllocatePool(EfiBootServicesData, size, (void**)&sectionHeadersTable);
	if (EFI_ERROR(status))
	{
		ExitByError(L"ERROR        | AllocatePool section headers table phase, line %llu\r\n", __LINE__);
	}
	status = Global.KernelFile->Read(Global.KernelFile, &size, &sectionHeadersTable[0]);
	if (EFI_ERROR(status))
	{
		ExitByError(L"ERROR        | Read section headers table phase, line %llu\r\n", __LINE__);
	}
	for (i = 0; i < sectionsCount; i++)
	{
		IMAGE_SECTION_HEADER* thisSectionHeader = &sectionHeadersTable[i];
		virtualSize = (virtualSize > (UINT64)(thisSectionHeader->VirtualAddress + thisSectionHeader->Misc.VirtualSize) ? virtualSize : (UINT64)(thisSectionHeader->VirtualAddress + thisSectionHeader->Misc.VirtualSize));
	}
	Global.KernelPages = pages;
	ZeroMem((void*)allocatedMemory, 1048576);
	if (VerifyZeroMem(pages << EFI_PAGE_SHIFT, allocatedMemory))
	{
		ExitByError(L"ERROR        | Zeroying memory phase, line %llu\r\n", __LINE__);
	}
	Print(L"NOTIFICATION | Loading image headers\r\n");
	status = Global.KernelFile->SetPosition(Global.KernelFile, 0);
	if (EFI_ERROR(status))
	{
		ExitByError(L"ERROR        | SetPosition in kernel file, line %llu\r\n", __LINE__);
	}
	status = Global.KernelFile->Read(Global.KernelFile, &headerSize, (EFI_PHYSICAL_ADDRESS*)allocatedMemory);
	if (EFI_ERROR(status))
	{
		ExitByError(L"ERROR        | Read header data phase, line %llu\r\n", __LINE__);
	}
	Print(L"NOTIFICATION | Verification:\r\n");
	Print(L"Section address : 0x%llx\r\n", allocatedMemory);
	Print(L"First 16 bytes: 0x%016llx%016llx\r\n", *(EFI_PHYSICAL_ADDRESS*)(allocatedMemory + 8), *(EFI_PHYSICAL_ADDRESS*)allocatedMemory);
	Print(L"Last 16 bytes: 0x%016llx%016llx\r\n", *(EFI_PHYSICAL_ADDRESS*)(allocatedMemory + headerSize - 8), *(EFI_PHYSICAL_ADDRESS*)(allocatedMemory + headerSize - 16));
	for (i = 0; i < sectionsCount; i++)
	{
		thisSectionHeader = &sectionHeadersTable[i];
		rawDataSize = (UINTN)thisSectionHeader->SizeOfRawData;
		sectionAddress = allocatedMemory + (UINT64)thisSectionHeader->VirtualAddress;
		status = Global.KernelFile->SetPosition(Global.KernelFile, (UINT64)thisSectionHeader->PointerToRawData);
		if (EFI_ERROR(status))
		{
			ExitByError(L"ERROR        | SetPosition, line %llu\r\n", __LINE__);
		}
		if (rawDataSize != 0)
		{
			// Some UEFI implementations can not read zero sized image sections, sadly
			status = Global.KernelFile->Read(Global.KernelFile, &rawDataSize, (EFI_PHYSICAL_ADDRESS*)sectionAddress);
			if (EFI_ERROR(status))
			{
				ExitByError(L"ERROR        | Read section phase, line %llu\r\n", __LINE__);
			}
		}
		Print(L"NOTIFICATION | Verification:\r\n");
		Print(L"Section address : 0x%llx\r\n", sectionAddress);
		Print(L"First 16 bytes: 0x%016llx%016llx\r\n", *(EFI_PHYSICAL_ADDRESS*)(sectionAddress + 8), *(EFI_PHYSICAL_ADDRESS*)sectionAddress);
		Print(L"Last 16 bytes: 0x%016llx%016llx\r\n", *(EFI_PHYSICAL_ADDRESS*)(sectionAddress + rawDataSize - 8), *(EFI_PHYSICAL_ADDRESS*)(sectionAddress + rawDataSize - 16));
	}
	if (sectionHeadersTable)
	{
		status = BS->FreePool(sectionHeadersTable);
		if (EFI_ERROR(status))
		{
			ExitByError(L"ERROR        | FreePool sectionHeadersTable phase, line %llu\r\n", __LINE__);
		}
	}
	Print(L"\nNOTIFICATION | Image loaded\r\n");
	Global.Entrypoint = allocatedMemory + (UINT64)Global.PeHeader.OptionalHeader.AddressOfEntryPoint;
}
