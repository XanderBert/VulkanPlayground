#pragma once
#include "lua.hpp"
#include "sol/sol.hpp"
#include "../Patterns/Singleton.h"

class LuaScriptRunner final : public Singleton<LuaScriptRunner>
{
public:
	sol::state& GetLuaRunner() { return m_Lua; }

private:
	friend class Singleton<LuaScriptRunner>;


	//Setup the Lua Script Runner
	LuaScriptRunner() 
	{
		m_Lua.open_libraries(sol::lib::base, sol::lib::io);
	}


	//Lua
	sol::state m_Lua{};
};
