#pragma once
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define WIN
#elif defined linux
#define LINUX
#endif

#include <iostream>
#include <fstream>

#ifdef WIN
//define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <Windows.h>
#include <TlHelp32.h>
typedef DWORD pid_t;
typedef uintptr_t mem_t;
#endif

#ifdef LINUX
typedef off_t mem_t;
#endif

#define BAD_RETURN -1

namespace Framework
{
#ifdef WIN
	namespace Utilities
	{
		unsigned int FileToArrayOfBytes(std::string filepath, char** pbuffer);
		void MultiByteToWideChar(char mbstr[], wchar_t wcbuf[], size_t max_size);
		void WideCharToMultiByte(wchar_t wcstr[], char mbbuf[], size_t max_size);
	}

	namespace Windows
	{
		namespace Memory
		{
			namespace Ex
			{
				pid_t GetPID(LPCWSTR processName);
				HANDLE GetProcessHandle(pid_t pid);
				mem_t GetModuleAddress(LPCWSTR moduleName, pid_t pid);
				mem_t GetPointer(HANDLE hProc, mem_t ptr, std::vector<mem_t> offsets);
				BOOL WriteBuffer(HANDLE hProc, mem_t address, const void* value, SIZE_T size);
				BOOL ReadBuffer(HANDLE hProc, mem_t address, void* buffer, SIZE_T size);
			}

			namespace In
			{
				HANDLE GetCurrentProcessHandle();
				pid_t GetCurrentPID();
				mem_t GetModuleAddress(LPCWSTR moduleName);
				mem_t GetPointer(mem_t baseAddress, std::vector<mem_t> offsets);
				bool WriteBuffer(mem_t address, const void* value, SIZE_T size);
				bool ReadBuffer(mem_t address, void* buffer, SIZE_T size);
				template <class type>
				type Read(mem_t address)
				{
					return *(type*)(address);
				}
				template <class type>
				void Write(mem_t address, type value)
				{
					*(type*)(address) = value;
				}
			}
		}
	}
#endif
#ifdef LINUX
#endif
}