#include "Cafe/OS/common/OSCommon.h"
#include "Cafe/OS/libs/coreinit/coreinit_Misc.h"
#include "Cafe/OS/libs/coreinit/coreinit_MessageQueue.h"
#include "Cafe/CafeSystem.h"
#include "Cafe/Filesystem/fsc.h"
#include <pugixml.hpp>

namespace coreinit
{
	sint32 ppc_vprintf(const char* formatStr, char* strOut, sint32 maxLength, ppc_va_list* vargs)
	{
		char tempStr[4096];
		sint32 writeIndex = 0;
		while (*formatStr)
		{
			char c = *formatStr;
			if (c == '%')
			{
				const char* formatStart = formatStr;
				formatStr++;
				if (*formatStr == '%')
				{
					// percent sign
					if (writeIndex >= maxLength)
						break;
					strOut[writeIndex] = '%';
					writeIndex++;
					formatStr++;
					continue;
				}
				// flags
				bool flag_leftJustify = false;
				bool flag_zeroPadding = false;
				if (*formatStr == '-')
				{
					flag_leftJustify = true;
					formatStr++;
				}
				if (*formatStr == '+')
				{
					// todo
					formatStr++;
				}
				if (*formatStr == ' ')
				{
					// todo
					formatStr++;
				}
				if (*formatStr == '#')
				{
					// todo
					formatStr++;
				}
				if (*formatStr == '0')
				{
					flag_zeroPadding = true;
					formatStr++;
				}
				// width
				if (*formatStr == '*')
				{
					cemu_assert_debug(false);
					formatStr++;
				}
				bool widthIsSpecified = false;
				sint32 width = 0;
				while (*formatStr >= '0' && *formatStr <= '9')
				{
					width *= 10;
					width += (*formatStr - '0');
					formatStr++;
					widthIsSpecified = true;
				}
				// precision
				if (*formatStr == '.')
				{
					formatStr++;
					if (*formatStr == '*')
					{
						cemu_assert_debug(false);
					}
					while (*formatStr >= '0' && *formatStr <= '9')
					{
						formatStr++;
					}
				}
				// length + specifier
				char tempFormat[64];
				if (*formatStr == 'X' || *formatStr == 'x' || *formatStr == 'u' || *formatStr == 'd' || *formatStr == 'p' || *formatStr == 'i' ||
					(formatStr[0] == 'l' && formatStr[1] == 'd'))
				{
					// number
					formatStr++;
					strncpy(tempFormat, formatStart, std::min((std::ptrdiff_t)sizeof(tempFormat) - 1, formatStr - formatStart));
					if ((formatStr - formatStart) < sizeof(tempFormat))
						tempFormat[(formatStr - formatStart)] = '\0';
					else
						tempFormat[sizeof(tempFormat) - 1] = '\0';
					sint32 tempLen = sprintf(tempStr, tempFormat, (uint32)*(uint32be*)_ppc_va_arg(vargs, ppc_va_type::INT32));
					for (sint32 i = 0; i < tempLen; i++)
					{
						if (writeIndex >= maxLength)
							break;
						strOut[writeIndex] = tempStr[i];
						writeIndex++;
					}
				}
				else if (*formatStr == 's')
				{
					// string
					formatStr++;
					strncpy(tempFormat, formatStart, std::min((std::ptrdiff_t)sizeof(tempFormat) - 1, formatStr - formatStart));
					if ((formatStr - formatStart) < sizeof(tempFormat))
						tempFormat[(formatStr - formatStart)] = '\0';
					else
						tempFormat[sizeof(tempFormat) - 1] = '\0';
					MPTR strOffset = *(uint32be*)_ppc_va_arg(vargs, ppc_va_type::INT32);
					sint32 tempLen = 0;
					if (strOffset == MPTR_NULL)
						tempLen = sprintf(tempStr, "NULL");
					else
						tempLen = sprintf(tempStr, tempFormat, memory_getPointerFromVirtualOffset(strOffset));
					for (sint32 i = 0; i < tempLen; i++)
					{
						if (writeIndex >= maxLength)
							break;
						strOut[writeIndex] = tempStr[i];
						writeIndex++;
					}
					strOut[std::min(maxLength - 1, writeIndex)] = '\0';
				}
				else if (*formatStr == 'c')
				{
					// character
					formatStr++;
					strncpy(tempFormat, formatStart, std::min((std::ptrdiff_t)sizeof(tempFormat) - 1, formatStr - formatStart));
					if ((formatStr - formatStart) < sizeof(tempFormat))
						tempFormat[(formatStr - formatStart)] = '\0';
					else
						tempFormat[sizeof(tempFormat) - 1] = '\0';
					sint32 tempLen = sprintf(tempStr, tempFormat, (uint32)*(uint32be*)_ppc_va_arg(vargs, ppc_va_type::INT32));
					for (sint32 i = 0; i < tempLen; i++)
					{
						if (writeIndex >= maxLength)
							break;
						strOut[writeIndex] = tempStr[i];
						writeIndex++;
					}
				}
				else if (*formatStr == 'f' || *formatStr == 'g' || *formatStr == 'G')
				{
					formatStr++;
					strncpy(tempFormat, formatStart, std::min((std::ptrdiff_t)sizeof(tempFormat) - 1, formatStr - formatStart));
					if ((formatStr - formatStart) < sizeof(tempFormat))
						tempFormat[(formatStr - formatStart)] = '\0';
					else
						tempFormat[sizeof(tempFormat) - 1] = '\0';
					sint32 tempLen = sprintf(tempStr, tempFormat, (double)*(betype<double>*)_ppc_va_arg(vargs, ppc_va_type::FLOAT_OR_DOUBLE));
					for (sint32 i = 0; i < tempLen; i++)
					{
						if (writeIndex >= maxLength)
							break;
						strOut[writeIndex] = tempStr[i];
						writeIndex++;
					}
				}
				else if (formatStr[0] == 'l' && formatStr[1] == 'f')
				{
					// double
					formatStr += 2;
					strncpy(tempFormat, formatStart, std::min((std::ptrdiff_t)sizeof(tempFormat) - 1, formatStr - formatStart));
					if ((formatStr - formatStart) < sizeof(tempFormat))
						tempFormat[(formatStr - formatStart)] = '\0';
					else
						tempFormat[sizeof(tempFormat) - 1] = '\0';
					sint32 tempLen = sprintf(tempStr, tempFormat, (double)*(betype<double>*)_ppc_va_arg(vargs, ppc_va_type::FLOAT_OR_DOUBLE));
					for (sint32 i = 0; i < tempLen; i++)
					{
						if (writeIndex >= maxLength)
							break;
						strOut[writeIndex] = tempStr[i];
						writeIndex++;
					}
				}
				else if ((formatStr[0] == 'l' && formatStr[1] == 'l' && (formatStr[2] == 'x' || formatStr[2] == 'X')))
				{
					formatStr += 3;
					// 64bit int
					strncpy(tempFormat, formatStart, std::min((std::ptrdiff_t)sizeof(tempFormat) - 1, formatStr - formatStart));
					if ((formatStr - formatStart) < sizeof(tempFormat))
						tempFormat[(formatStr - formatStart)] = '\0';
					else
						tempFormat[sizeof(tempFormat) - 1] = '\0';
					sint32 tempLen = sprintf(tempStr, tempFormat, (uint64)*(uint64be*)_ppc_va_arg(vargs, ppc_va_type::INT64));
					for (sint32 i = 0; i < tempLen; i++)
					{
						if (writeIndex >= maxLength)
							break;
						strOut[writeIndex] = tempStr[i];
						writeIndex++;
					}
				}
				else if ((formatStr[0] == 'l' && formatStr[1] == 'l' && formatStr[2] == 'd'))
				{
					formatStr += 3;
					// signed integer (64bit)
					strncpy(tempFormat, formatStart, std::min((std::ptrdiff_t)sizeof(tempFormat) - 1, formatStr - formatStart));
					if ((formatStr - formatStart) < sizeof(tempFormat))
						tempFormat[(formatStr - formatStart)] = '\0';
					else
						tempFormat[sizeof(tempFormat) - 1] = '\0';
					sint32 tempLen = sprintf(tempStr, tempFormat, (sint64)*(sint64be*)_ppc_va_arg(vargs, ppc_va_type::INT64));
					for (sint32 i = 0; i < tempLen; i++)
					{
						if (writeIndex >= maxLength)
							break;
						strOut[writeIndex] = tempStr[i];
						writeIndex++;
					}
				}
				else
				{
					// unsupported / unknown specifier
					cemu_assert_debug(false);
					break;
				}
			}
			else
			{
				if (writeIndex >= maxLength)
					break;
				strOut[writeIndex] = c;
				writeIndex++;
				formatStr++;
			}
		}
		strOut[std::min(writeIndex, maxLength - 1)] = '\0';
		return std::min(writeIndex, maxLength - 1);
	}

	/* coreinit logging and string format */

	sint32 __os_snprintf(char* outputStr, sint32 maxLength, const char* formatStr)
	{
		ppc_define_va_list(3, 0);
		sint32 r = ppc_vprintf(formatStr, outputStr, maxLength, &vargs);
		return r;
	}

	enum class CafeLogType
	{
		OSCONSOLE = 0,
	};

	struct CafeLogBuffer
	{
		std::array<char, 270> lineBuffer;
		size_t lineLength{};
	};

	CafeLogBuffer g_logBuffer_OSReport;

	CafeLogBuffer& getLogBuffer(CafeLogType cafeLogType)
	{
		if (cafeLogType == CafeLogType::OSCONSOLE)
			return g_logBuffer_OSReport;
		// default to OSReport
		return g_logBuffer_OSReport;
	}

	std::string_view getLogBufferName(CafeLogType cafeLogType)
	{
		if (cafeLogType == CafeLogType::OSCONSOLE)
			return "OSConsole";
		return "Unknown";
	}

	std::mutex sCafeConsoleMutex;
	
	void WriteCafeConsole(CafeLogType cafeLogType, const char* msg, sint32 len)
	{
		std::unique_lock _l(sCafeConsoleMutex);
		CafeLogBuffer& logBuffer = getLogBuffer(cafeLogType);
		// once a line is full or \n is written it will be posted to log
		auto flushLine = [](CafeLogBuffer& cafeLogBuffer, std::string_view cafeLogName) -> void
		{
			cemuLog_log(LogType::CoreinitLogging, "[{0}] {1}", cafeLogName, std::basic_string_view(cafeLogBuffer.lineBuffer.data(), cafeLogBuffer.lineLength));
			cafeLogBuffer.lineLength = 0;
		};

		while (len)
		{
			char c = *msg;
			msg++;
			len--;
			if (c == '\r')
				continue;
			if (c == '\n')
			{
				// flush line immediately
				flushLine(logBuffer, getLogBufferName(cafeLogType));
				continue;
			}
			logBuffer.lineBuffer[logBuffer.lineLength] = c;
			logBuffer.lineLength++;
			if (logBuffer.lineLength >= logBuffer.lineBuffer.size())
				flushLine(logBuffer, getLogBufferName(cafeLogType));
		}
	}

	void COSVReport(COSReportModule module, COSReportLevel level, const char* format, ppc_va_list* vargs)
	{
		char tmpBuffer[1024];
		sint32 len = ppc_vprintf(format, tmpBuffer, sizeof(tmpBuffer), vargs);
		WriteCafeConsole(CafeLogType::OSCONSOLE, tmpBuffer, len);
	}

	void OSReport(const char* format)
	{
		ppc_define_va_list(1, 0);
		COSVReport(COSReportModule::coreinit, COSReportLevel::Info, format, &vargs);
	}

	void OSVReport(const char* format, ppc_va_list* vargs)
	{
		COSVReport(COSReportModule::coreinit, COSReportLevel::Info, format, vargs);
	}

	void COSWarn(int moduleId, const char* format)
	{
		ppc_define_va_list(2, 0);
		char tmpBuffer[1024];
		int prefixLen = sprintf(tmpBuffer, "[COSWarn-%d] ", moduleId);
		sint32 len = ppc_vprintf(format, tmpBuffer + prefixLen, sizeof(tmpBuffer) - prefixLen, &vargs);
		WriteCafeConsole(CafeLogType::OSCONSOLE, tmpBuffer, len + prefixLen);
	}

	void OSLogPrintf(int ukn1, int ukn2, int ukn3, const char* format)
	{
		ppc_define_va_list(4, 0);
		char tmpBuffer[1024];
		int prefixLen = sprintf(tmpBuffer, "[OSLogPrintf-%d-%d-%d] ", ukn1, ukn2, ukn3);
		sint32 len = ppc_vprintf(format, tmpBuffer + prefixLen, sizeof(tmpBuffer) - prefixLen, &vargs);
		WriteCafeConsole(CafeLogType::OSCONSOLE, tmpBuffer, len + prefixLen);
	}

	void OSConsoleWrite(const char* strPtr, sint32 length)
	{
		if (length < 0)
			return;
		WriteCafeConsole(CafeLogType::OSCONSOLE, strPtr, length);
	}

	/* home button menu */

	bool g_homeButtonMenuEnabled = false;

	bool OSIsHomeButtonMenuEnabled()
	{
		return g_homeButtonMenuEnabled;
	}

	bool OSEnableHomeButtonMenu(bool enable)
	{
		g_homeButtonMenuEnabled = enable;
		return true;
	}

	uint32 OSGetPFID()
	{
		return 15; // hardcoded as game
	}

	uint32 OSGetUPID()
	{
		return OSGetPFID();
	}

	uint64 s_currentTitleId;

	uint64 OSGetTitleID()
	{
		return s_currentTitleId;
	}

	uint32 s_sdkVersion;

	uint32 __OSGetProcessSDKVersion()
	{
		return s_sdkVersion;
	}

	// move this to CafeSystem.cpp?
	void OSLauncherThread(uint64 titleId)
	{
		CafeSystem::ShutdownTitle();
		CafeSystem::PrepareForegroundTitle(titleId);
		CafeSystem::RequestRecreateCanvas();
		CafeSystem::LaunchForegroundTitle();
	}

	uint32 __LaunchByTitleId(uint64 titleId, uint32 argc, MEMPTR<char>* argv)
	{
		// prepare argument buffer
		#if 0
		char argumentBuffer[4096];
		uint32 argumentBufferLength = 0;
		char* argWriter = argumentBuffer;
		for(uint32 i=0; i<argc; i++)
		{
			const char* arg = argv[i];
			uint32 argLength = strlen(arg);
			if((argumentBufferLength + argLength + 1) >= sizeof(argumentBuffer))
			{
				// argument buffer full
				cemuLog_logDebug(LogType::Force, "LaunchByTitleId: argument buffer full");
				return 0x80000000;
			}
			memcpy(argWriter, arg, argLength);
			argWriter[argLength] = '\0';
			argWriter += argLength + 1;
			argumentBufferLength += argLength + 1;
		}
		#endif
		// normally the above buffer is passed to the PPC kernel via syscall 0x2B and then
		// the kernel forwards it to IOSU MCP when requesting a title launch
		// but for now we HLE most of the launching code and can just set the argument array directly
		std::vector<std::string> argArray;
		for(uint32 i=0; i<argc; i++)
			argArray.emplace_back(argv[i]);
		CafeSystem::SetOverrideArgs(argArray);
		// spawn launcher thread (this current thread will be destroyed during relaunch)
		std::thread launcherThread(OSLauncherThread, titleId);
		launcherThread.detach();
		// suspend this thread
		coreinit::OSSuspendThread(coreinit::OSGetCurrentThread());
		return 0;
	}

	uint32 OSLaunchTitleByPathl(const char* path, uint32 pathLength, uint32 argc)
	{
		char appXmlPath[1024];
		cemu_assert_debug(argc == 0); // custom args not supported yet
		if(pathLength >= (sizeof(appXmlPath) - 32))
		{
			// path too long
			cemuLog_logDebug(LogType::Force, "OSLaunchTitleByPathl: path too long");
			return 0x80000000;
		}
		// read app.xml to get the titleId
		memcpy(appXmlPath, path, pathLength);
		appXmlPath[pathLength] = '\0';
		strcat(appXmlPath, "/code/app.xml");
		sint32 status;
		auto fscfile = fsc_open(appXmlPath, FSC_ACCESS_FLAG::OPEN_FILE | FSC_ACCESS_FLAG::READ_PERMISSION, &status);
		if (!fscfile)
		{
			cemuLog_logDebug(LogType::Force, "OSLaunchTitleByPathl: failed to open target app.xml");
			return 0x80000000;
		}
		uint32 size = fsc_getFileSize(fscfile);
		std::vector<uint8> tmpData(size);
		fsc_readFile(fscfile, tmpData.data(), size);
		fsc_close(fscfile);
		// parse app.xml to get the titleId
		pugi::xml_document app_doc;
		if (!app_doc.load_buffer_inplace(tmpData.data(), tmpData.size()))
			return false;
		uint64 titleId = std::stoull(app_doc.child("app").child("title_id").child_value(), nullptr, 16);
		if(titleId == 0)
		{
			cemuLog_logDebug(LogType::Force, "OSLaunchTitleByPathl: failed to parse titleId from app.xml");
			return 0x80000000;
		}
		__LaunchByTitleId(titleId, 0, nullptr);
		return 0;
	}

	uint32 OSRestartGame(uint32 argc, MEMPTR<char>* argv)
	{
		__LaunchByTitleId(CafeSystem::GetForegroundTitleId(), argc, argv);
		return 0;
	}

	void OSReleaseForeground()
	{
		cemuLog_logDebug(LogType::Force, "OSReleaseForeground not implemented");
	}

	bool s_transitionToBackground = false;
	bool s_transitionToForeground = false;

	void StartBackgroundForegroundTransition()
	{
		s_transitionToBackground = true;
		s_transitionToForeground = true;
	}

	// called at the beginning of OSReceiveMessage if the queue is the system message queue
	void UpdateSystemMessageQueue()
	{
		if(!OSIsInterruptEnabled())
			return;
		cemu_assert_debug(!__OSHasSchedulerLock());
		// normally syscall 0x2E is used to get the next message
		// for now we just have some preliminary logic here to allow a fake transition to background & foreground
		if(s_transitionToBackground)
		{
			// add transition to background message
			OSMessage msg{};
			msg.data0 = stdx::to_underlying(SysMessageId::MsgReleaseForeground);
			msg.data1 = 0; // 1 -> System is shutting down 0 -> Begin transitioning to background
			OSMessageQueue* systemMessageQueue = coreinit::OSGetSystemMessageQueue();
			if(OSSendMessage(systemMessageQueue, &msg, 0))
				s_transitionToBackground = false;
			return;
		}
		if(s_transitionToForeground)
		{
			// add transition to foreground message
			OSMessage msg{};
			msg.data0 = stdx::to_underlying(SysMessageId::MsgAcquireForeground);
			msg.data1 = 1; // ?
			msg.data2 = 1; // ?
			OSMessageQueue* systemMessageQueue = coreinit::OSGetSystemMessageQueue();
			if(OSSendMessage(systemMessageQueue, &msg, 0))
				s_transitionToForeground = false;
			return;
		}
	}

	// called when OSReceiveMessage returns a message from the system message queue
	void HandleReceivedSystemMessage(OSMessage* msg)
	{
		cemu_assert_debug(!__OSHasSchedulerLock());
		cemuLog_log(LogType::Force, "Receiving message: {:08x}", (uint32)msg->data0);
	}

	uint32 OSDriver_Register(uint32 moduleHandle, sint32 priority, OSDriverInterface* driverCallbacks, sint32 driverId, uint32be* outUkn1, uint32be* outUkn2, uint32be* outUkn3)
	{
		cemuLog_logDebug(LogType::Force, "OSDriver_Register stubbed");
		return 0;
	}

	uint32 OSDriver_Deregister(uint32 moduleHandle, sint32 driverId)
	{
		cemuLog_logDebug(LogType::Force, "OSDriver_Deregister stubbed");
		return 0;
	}

	void miscInit()
	{
		s_currentTitleId = CafeSystem::GetForegroundTitleId();
		s_sdkVersion = CafeSystem::GetForegroundTitleSDKVersion();
		s_transitionToBackground = false;
		s_transitionToForeground = false;

		cafeExportRegister("coreinit", __os_snprintf, LogType::Placeholder);

		cafeExportRegister("coreinit", COSVReport, LogType::Placeholder);
		cafeExportRegister("coreinit", COSWarn, LogType::Placeholder);
		cafeExportRegister("coreinit", OSReport, LogType::Placeholder);
		cafeExportRegister("coreinit", OSVReport, LogType::Placeholder);
		cafeExportRegister("coreinit", OSLogPrintf, LogType::Placeholder);
		cafeExportRegister("coreinit", OSConsoleWrite, LogType::Placeholder);

		cafeExportRegister("coreinit", OSGetPFID, LogType::Placeholder);
		cafeExportRegister("coreinit", OSGetUPID, LogType::Placeholder);
		cafeExportRegister("coreinit", OSGetTitleID, LogType::Placeholder);
		cafeExportRegister("coreinit", __OSGetProcessSDKVersion, LogType::Placeholder);

		g_homeButtonMenuEnabled = true; // enabled by default
		// Disney Infinity 2.0 actually relies on home button menu being enabled by default. If it's false it will crash due to calling erreula->IsAppearHomeNixSign() before initializing erreula
		cafeExportRegister("coreinit", OSIsHomeButtonMenuEnabled, LogType::CoreinitThread);
		cafeExportRegister("coreinit", OSEnableHomeButtonMenu, LogType::CoreinitThread);

		cafeExportRegister("coreinit", OSLaunchTitleByPathl, LogType::Placeholder);
		cafeExportRegister("coreinit", OSRestartGame, LogType::Placeholder);

		cafeExportRegister("coreinit", OSReleaseForeground, LogType::Placeholder);

		cafeExportRegister("coreinit", OSDriver_Register, LogType::Placeholder);
		cafeExportRegister("coreinit", OSDriver_Deregister, LogType::Placeholder);
	}

};
