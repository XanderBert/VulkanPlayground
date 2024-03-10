#pragma once
#include "lua.hpp"
#include "sol/sol.hpp"
#include "../Patterns/Singleton.h"

class LuaScriptRunner final
{
public:
	LuaScriptRunner()
	{
		m_Lua.open_libraries(sol::lib::base, sol::lib::io);
	}

	sol::state& GetLuaRunner() { return m_Lua; }


private:
	//Lua
	sol::state m_Lua{};
};
