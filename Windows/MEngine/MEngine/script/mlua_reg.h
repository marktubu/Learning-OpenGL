#pragma once

#include <lua/lua.hpp>
#include <lua/lauxlib.h>

#include <string>
#include <unordered_map> 

#define metatable_name(n) "mlua."+n

template<typename T>
class mlua_metatable {
	static std::unordered_map<std::string, int> name_mt_idx;
};

template<typename T>
class metatable_reg_impl {
	static int index_function(lua_State* L) {
		lua_gettop(L);
	}
};

class mlua_reg {
public:
	mlua_reg(lua_State* l): ls(l) {

	}

	template<typename T>
	void get_metatable(std::string name) {
		std::string n = metatable_name(name);
		luaL_getmetatable(L, n);
	}

	template<typename T>
	void Reg(std::string name, T clz) {
		std::string n = metatable_name(name);
		int idx = luaL_newmetatable(ls, n);
		auto index_func = &metatable_reg_impl<T>::index_function;
		lua_pushstring(ls, "__index");
		lua_pushcclosure(ls, index_func);
		lua_settable(ls, -3);
	}

	void delete_func() {

	}


private:
	lua_State* ls;
};