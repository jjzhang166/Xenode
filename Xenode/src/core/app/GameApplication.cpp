#include "pch"
#include "GameApplication.h"
#include "Log.h"

namespace Xen {

	void GameApplication::Run()
	{
		Log::Init();
		XEN_ENGINE_LOG_ERROR_SEVERE("Hello!!")
		XEN_ENGINE_LOG_ERROR("Hello!! {0}", 32)
		XEN_ENGINE_LOG_WARN("Hello!!")
		XEN_ENGINE_LOG_INFO("Hello!!")
		XEN_ENGINE_LOG_TRACE("Hello!!")
	}
}
