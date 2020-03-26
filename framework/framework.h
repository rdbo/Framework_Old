#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__) && !defined(linux)
#define WIN
#endif

#if defined(linux) && !defined(WIN)
#define LINUX
#endif

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <functional>

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
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/mman.h>
#define INVALID_PID -1
#define MAX_FILENAME 256

typedef off_t mem_t;
#endif

#define BAD_FUNCTION 0
#define BAD_RETURN 0
#if defined(WIN) && !defined(LINUX)
#define IS_WIN
#elif !defined(WIN) && defined(LINUX)
#define IS_LINUX
#endif

#if defined(WIN) || defined(LINUX)
namespace Framework
{
	namespace Utility
	{

		unsigned int FileToArrayOfBytes(std::string filepath, char*& pbuffer);
		void MultiByteToWideChar(char mbstr[], wchar_t wcbuf[], size_t max_size);
		void WideCharToMultiByte(wchar_t wcstr[], char mbbuf[], size_t max_size);

		namespace FunctionManager
		{
			template <class type_t>
			std::unordered_map<std::string, std::function<type_t()>> function_arr;
			template <class type_t>
			bool Register(std::string strName, std::function<type_t()> func, bool overwrite = true)
			{
				if (!overwrite && function_arr<type_t>.count(strName) > 0) return false;
				function_arr<type_t>.insert(std::pair<std::string, std::function<type_t()>>(strName, func));
				return true;
			}
			template <class type_t>
			type_t Call(std::string strName)
			{
				if (function_arr<type_t>.count(strName) == 0 || function_arr<type_t>[strName] == BAD_FUNCTION) return (type_t)BAD_RETURN;

				try
				{
					type_t ret = (type_t)function_arr<type_t>[strName]();
					return ret;
				}

				catch (const std::exception & e)
				{
					function_arr<type_t>[strName] = BAD_FUNCTION;
					return (type_t)BAD_RETURN;
				}
			}
		}
	}

	namespace Memory
	{
		void ZeroMem(void* src, size_t size);
		bool IsBadPointer(void* pointer);

		namespace Ex
		{
#if defined(WIN)
			pid_t GetProcessIdByName(LPCWSTR processName);
			pid_t GetProcessIdByWindow(LPCSTR windowName);
			pid_t GetProcessIdByWindow(LPCSTR windowClass, LPCSTR windowName);
			HANDLE GetProcessHandle(pid_t pid);
			mem_t GetModuleAddress(LPCWSTR moduleName, pid_t pid);
			mem_t GetPointer(HANDLE hProc, mem_t ptr, std::vector<mem_t> offsets);
			BOOL WriteBuffer(HANDLE hProc, mem_t address, const void* value, SIZE_T size);
			BOOL ReadBuffer(HANDLE hProc, mem_t address, void* buffer, SIZE_T size);
#elif defined(LINUX)
			pid_t GetProcessIdByName(std::string processName);
			void ReadBuffer(pid_t pid, mem_t address, void* buffer, size_t size);
			void WriteBuffer(pid_t pid, mem_t address, void* value, size_t size);
			bool IsProcessRunning(pid_t pid);
#endif
		}

		namespace In
		{
			pid_t GetCurrentProcessID();
			template <class type_t>
			type_t Read(mem_t address)
			{
				if (IsBadPointer((void*)address)) return (type_t)BAD_RETURN;
				return *(type_t*)(address);
			}
			template <class type_t>
			bool Write(mem_t address, type_t value)
			{
				if (IsBadPointer((void*)address)) return false;
				*(type_t*)(address) = value;
				return true;
			}
#if defined(WIN)
			HANDLE GetCurrentProcessHandle();
			mem_t GetModuleAddress(LPCWSTR moduleName);
			mem_t GetPointer(mem_t baseAddress, std::vector<mem_t> offsets);
			bool WriteBuffer(mem_t address, const void* value, SIZE_T size);
			bool ReadBuffer(mem_t address, void* buffer, SIZE_T size);
#elif defined(LINUX)
			bool ReadBuffer(mem_t address, void* buffer, size_t size);
			bool WriteBuffer(mem_t address, void* value, size_t size);
#endif
			namespace Hook
			{
				namespace x86
				{
#if defined(WIN)
					bool Detour(char* src, char* dst, size_t size);
					char* TrampolineHook(char* src, char* dst, size_t size);
#endif
				}
			}
		}
	}
}
#endif