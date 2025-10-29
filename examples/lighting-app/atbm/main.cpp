/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "DeviceCallbacks.h"

#include "AppTask.h"
#include <common/CHIPDeviceManager.h>
#include <common/ATBMAppServer.h>

#include <app/server/Dnssd.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <platform/atbm/ATBMUtils.h>

#if CONFIG_ENABLE_ATBM_FACTORY_DATA_PROVIDER
#include <platform/atbm/ATBMFactoryDataProvider.h>
#else
#include "CommissionableInit.h"
#endif // CONFIG_ENABLE_ATBM_FACTORY_DATA_PROVIDER

#if CONFIG_ENABLE_ATBM_DEVICE_INFO_PROVIDER
#include <platform/atbm/ATBMDeviceInfoProvider.h>
#else
#include <DeviceInfoProviderImpl.h>
#endif // CONFIG_ENABLE_ATBM_DEVICE_INFO_PROVIDER

#if CONFIG_SEC_CERT_DAC_PROVIDER
#include <platform/atbm/ATBMSecureCertDACProvider.h>
#endif

#include "launch_shell.h"
#include "atbm_general.h"
#include "easyflash.h"

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;

static AppDeviceCallbacks EchoCallbacks;
static AppDeviceCallbacksDelegate sAppDeviceCallbacksDelegate;

namespace {
#if CONFIG_ENABLE_ATBM_FACTORY_DATA_PROVIDER
DeviceLayer::ATBMFactoryDataProvider sFactoryDataProvider;
#else
ExampleCommissionableDataProvider gExampleCommissionableDataProvider;
#endif // CONFIG_ENABLE_ATBM_FACTORY_DATA_PROVIDER

#if CONFIG_ENABLE_ATBM_DEVICE_INFO_PROVIDER
DeviceLayer::ATBMDeviceInfoProvider gExampleDeviceInfoProvider;
#else
DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;
#endif // CONFIG_ENABLE_ATBM_DEVICE_INFO_PROVIDER

#if CONFIG_SEC_CERT_DAC_PROVIDER
	DeviceLayer::ESP32SecureCertDACProvider gSecureCertDACProvider;
#endif // CONFIG_SEC_CERT_DAC_PROVIDER

chip::Credentials::DeviceAttestationCredentialsProvider * get_dac_provider(void)
{
#if CONFIG_SEC_CERT_DAC_PROVIDER
    return &gSecureCertDACProvider;
#elif CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER
    return &sFactoryDataProvider;
#else // EXAMPLE_DAC_PROVIDER
    return chip::Credentials::Examples::GetExampleDACProvider();
#endif
}

} // namespace

static void InitServer(intptr_t context)
{
    DeviceCallbacksDelegate::Instance().SetAppDelegate(&sAppDeviceCallbacksDelegate);
    ATBMAppServer::Init();
}

extern "C" void app_matter_main()
{
    ChipLogProgress(DeviceLayer, "Enter app_matter_main...");
    //platform init
    EfErrCode err = easyflash_init();
    if (err != EF_NO_ERR)
    {
        ChipLogError(DeviceLayer, "easyflash_init fail...");
        return;
    }

#if CONFIG_ENABLE_CHIP_SHELL
    chip::LaunchShell();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
	if (Internal::ATBMUtils::InitWiFiStack() != CHIP_NO_ERROR)
	{
		ChipLogError(DeviceLayer, "Failed to initialize WiFi stack");
		return;
	}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

    DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();
    CHIP_ERROR error              = deviceMgr.Init(&EchoCallbacks);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "device.Init() failed: %s", ErrorStr(error));
        return;
    }

#if CONFIG_ENABLE_ATBM_FACTORY_DATA_PROVIDER
    SetCommissionableDataProvider(&sFactoryDataProvider);
#if CONFIG_ENABLE_ATBM_DEVICE_INSTANCE_INFO_PROVIDER
    SetDeviceInstanceInfoProvider(&sFactoryDataProvider);
#endif
#else
    error = chip::examples::InitCommissionableDataProvider(gExampleCommissionableDataProvider);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "InitCommissionableDataProvider failed");
	return;
    }
    SetCommissionableDataProvider(&gExampleCommissionableDataProvider);
#endif // CONFIG_ENABLE_ATBM_FACTORY_DATA_PROVIDER

    SetDeviceAttestationCredentialsProvider(get_dac_provider());

    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitServer, reinterpret_cast<intptr_t>(nullptr));

    error = GetAppTask().StartAppTask();
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "GetAppTask().StartAppTask() failed : %s", ErrorStr(error));
    }
    else
    {
        ChipLogProgress(DeviceLayer, "StartAppTask over.");
    }
}
