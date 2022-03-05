#pragma once
#include "efi.h"
#include "efilib.h"
#include "pe.h"

typedef struct
{
	CHAR8 Signature[8];
	UINT8 Checksum;
	UINT8 OemId[6];
	UINT8 Revision;
	UINT32 RsdtAddress;
	UINT32 Length;
	UINT64 XsdtAddress;
	UINT8 ExtendedChecksum;
	UINT8 Reserved[3];
} Acpi20RootSystemDescriptionPointer;

typedef struct
{
	CHAR8 Signature[4];
	UINT32 Length;
	UINT8 Revision;
	UINT8 Checksum;
	CHAR8 OemId[6];
	CHAR8 OemTableId[8];
	UINT32 OemRevision;
	UINT32 CreatorId;
	UINT32 CreatorRevision;
} AcpiXsdtHeader;

struct
{
	long long unsigned int Megabytes;
	void* wom;
	void* wpm;
	void* wdpm;
	long long unsigned int Architecture;
} Base;

#define EFI_ACPI_TABLE_GUID { 0xeb9d2d30, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d }}
#define EFI_ACPI_20_TABLE_GUID { 0x8868e871, 0xe4f1, 0x11d3, {0xbc, 0x22, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 }} 
#define Acpi20RootSystemDescriptionPointerRevision 0x02
#define PEIMAGE 1
#define ELFIMAGE 2

struct
{
	EFI_HANDLE ImageHandle;
	EFI_SYSTEM_TABLE* SystemTable;
	EFI_FILE* KernelFile;
	UINT8 KernelImageFormatType;
	UINTN KernelPages = 0;
	EFI_PHYSICAL_ADDRESS Entrypoint;
	EFI_PHYSICAL_ADDRESS KernelBaseAddress = 0x100000;
	EFI_TIME Time;
	EFI_CONFIGURATION_TABLE* Ect;
	EFI_GUID AcpiTableGuid = EFI_ACPI_TABLE_GUID;
	EFI_GUID Acpi2TableGuid = EFI_ACPI_20_TABLE_GUID;
	VOID* AcpiTable = NULL;
	VOID* Madt = NULL;
	VOID* MpsTable = NULL;
	VOID* SalSystemTable = NULL;
	VOID* SmBiosTable = NULL;
	Acpi20RootSystemDescriptionPointer* Rsdp;
} Global;

EFI_GUID EfiFileInfoGuid = EFI_FILE_INFO_ID;

void WriteOnlyMessage(void* message);
void WriteParametricMessage(void* message, UINT64 parameter1);
void WriteDoubleParametricMessage(void* message, UINT64 parameter1, UINT64 parameter2);
void ExitByError(CHAR16* errorMessage);
UINTN CompareAsciiStrings(CHAR8* string1, CHAR8* string2, UINTN length);
UINT8 CompareRawData(const void* left, const void* right, UINT64 length);
void ConvertAsciiStringToUnicodeString(CHAR8* asciiString, CHAR16* unicodeString, UINT8 length);
void ConvertGuidToString(CHAR16* buffer, EFI_GUID* guid);
UINT8 VerifyZeroMem(UINTN bytesCount, EFI_PHYSICAL_ADDRESS baseAddress);
void OpenKernelFile(void);
void ParseKernelFile(void);
void LoadPeKernelImage(void);
void LoadElfKernelImage(void);
UINT8 ExploreAcpiTables(void);
void* ParseRsdp(Acpi20RootSystemDescriptionPointer* rsdp, CHAR16* guidString);
