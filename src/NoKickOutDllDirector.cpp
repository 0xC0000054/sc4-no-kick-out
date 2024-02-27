////////////////////////////////////////////////////////////////////////////
//
// This file is part of sc4-no-kick-out, a DLL Plugin for SimCity 4 that
// stops the game from kicking out lower wealth occupants.
//
// Copyright (c) 2024 Nicholas Hayes
//
// This file is licensed under terms of the MIT License.
// See LICENSE.txt for more information.
//
////////////////////////////////////////////////////////////////////////////

#include "version.h"
#include "Logger.h"
#include "cIGZCOM.h"
#include "cIGZFrameWork.h"
#include "cIGZMessageServer2.h"
#include "cIGZMessageTarget2.h"
#include "cGZPersistResourceKey.h"
#include "cIGZPersistResourceManager.h"
#include "cIGZString.h"
#include "cIGZVariant.h"
#include "cISCProperty.h"
#include "cISCPropertyHolder.h"
#include "cISC4App.h"
#include "cRZAutoRefCount.h"
#include "cRZMessage2COMDirector.h"
#include "cRZMessage2Standard.h"
#include "cRZBaseString.h"
#include "GZServPtrs.h"

#include <filesystem>
#include <string>
#include <Windows.h>
#include "wil/resource.h"
#include "wil/win32_helpers.h"

static constexpr uint32_t kSC4MessagePreCityInit = 0x26D31EC0;
static constexpr uint32_t kSC4MessagePostCityInit = 0x26D31EC1;

static constexpr uint32_t kNoKickOutDirectorID = 0x3BF47A2F;

static constexpr uint32_t kTractDeveloperKickOutLowerWealth = 0x47E2C540;

static constexpr std::string_view PluginLogFileName = "SC4NoKickOut.log";

class NoKickOutDllDirector : public cRZMessage2COMDirector
{
public:

	NoKickOutDllDirector()
	{
		std::filesystem::path dllFolderPath = GetDllFolderPath();

		std::filesystem::path logFilePath = dllFolderPath;
		logFilePath /= PluginLogFileName;

		Logger& logger = Logger::GetInstance();
		logger.Init(logFilePath, LogLevel::Error);
		logger.WriteLogFileHeader("SC4NoKickOut v" PLUGIN_VERSION_STR);
	}

	uint32_t GetDirectorID() const
	{
		return kNoKickOutDirectorID;
	}

	void PreCityInit()
	{
		Logger& logger = Logger::GetInstance();

		// The resource key is set to the TGI of the building development simulator tuning exemplar.
		cGZPersistResourceKey key(0x6534284A, 0xE7E2C2DB, 0xE8DA7677);

		cIGZPersistResourceManagerPtr pResourceManager;

		if (pResourceManager)
		{
			cRZAutoRefCount<cISCPropertyHolder> propertyHolder;
			bool valueChanged = false;

			// Load the building development simulator tuning exemplar.
			// The game will temporarily cache the loaded exemplar, which allows us
			// to modify the in-memory copy.

			bool result = pResourceManager->GetResource(
				key,
				GZIID_cISCPropertyHolder,
				propertyHolder.AsPPVoid(),
				0,
				nullptr);

			if (result)
			{
				cISCProperty* property = propertyHolder->GetProperty(kTractDeveloperKickOutLowerWealth);

				if (property)
				{
					cIGZVariant* data = property->GetPropertyValue();

					if (data)
					{
						const uint16_t type = data->GetType();

						if (type == cIGZVariant::Type::Bool)
						{
							data->SetValBool(false);
							logger.WriteLine(
								LogLevel::Info,
								"Set the 'Tract Developer Kick Out Lower Wealth' property to false.");
						}
						else
						{
							logger.WriteLineFormatted(
								LogLevel::Error,
								"The 'Tract Developer Kick Out Lower Wealth' property data has an unexpected type"
								", type=0x04x. Expected type=0x0001.",
								type);
						}
					}
					else
					{
						logger.WriteLine(
							LogLevel::Error,
							"The 'Tract Developer Kick Out Lower Wealth' property data was null.");
					}
				}
				else
				{
					logger.WriteLine(
						LogLevel::Error,
						"The 'Tract Developer Kick Out Lower Wealth' property does not exist.");
				}
			}
			else
			{
				logger.WriteLine(
					LogLevel::Error,
					"Failed to load the building development simulator tuning exemplar.");
			}
		}
		else
		{
			logger.WriteLine(
				LogLevel::Error,
				"The resource manager pointer was null.");
		}
	}


	void PostCityInit()
	{
		// The resource key is set to the TGI of the building development simulator tuning exemplar.
		cGZPersistResourceKey key(0x6534284A, 0xE7E2C2DB, 0xE8DA7677);

		cIGZPersistResourceManagerPtr pResourceManager;

		if (pResourceManager)
		{
			cRZAutoRefCount<cISCPropertyHolder> propertyHolder;
			bool valueChanged = false;

			// Load the building development simulator tuning exemplar.

			bool result = pResourceManager->GetResource(
				key,
				GZIID_cISCPropertyHolder,
				propertyHolder.AsPPVoid(),
				0,
				nullptr);

			if (result)
			{
				cISCProperty* property = propertyHolder->GetProperty(kTractDeveloperKickOutLowerWealth);

				if (property)
				{
					cIGZVariant* data = property->GetPropertyValue();

					if (data)
					{
						const uint16_t type = data->GetType();

						if (type == cIGZVariant::Type::Bool)
						{
							// Verify that the 'Tract Developer Kick Out Lower Wealth' property is
							// still set to false.
							if (data->GetValBool())
							{
								 Logger::GetInstance().WriteLine(
									LogLevel::Error,
									"Someone else changed the 'Tract Developer Kick Out Lower Wealth' property' value, cache refresh ?");
							}
						}
					}
				}
			}
		}
	}

	bool DoMessage(cIGZMessage2* pMessage)
	{
		uint32_t dwType = pMessage->GetType();

		switch (dwType)
		{
		case kSC4MessagePreCityInit:
			PreCityInit();
			break;
		case kSC4MessagePostCityInit:
			PostCityInit();
			break;
		}

		return true;
	}

	bool PostAppInit()
	{
		Logger& logger = Logger::GetInstance();

		cIGZMessageServer2Ptr pMsgServ;

		if (pMsgServ)
		{
			std::vector<uint32_t> requiredNotifications;
			requiredNotifications.push_back(kSC4MessagePreCityInit);
			requiredNotifications.push_back(kSC4MessagePostCityInit);

			for (uint32_t messageID : requiredNotifications)
			{
				if (!pMsgServ->AddNotification(this, messageID))
				{
					logger.WriteLine(LogLevel::Error, "Failed to subscribe to the required notifications.");
					return false;
				}
			}
		}
		else
		{
			logger.WriteLine(LogLevel::Error, "Failed to subscribe to the required notifications.");
			return false;
		}

		return true;
	}

	bool OnStart(cIGZCOM * pCOM)
	{
		cIGZFrameWork* const pFramework = RZGetFrameWork();

		const cIGZFrameWork::FrameworkState state = pFramework->GetState();

		if (state < cIGZFrameWork::kStatePreAppInit)
		{
			pFramework->AddHook(this);
		}
		else
		{
			PreAppInit();
		}
		return true;
	}

private:

	std::filesystem::path GetDllFolderPath()
	{
		wil::unique_cotaskmem_string modulePath = wil::GetModuleFileNameW(wil::GetModuleInstanceHandle());

		std::filesystem::path temp(modulePath.get());

		return temp.parent_path();
	}
};

cRZCOMDllDirector* RZGetCOMDllDirector() {
	static NoKickOutDllDirector sDirector;
	return &sDirector;
}