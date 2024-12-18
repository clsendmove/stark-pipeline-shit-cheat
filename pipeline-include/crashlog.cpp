#include "crashlog.hpp"

#include <ImageHlp.h>
#include <deque>
#include <sstream>
#include <fstream>
#include <format>
#include "../pipeline-source/factory/factory.hpp"
#pragma comment(lib, "imagehlp.lib")

struct Frame
{
	std::string m_sModule = "";
	uintptr_t m_pAddress = 0;
	std::string m_sFile = "";
	unsigned int m_uLine = 0;
	std::string m_sName = "";
};

static std::deque<Frame> StackTrace(PCONTEXT context)
{
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();

	if (!SymInitialize(hProcess, nullptr, TRUE))
		return {};

	SymSetOptions(SYMOPT_LOAD_LINES);

	STACKFRAME64 frame = {};
	frame.AddrPC.Offset = context->Rip;
	frame.AddrFrame.Offset = context->Rbp;
	frame.AddrStack.Offset = context->Rsp;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Mode = AddrModeFlat;

	std::deque<Frame> vTrace = {};
	while (StackWalk64(IMAGE_FILE_MACHINE_AMD64, hProcess, hThread, &frame, context, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
	{
		Frame tFrame = {};

		tFrame.m_pAddress = frame.AddrPC.Offset;

		if (auto hBase = HINSTANCE(SymGetModuleBase64(hProcess, frame.AddrPC.Offset)))
		{
			char buf[MAX_PATH];
			if (GetModuleFileNameA(hBase, buf, MAX_PATH))
			{
				tFrame.m_sModule = std::format("{}, {:#x}", buf, uintptr_t(hBase));
				auto find = tFrame.m_sModule.rfind("\\");
				if (find != std::string::npos)
					tFrame.m_sModule.replace(0, find + 1, "");
			}
			else
				tFrame.m_sModule = std::format("{:#x}", uintptr_t(hBase));
		}

		{
			DWORD dwOffset = 0;
			IMAGEHLP_LINE64 line = {};
			line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
			if (SymGetLineFromAddr64(hProcess, frame.AddrPC.Offset, &dwOffset, &line))
			{
				tFrame.m_sFile = line.FileName;
				tFrame.m_uLine = line.LineNumber;
				auto find = tFrame.m_sFile.rfind("\\");
				if (find != std::string::npos)
					tFrame.m_sFile.replace(0, find + 1, "");
			}
		}

		{
			uintptr_t dwOffset = 0;
			char buf[sizeof(IMAGEHLP_SYMBOL64) + 255];
			auto symbol = PIMAGEHLP_SYMBOL64(buf);
			symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64) + 255;
			symbol->MaxNameLength = 254;
			if (SymGetSymFromAddr64(hProcess, frame.AddrPC.Offset, &dwOffset, symbol))
				tFrame.m_sName = symbol->Name;
		}

		vTrace.push_back(tFrame);
	}
	//if (!vTrace.empty())
	//	vTrace.pop_front();

	SymCleanup(hProcess);

	return vTrace;
}

static LONG APIENTRY ExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo)
{
	static LPVOID pLastAddress = nullptr;
	static bool bException = false;

	// unsure of a way to filter nonfatal exceptions
	if (ExceptionInfo->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION
		|| ExceptionInfo->ExceptionRecord->ExceptionAddress && ExceptionInfo->ExceptionRecord->ExceptionAddress == pLastAddress
		|| bException && GetAsyncKeyState(VK_SHIFT) & 0x8000 && GetAsyncKeyState(VK_RETURN) & 0x8000)
		return EXCEPTION_EXECUTE_HANDLER;
	pLastAddress = ExceptionInfo->ExceptionRecord->ExceptionAddress;

	std::stringstream ssErrorStream;
	ssErrorStream << std::format("Error: {:#X}\n", ExceptionInfo->ExceptionRecord->ExceptionCode);
	ssErrorStream << std::format("Address: {:#X}\n\n", uintptr_t(ExceptionInfo->ExceptionRecord->ExceptionAddress));
	ssErrorStream << std::format("RIP: {:#x}\n", ExceptionInfo->ContextRecord->Rip);
	ssErrorStream << std::format("RAX: {:#x}\n", ExceptionInfo->ContextRecord->Rax);
	ssErrorStream << std::format("RCX: {:#x}\n", ExceptionInfo->ContextRecord->Rcx);
	ssErrorStream << std::format("RDX: {:#x}\n", ExceptionInfo->ContextRecord->Rdx);
	ssErrorStream << std::format("RBX: {:#x}\n", ExceptionInfo->ContextRecord->Rbx);
	ssErrorStream << std::format("RSP: {:#x}\n", ExceptionInfo->ContextRecord->Rsp);
	ssErrorStream << std::format("RBP: {:#x}\n", ExceptionInfo->ContextRecord->Rbp);
	ssErrorStream << std::format("RSI: {:#x}\n", ExceptionInfo->ContextRecord->Rsi);
	ssErrorStream << std::format("RDI: {:#x}\n\n", ExceptionInfo->ContextRecord->Rdi);

	auto vTrace = StackTrace(ExceptionInfo->ContextRecord);
	if (!vTrace.empty())
	{
		for (auto& tFrame : vTrace)
		{
			ssErrorStream << std::format("{:#x}", tFrame.m_pAddress);
			if (!tFrame.m_sModule.empty())
				ssErrorStream << std::format(" ({})", tFrame.m_sModule);
			if (!tFrame.m_sFile.empty())
				ssErrorStream << std::format(" ({} L{})", tFrame.m_sFile, tFrame.m_uLine);
			if (!tFrame.m_sName.empty())
				ssErrorStream << std::format(" ({})", tFrame.m_sName);
			ssErrorStream << "\n";
		}
		ssErrorStream << "\n";
	}

	ssErrorStream << "Ctrl + C to copy. Logged to pipeline\\crash_log.txt. \n";
	ssErrorStream << "Built @ " __DATE__ ", " __TIME__;
	if (bException)
		ssErrorStream << "\nShift + Enter to skip repetitive exceptions. ";
	bException = true;

	MessageBox(nullptr, ssErrorStream.str().c_str(), "Unhandled exception", MB_OK | MB_ICONERROR);

	ssErrorStream << "\n\n\n\n";
	std::ofstream file;
	file.open( factory->configpath + "\\crash_log.txt", std::ios_base::app);
	file << ssErrorStream.str();
	file.close();

	return EXCEPTION_EXECUTE_HANDLER;
}

void Crashlog::Setup()
{
	AddVectoredExceptionHandler(1, ExceptionFilter);
}