#include "framework.h"

//Helper Function Definitions

#ifdef WIN
BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam);
#endif

//Variables
#ifdef WIN
HWND window;
#endif
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
#		if INCLUDE_DIRECTX && defined(WIN)
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
#			if INCLUDE_DIRECTX11
			namespace DX11
			{
				bool GetSwapchainDeviceContext(void** pSwapchainTable, size_t Size_Swapchain, void** pDeviceTable, size_t Size_Device, void** pContextTable, size_t Size_Context)
				{
					DXGI_SWAP_CHAIN_DESC swapChainDesc{ 0 };
					swapChainDesc.BufferCount = 1;
					swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
					swapChainDesc.OutputWindow = GetForegroundWindow();
					swapChainDesc.SampleDesc.Count = 1;
					swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
					swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
					swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

					D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

					IDXGISwapChain* pDummySwapChain = nullptr;
					ID3D11Device* pDummyDevice = nullptr;
					ID3D11DeviceContext* pDummyContext = nullptr;

					if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc, &pDummySwapChain, &pDummyDevice, NULL, &pDummyContext)))
					{
						swapChainDesc.Windowed = TRUE;
						if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc, &pDummySwapChain, &pDummyDevice, NULL, &pDummyContext)))
						{
							return false;
						}
					}

					if (pSwapchainTable)
					{
						memcpy(pSwapchainTable, *reinterpret_cast<void***>(pDummySwapChain), Size_Swapchain);
					}

					if (pDeviceTable)
					{
						memcpy(pDeviceTable, *reinterpret_cast<void***>(pDummyDevice), Size_Device);
					}

					if (pContextTable)
					{
						memcpy(pContextTable, *reinterpret_cast<void***>(pDummyContext), Size_Context);
					}

					pDummySwapChain->Release();
					pDummyDevice->Release();
					pDummyContext->Release();

					return true;
				}
			}
#			endif
		}
#		endif //D3D
	}
#	endif //API

#	if INCLUDE_UTILITY
	namespace Utility
	{
		size_t FileToArrayOfBytes(str_t filepath, char*& pbuffer)
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
			pid_t GetProcessIdByName(str_t processName)
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
							if (!_tcscmp(procEntry.szExeFile, processName.c_str()))
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
			pid_t GetProcessIdByWindow(str_t windowName)
			{
				pid_t pid;
				GetWindowThreadProcessId(FindWindow(NULL, windowName.c_str()), &pid);
				return pid;
			}
			pid_t GetProcessIdByWindow(str_t windowClass, str_t windowName)
			{
				pid_t pid;
				GetWindowThreadProcessId(FindWindow(windowClass.c_str(), windowName.c_str()), &pid);
				return pid;
			}
			HANDLE GetProcessHandle(pid_t pid)
			{
				return OpenProcess(PROCESS_ALL_ACCESS, NULL, pid);
			}
			mem_t GetModuleAddress(str_t moduleName, pid_t pid)
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
							if (!lstrcmp(modEntry.szModule, moduleName.c_str()))
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
			pid_t GetProcessIdByName(str_t processName)
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
						str_t cmdpath = str_t("/proc/") + pdirent->d_name + "/cmdline";
						std::ifstream cmdfile(cmdpath.c_str());
						str_t cmdline;
						getline(cmdfile, cmdline);
						size_t pos = cmdline.find('\0');
						if (!cmdline.empty())
						{
							if (pos != str_t::npos)
								cmdline = cmdline.substr(0, pos);
							pos = cmdline.rfind('/');
							if (pos != str_t::npos)
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
			mem_t GetModuleAddress(str_t moduleName)
			{
				return (mem_t)GetModuleHandle(moduleName.c_str());
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
				std::map<mem_t, std::vector<byte_t>> restore_arr;
				bool Restore(mem_t address)
				{
					if (restore_arr.count(address) <= 0) return false;
					std::vector<byte_t> obytes = restore_arr.at(address);
					WriteBuffer(address, obytes.data(), obytes.size());
					return true;
				}
#				ifdef WIN
				bool Detour(byte_t* src, byte_t* dst, size_t size)
				{
					if (size < HOOK_MIN_SIZE) return false;

					//Save stolen bytes
					byte_t* bytes = new byte_t(size);
					ZeroMem(bytes, size);
					memcpy(bytes, src, size);
					std::vector<byte_t> vbytes;
					vbytes.reserve(size);
					for (int i = 0; i < size; i++)
					{
						vbytes.insert(vbytes.begin() + i, bytes[i]);
					}
					restore_arr.insert(std::pair<mem_t, std::vector<byte_t>>((mem_t)src, vbytes));

					//Detour
					DWORD  oProtect;
					VirtualProtect(src, size, PAGE_EXECUTE_READWRITE, &oProtect);
#					if defined(ARCH_X86)
					mem_t  jmpAddr = (mem_t)(dst - (mem_t)src) - HOOK_MIN_SIZE;
					*src = JMP;
					*(mem_t*)((mem_t)src + BYTE_SIZE) = jmpAddr;
#					elif defined(ARCH_X64)
					mem_t jmpAddr = (mem_t)dst;
					*(byte_t*)src = MOV_RAX[0];
					*(byte_t*)((mem_t)src + BYTE_SIZE) = MOV_RAX[1];
					*(mem_t*)((mem_t)src + BYTE_SIZE + BYTE_SIZE) = jmpAddr;
					*(byte_t*)((mem_t)src + BYTE_SIZE + BYTE_SIZE + sizeof(mem_t)) = JMP_RAX[0];
					*(byte_t*)((mem_t)src + BYTE_SIZE + BYTE_SIZE + sizeof(mem_t) + BYTE_SIZE) = JMP_RAX[1];
#					endif
					VirtualProtect(src, size, oProtect, &oProtect);
					return true;
				}
				byte_t* TrampolineHook(byte_t* src, byte_t* dst, size_t size)
				{
					if (size < HOOK_MIN_SIZE) return 0;
					void* gateway = VirtualAlloc(0, size + HOOK_MIN_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
					memcpy(gateway, src, size);
#					if defined(ARCH_X86)
					mem_t jmpBack = ((mem_t)src - (mem_t)gateway) - HOOK_MIN_SIZE;
					*(byte_t*)((mem_t)gateway + size) = JMP;
					*(mem_t*)((mem_t)gateway + size + BYTE_SIZE) = jmpBack;
#					elif defined(ARCH_X64)
					mem_t jmpBack = (mem_t)src + size;
					//mov rax, jmpBack
					*(byte_t*)((mem_t)gateway + size) = MOV_RAX[0];
					*(byte_t*)((mem_t)gateway + size + BYTE_SIZE) = MOV_RAX[1];
					*(mem_t*)((mem_t)gateway + size + BYTE_SIZE + BYTE_SIZE) = jmpBack;

					//jmp rax
					*(byte_t*)((mem_t)gateway + size + BYTE_SIZE + BYTE_SIZE + sizeof(mem_t)) = JMP_RAX[0];
					*(byte_t*)((mem_t)gateway + size + BYTE_SIZE + BYTE_SIZE + sizeof(mem_t) + BYTE_SIZE) = JMP_RAX[1];
#					endif
					Detour(src, dst, size);
					return (byte_t*)gateway;
				}
#				endif
			}
		}
	}
#	endif //Memory
#	if INCLUDE_INJECTION
	namespace Injection
	{
#		if defined(WIN)
		namespace DynamicLinkLib
		{
			bool LoadLib(HANDLE hProc, str_t dllPath)
			{
				void* loc = VirtualAllocEx(hProc, 0, (dllPath.length() + 1) * sizeof(TCHAR), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
				WriteProcessMemory(hProc, loc, dllPath.c_str(), (dllPath.length() + 1) * sizeof(TCHAR), 0);
				HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibrary, loc, 0, 0);
				return true;
			}
		}
#		endif
	}
#	endif //Injection
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