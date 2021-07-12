#pragma once
#include "Module.h"

namespace dyno
{
	class InputModule : public Module
	{
	public:
		InputModule();
		virtual ~InputModule();

		std::string getModuleType() final { return "OuputModule"; }
	protected:
		bool m_enabled;
	};
}