#include "framework.h"

namespace Framework
{
	namespace Utilities
	{
		void ZeroMem(void* src, size_t size)
		{
			memset(src, 0x0, size + 1);
		}
		/*-----------------------------------------------------------*/
		unsigned int FileToArrayOfBytes(std::string filepath, char*& pbuffer)
		{
			std::ifstream filestream(filepath, std::ios::binary | std::ios::ate);
			if (filestream.fail()) return BAD_RETURN;

			unsigned int size = filestream.tellg();
			pbuffer = new char(size);
			ZeroMem(pbuffer, size);
			filestream.seekg(0, std::ios::beg);
			filestream.read((char*)pbuffer, size);
			filestream.close();
			return size;
		}
		/*-----------------------------------------------------------*/
		void MultiByteToWideChar(char mbstr[], wchar_t wcbuf[], size_t max_size)
		{
			mbstowcs(wcbuf, mbstr, max_size);
		}
		/*-----------------------------------------------------------*/
		void WideCharToMultiByte(wchar_t wcstr[], char mbbuf[], size_t max_size)
		{
			wcstombs(mbbuf, wcstr, max_size);
		}
	}

	namespace Memory
	{
/*---WINDOWS SPECIFIC----------------------------------------------------------*/
#if defined(WIN) && !defined(LINUX)
		bool IsBadPointer(void* pointer)
		{
			MEMORY_BASIC_INFORMATION mbi = { 0 };
			if (VirtualQuery(pointer, &mbi, sizeof(mbi)))
			{
				DWORD mask = (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
				bool b = !(mbi.Protect & mask);
				if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) b = true;

				return b;
			}
			return true;
		}
		namespace Ex
		{
			pid_t GetPID(LPCWSTR processName)
			{
				pid_t pid = 0;
				HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
				if (hSnap != INVALID_HANDLE_VALUE)
				{
					PROCESSENTRY32 procEntry;
					procEntry.dwSize = sizeof(procEntry);

					if (Process32First(hSnap, &procEntry))
					{
						do
						{
							if (!lstrcmp(procEntry.szExeFile, processName))
							{
								pid = procEntry.th32ProcessID;
								break;
							}
						} while (Process32Next(hSnap, &procEntry));

					}
				}
				CloseHandle(hSnap);
				return pid;
			}
			/*-----------------------------------------------------------*/
			mem_t GetModuleAddress(LPCWSTR moduleName, pid_t pid)
			{
				mem_t moduleAddr = NULL;
				HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
				if (hSnap != INVALID_HANDLE_VALUE)
				{
					MODULEENTRY32 modEntry;
					modEntry.dwSize = sizeof(modEntry);
					if (Module32First(hSnap, &modEntry))
					{
						do
						{
							if (!lstrcmp(modEntry.szModule, moduleName))
							{
								moduleAddr = (mem_t)modEntry.modBaseAddr;
								break;
							}
						} while (Module32Next(hSnap, &modEntry));
					}
				}
				CloseHandle(hSnap);
				return moduleAddr;
			}
			/*-----------------------------------------------------------*/
			HANDLE GetProcessHandle(pid_t pid)
			{
				return OpenProcess(PROCESS_ALL_ACCESS, NULL, pid);
			}
			/*-----------------------------------------------------------*/
			BOOL WriteBuffer(HANDLE hProc, mem_t address, const void* value, SIZE_T size)
			{
				return WriteProcessMemory(hProc, (BYTE*)address, value, size, nullptr);
			}
			/*-----------------------------------------------------------*/
			BOOL ReadBuffer(HANDLE hProc, mem_t address, void* buffer, SIZE_T size)
			{
				return ReadProcessMemory(hProc, (BYTE*)address, buffer, size, nullptr);
			}
			/*-----------------------------------------------------------*/
			mem_t GetPointer(HANDLE hProc, mem_t baseAddress, std::vector<mem_t> offsets)
			{
				mem_t addr = baseAddress;
				for (unsigned int i = 0; i < offsets.size(); ++i)
				{
					addr += offsets[i];
					ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);
				}
				return addr;
			}
			/*-----------------------------------------------------------*/
		}

		namespace In
		{
			HANDLE GetCurrentProcessHandle()
			{
				return GetCurrentProcess();
			}
			/*-----------------------------------------------------------*/
			pid_t GetCurrentPID()
			{
				return GetCurrentProcessId();
			}
			/*-----------------------------------------------------------*/
			mem_t GetModuleAddress(LPCWSTR moduleName)
			{
				return (mem_t)GetModuleHandle(moduleName);
			}
			/*-----------------------------------------------------------*/
			mem_t GetPointer(mem_t baseAddress, std::vector<mem_t> offsets)
			{
				mem_t addr = baseAddress;
				for (unsigned int i = 0; i < offsets.size(); ++i)
				{
					addr += offsets[i];
					addr = *(mem_t*)addr;
				}
				return addr;
			}
			/*-----------------------------------------------------------*/
			bool WriteBuffer(mem_t address, const void* value, SIZE_T size)
			{
				DWORD oProtection;
				if (VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &oProtection))
				{
					memcpy((void*)(address), value, size);
					VirtualProtect((LPVOID)address, size, oProtection, NULL);
					return true;
				}

				return false;
			}
			/*-----------------------------------------------------------*/
			bool ReadBuffer(mem_t address, void* buffer, SIZE_T size)
			{
				WriteBuffer((mem_t)buffer, (void*)address, size);
				return false;
			}
		}
#endif
/*---LINUX,ETC SPECIFIC----------------------------------------------------------*/
#if defined(LINUX) && !defined(WIN)
		bool IsBadPointer(void* pointer)
		{
			int fh = open((const char*)pointer, 0, 0);
			int e = errno;

			if (fh == -1 && e != EFAULT)
			{
				close(fh);
				return false;
			}

			return true;
		}

		namespace Ex
		{
			pid_t GetProcessID(std::string processName)
			{
				pid_t pid = INVALID_PID;
				DIR* pdir = opendir("/proc");
				if (!pdir)
					return INVALID_PID;

				struct dirent* pdirent;
				while (pid < 0 && (pdirent = readdir(pdir)))
				{
					int id = atoi(pdirent->d_name);
					if (id > 0)
					{
						std::string cmdpath = std::string("/proc/") + pdirent->d_name + "/cmdline";
						std::ifstream cmdfile(cmdpath.c_str());
						std::string cmdline;
						getline(cmdfile, cmdline);
						size_t pos = cmdline.find('\0');
						if (!cmdline.empty())
						{
							if (pos != std::string::npos)
								cmdline = cmdline.substr(0, pos);
							pos = cmdline.rfind('/');
							if (pos != std::string::npos)
								cmdline = cmdline.substr(pos + 1);
							if (processName == cmdline.c_str())
								pid = id;
						}
					}
				}
				closedir(pdir);
				return pid;
			}
			/*-----------------------------------------------------------*/
			void ReadBuffer(pid_t pid, mem_t address, void* buffer, size_t size)
			{
				char file[MAX_FILENAME];
				sprintf(file, "/proc/%ld/mem", (long)pid);
				int fd = open(file, O_RDWR);
				ptrace(PTRACE_ATTACH, pid, 0, 0);
				waitpid(pid, NULL, 0);
				pread(fd, buffer, size, address);
				ptrace(PTRACE_DETACH, pid, 0, 0);
				close(fd);
			}
			/*-----------------------------------------------------------*/
			void WriteBuffer(pid_t pid, mem_t address, void* value, size_t size)
			{
				char file[MAX_FILENAME];
				sprintf(file, "/proc/%ld/mem", (long)pid);
				int fd = open(file, O_RDWR);
				ptrace(PTRACE_ATTACH, pid, 0, 0);
				waitpid(pid, NULL, 0);
				pwrite(fd, value, size, address);
				ptrace(PTRACE_DETACH, pid, 0, 0);
				close(fd);
			}
			/*-----------------------------------------------------------*/
			bool IsProcessRunning(pid_t pid)
			{
				char dirbuf[MAX_FILENAME];
				sprintf(dirbuf, "/proc/%ld", (long)pid);
				struct stat status;
				stat(dirbuf, &status);
				return status.st_mode & S_IFDIR != 0;
			}
		}

		namespace In
		{
			pid_t GetCurrentProcessID()
			{
				pid_t pid = getpid();
				if (pid <= 0) pid = INVALID_PID;
				return pid;
			}
			/*-----------------------------------------------------------*/
			bool ReadBuffer(mem_t address, void* buffer, size_t size)
			{
				memcpy((void*)buffer, (void*)address, size);
				return true;
			}
			/*-----------------------------------------------------------*/
			bool WriteBuffer(mem_t address, void* value, size_t size)
			{
				memcpy((void*)address, (void*)value, size);
				return true;
			}
		}
#endif
	}
}