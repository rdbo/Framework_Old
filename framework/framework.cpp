#pragma once
#include "framework.h"

//Helper Function Definitions

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam);

//Variables

HWND window;

//Framework

#ifdef FRAMEWORK
namespace Framework
{
#	if INCLUDE_API
	namespace API
	{
#		if defined(WIN)
		namespace Windows
		{
			HWND GetCurrentWindow()
			{
				window = NULL;
				EnumWindows(EnumWindowsCallback, NULL);
				return window;
			}
		}
#		endif //Windows
#		if INCLUDE_DIRECTX
		namespace D3D
		{
#			if INCLUDE_DIRECTX9
			namespace DX9
			{
				bool GetCurrentDevice(void** vtable, size_t size)
				{
					if (!vtable)
						return false;

					IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);

					if (!pD3D)
						return false;

					IDirect3DDevice9* pDummyDevice = NULL;
					D3DPRESENT_PARAMETERS d3dpp = {};
					d3dpp.Windowed = false;
					d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
					d3dpp.hDeviceWindow = Framework::API::Windows::GetCurrentWindow();

					HRESULT dummyDeviceCreated = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDummyDevice);

					if (dummyDeviceCreated != S_OK)
					{
						d3dpp.Windowed = !d3dpp.Windowed;

						dummyDeviceCreated = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDummyDevice);

						if (dummyDeviceCreated != S_OK)
						{
							pD3D->Release();
							return false;
						}
					}

					memcpy(vtable, *reinterpret_cast<void***>(pDummyDevice), size);

					pDummyDevice->Release();
					pD3D->Release();
					return true;
				}

				namespace Draw
				{
					void FilledRectangle(int x, int y, int w, int h, D3DCOLOR color, LPDIRECT3DDEVICE9 pdevice)
					{
						D3DRECT BarRect = { x, y, x + w, y + h };
						pdevice->Clear(1, &BarRect, D3DCLEAR_TARGET | D3DCLEAR_TARGET, color, 0, 0);
					}
					void Rectangle(int x, int y, int w, int h, int thickness, D3DCOLOR color, LPDIRECT3DDEVICE9 pdevice)
					{
						FilledRectangle(x, y, w, thickness, color, pdevice);
						FilledRectangle(x, y, thickness, h, color, pdevice);
						FilledRectangle((x + w), y, thickness, h, color, pdevice);
						FilledRectangle(x, y + h, w + thickness, thickness, color, pdevice);
					}
				}
			}
#			endif //DX9
		}
#		endif //D3D
	}
#	endif //API

#	if INCLUDE_UTILITY
	namespace Utility
	{
		size_t FileToArrayOfBytes(std::string filepath, char*& pbuffer)
		{
			std::ifstream filestream(filepath, std::ios::binary | std::ios::ate);
			if (filestream.fail()) return BAD_RETURN;

			size_t size = filestream.tellg();
			pbuffer = new char(size);
			Framework::Memory::ZeroMem(pbuffer, size);
			filestream.seekg(0, std::ios::beg);
			filestream.read((char*)pbuffer, size);
			filestream.close();
			return size;
		}
		void MultiByteToWideChar(char mbstr[], wchar_t wcbuf[], size_t max_size)
		{
			mbstowcs(wcbuf, mbstr, max_size);
		}
		void WideCharToMultiByte(wchar_t wcstr[], char mbbuf[], size_t max_size)
		{
			wcstombs(mbbuf, wcstr, max_size);
		}
	}
#	endif //Utility

#	if INCLUDE_MEMORY
	namespace Memory
	{
		void ZeroMem(void* src, size_t size)
		{
			memset(src, 0x0, size + 1);
		}
		bool IsBadPointer(void* pointer)
		{
#			if defined(WIN)
			MEMORY_BASIC_INFORMATION mbi = { 0 };
			if (VirtualQuery(pointer, &mbi, sizeof(mbi)))
			{
				DWORD mask = (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
				bool b = !(mbi.Protect & mask);
				if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) b = true;

				return b;
			}
#			elif defined(LINUX)
			int fh = open((const char*)pointer, 0, 0);
			int e = errno;

			if (fh == -1 && e != EFAULT)
			{
				close(fh);
				return false;
			}
#			endif
			return true;
		}

		namespace Ex
		{
#			if defined(WIN)
			pid_t GetProcessIdByName(LPCWSTR processName)
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
			pid_t GetProcessIdByWindow(LPCSTR windowName)
			{
				pid_t pid;
				GetWindowThreadProcessId(FindWindowA(NULL, windowName), &pid);
				return pid;
			}
			pid_t GetProcessIdByWindow(LPCSTR windowClass, LPCSTR windowName)
			{
				pid_t pid;
				GetWindowThreadProcessId(FindWindowA(windowClass, windowName), &pid);
				return pid;
			}
			HANDLE GetProcessHandle(pid_t pid)
			{
				return OpenProcess(PROCESS_ALL_ACCESS, NULL, pid);
			}
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
			BOOL WriteBuffer(HANDLE hProc, mem_t address, const void* value, SIZE_T size)
			{
				return WriteProcessMemory(hProc, (BYTE*)address, value, size, nullptr);
			}
			BOOL ReadBuffer(HANDLE hProc, mem_t address, void* buffer, SIZE_T size)
			{
				return ReadProcessMemory(hProc, (BYTE*)address, buffer, size, nullptr);
			}

#			elif defined(LINUX)
			pid_t GetProcessIdByName(std::string processName)
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
			bool IsProcessRunning(pid_t pid)
			{
				char dirbuf[MAX_FILENAME];
				sprintf(dirbuf, "/proc/%ld", (long)pid);
				struct stat status;
				stat(dirbuf, &status);
				return status.st_mode & S_IFDIR != 0;
			}
#			endif //Ex
		}

		namespace In
		{
			pid_t GetCurrentProcessID()
			{
#				if defined(WIN)
				return GetCurrentProcessId();
#				elif defined(LINUX)
				pid_t pid = getpid();
				if (pid <= 0) pid = INVALID_PID;
				return pid;
#				endif
			}

#			if defined(WIN)
			HANDLE GetCurrentProcessHandle()
			{
				return GetCurrentProcess();
			}
			mem_t GetModuleAddress(LPCWSTR moduleName)
			{
				return (mem_t)GetModuleHandle(moduleName);
			}

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
			bool ReadBuffer(mem_t address, void* buffer, SIZE_T size)
			{
				WriteBuffer((mem_t)buffer, (void*)address, size);
				return true;
			}
#			elif defined(LINUX)
			bool ReadBuffer(mem_t address, void* buffer, size_t size)
			{
				memcpy((void*)buffer, (void*)address, size);
				return true;
			}
			bool WriteBuffer(mem_t address, void* value, size_t size)
			{
				memcpy((void*)address, (void*)value, size);
				return true;
			}
#			endif //In

			namespace Hook
			{
				std::map<mem_t, std::vector<char>> restore_arr;
				bool Restore(mem_t address)
				{
					if (restore_arr.count(address) <= 0) return false;
					std::vector<char> obytes = restore_arr.at(address);
					WriteBuffer(address, obytes.data(), obytes.size());
					return true;
				}
#				if defined(WIN) && defined(ARCH_X86)
				bool Detour(char* src, char* dst, size_t size)
				{
					if (size < 5) return false;
					DWORD  oProtect;
					VirtualProtect(src, size, PAGE_EXECUTE_READWRITE, &oProtect);
					mem_t  relAddr = (mem_t)(dst - (mem_t)src) - X86_JMP_SIZE;
					*src = X86_JMP;
					*(mem_t*)((mem_t)src + INSTRUCTION_SIZE) = relAddr;
					VirtualProtect(src, size, oProtect, &oProtect);
					return true;
				}
				char* TrampolineHook(char* src, char* dst, size_t size)
				{
					if (size < 5) return 0;
					void* gateway = VirtualAlloc(0, size + X86_JMP_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

					char* bytes = new char(size);
					ZeroMem(bytes, size);
					memcpy(bytes, src, size);
					std::vector<char> vbytes;
					vbytes.reserve(size);
					for (int i = 0; i < size; i++)
					{
						vbytes.insert(vbytes.begin() + i, bytes[i]);
					}
					restore_arr.insert(std::pair<mem_t, std::vector<char>>((mem_t)src, vbytes));

					memcpy(gateway, src, size);
					mem_t  gatewayRelAddr = ((mem_t)src - (mem_t)gateway) - X86_JMP_SIZE;
					*(char*)((mem_t)gateway + size) = X86_JMP;
					*(mem_t*)((mem_t)gateway + size + INSTRUCTION_SIZE) = gatewayRelAddr;
					Detour(src, dst, size);
					return (char*)gateway;
				}
#				endif //Hook
			}
		}
	}
#	endif //Memory
}
#endif //Framework

//Helper Functions
#if defined(WIN) && defined(INCLUDE_API)
BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
	DWORD wndProcId;
	GetWindowThreadProcessId(handle, &wndProcId);

	if (GetCurrentProcessId() != wndProcId)
		return TRUE;

	window = handle;
	return FALSE;
}
#endif