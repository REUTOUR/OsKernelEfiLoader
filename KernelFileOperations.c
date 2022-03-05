#include "EfiLoader.h"

void OpenKernelFile(void)
{
	EFI_STATUS status = EFI_SUCCESS;
	EFI_LOADED_IMAGE* loadedEfiImage = NULL;
	EFI_DEVICE_PATH* devicePath = NULL;
	CHAR16* devicePathAsString = NULL;
	EFI_FILE_HANDLE rootFilesystem = NULL;
	EFI_FILE_IO_INTERFACE* volume = NULL;
	CHAR16 kernelFileName[100];
	status = BS->HandleProtocol(Global.ImageHandle, &LoadedImageProtocol, (VOID*)&loadedEfiImage);
	if (EFI_ERROR(status))
	{
		ExitByError(L"ERROR        | Failed retrieve a LoadedImageProtocol phase, line %llu\r\n", __LINE__);
	}
	status = BS->HandleProtocol(loadedEfiImage->DeviceHandle, &DevicePathProtocol, (VOID*)&devicePath);
	if (EFI_ERROR(status) || devicePath == NULL)
	{
		ExitByError(L"ERROR        | Failed retrieve a DevicePathProtocol phase, line %llu\r\n", __LINE__);
	}
	devicePathAsString = DevicePathToStr(devicePath);
	if (devicePathAsString != NULL)
	{
		Print(L"NOTIFICATION | Efi loader image device: %s\r\n", devicePathAsString);
		FreePool(devicePathAsString);
	}
	devicePathAsString = DevicePathToStr(loadedEfiImage->FilePath);
	if (devicePathAsString != NULL)
	{
		Print(L"NOTIFICATION | Efi loader image file full name: %s\r\n", DevicePathToStr(loadedEfiImage->FilePath));
		FreePool(devicePathAsString);
	}
	status = BS->HandleProtocol(loadedEfiImage->DeviceHandle, &FileSystemProtocol, (VOID*)&volume);
	if (EFI_ERROR(status))
	{
		ExitByError(L"ERROR        | Failed retrieve a FileSystemProtocol phase, line %llu\r\n", __LINE__);
	}
	status = volume->OpenVolume(volume, &rootFilesystem);
	if (EFI_ERROR(status))
	{
		ExitByError(L"ERROR        | Failed open the filesystem volume phase, line %llu\r\n", __LINE__);
	}
	devicePathAsString = DevicePathToStr(loadedEfiImage->FilePath);
	if (devicePathAsString != NULL)
	{
		StrCpy(kernelFileName, devicePathAsString);
		FreePool(devicePathAsString);
	}
	for (i = StrLen(kernelFileName); i > 0 && kernelFileName[i] != '/'; i -= 1);
	if (kernelFileName[i - 1] == '\\')
	{
		i -= 1;
	}
	kernelFileName[i] = 0;
	StrCat(kernelFileName, L"\\System\\Kernel.bin");
	status = rootFilesystem->Open(rootFilesystem, &Global.KernelFile, kernelFileName, EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(status))
	{
		ExitByError(L"ERROR        | Failed open file %s, line %llu\r\n", kernelFileName, __LINE__);
	}
}

void ParseKernelFile(void)
{
	EFI_STATUS status;
	IMAGE_DOS_HEADER dosHeader;
	UINTN size;
	status = Global.KernelFile->Read(Global.KernelFile, &size, &dosHeader);
	if (EFI_ERROR(status))
	{
		ExitByError(L"ERROR        | Failed DOS header read phase, line %llu\r\n", __LINE__);
	}
	if (dosHeader.e_magic == IMAGE_DOS_SIGNATURE)
	{
		status = Global.KernelFile->SetPosition(Global.KernelFile, (UINT64)dosHeader.e_lfanew);
		if (EFI_ERROR(status))
		{
			ExitByError(L"ERROR        | SetPosition, line %llu\r\n", __LINE__);
		}
		size = sizeof(IMAGE_NT_HEADERS);
		status = Global.KernelFile->Read(Global.KernelFile, &size, &Global.PeHeader);
		if (EFI_ERROR(status))
		{
			ExitByError(L"ERROR        | Failed PE header read phase, line %llu\r\n", __LINE__);
		}
		if (Global.PeHeader.Signature == IMAGE_NT_SIGNATURE)
		{
			if (Global.PeHeader.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
			{
				Global.KernelImageFormatType = PEIMAGE;
				if (Global.PeHeader.OptionalHeader.Subsystem != IMAGE_SUBSYSTEM_NATIVE)
				{
					ExitByError(L"ERROR        | Non-native subsystem: %hu, line %llu\r\n", Global.PeHeader.OptionalHeader.Subsystem, __LINE__);
				}
				return;
			}
			else
			{
				ExitByError(L"ERROR        | Non-8664 image, line %llu\r\n", __LINE__);
			}
		}
		else
		{
			ExitByError(L"ERROR        | Non-NT signature, line %llu\r\n", __LINE__);
		}
	}
	else
	{
		ExitByError(L"ERROR        | Reset SetPosition in non-PE image, line %llu\r\n", __LINE__);
	}
}
