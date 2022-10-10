#pragma once

#include <iostream>
#include <string>

#include <lua/lua.hpp>

struct c_void_t {};
class lua_nil_t {};

template <typename T>
struct init_value_t;

template <typename T>
struct init_value_t
{
	inline static T value() { return T(); }
};

template <typename T>
struct init_value_t<const T*>
{
	inline static T* value() { return NULL; }
};

template <typename T>
struct init_value_t<const T&>
{
	inline static T value() { return T(); }
};

template <>
struct init_value_t<string>
{
	inline static const char* value() { return ""; }
};

template <>
struct init_value_t<const string&>
{
	inline static const char* value() { return ""; }
};

class mlua_tool {
public:
	static void dump_stack(lua_State* L) {
		int top = lua_gettop(L);

		std::string str;
		for (int i = 1;i <= top;i++) {
			int type = lua_type(L, i);
			switch (type)
			{
			case LUA_TNIL:
				str.append("nil\n");
				break;
			case LUA_TBOOLEAN:
				str.append("boolean: ").append(lua_toboolean(L, i) ? "true" : "false").append("\n");
				break;
			case LUA_TLIGHTUSERDATA: {
				auto p = (int)lua_touserdata(L, i);
				str.append("lightuserdata: ").append(std::to_string(p)).append("\n");
				break;
			}
			case LUA_TNUMBER: {
				auto n = lua_tonumber(L, i);
				str.append("number: ").append(std::to_string(n)).append("\n");
				break;
			}
			case LUA_TSTRING:
				str.append("string: ").append(lua_tostring(L, i)).append("\n");
				break;
			case LUA_TTABLE:
				str.append("table start:\n");
				lua_pushnil(L);
				while (lua_next(L, i)) {
					str.append("key: ").append(lua_typename(L, lua_type(L, -2))).append("\n");
					str.append("value: ").append(lua_typename(L, lua_type(L, -1))).append("\n");
					lua_pop(L, 1); // pop value, use key as iterator
				}
				str.append("table end\n");
				break;
			case LUA_TFUNCTION:
				str.append("function: ").append(std::to_string((int)lua_tocfunction(L, i))).append("\n");
				break;
			case LUA_TUSERDATA: {
				auto p2 = (int)lua_touserdata(L, i);
				str.append("userdata: ").append(std::to_string(p2)).append("\n");
				break;
			}
			case LUA_TTHREAD:
				str.append("thread: ").append(std::to_string((int)lua_tothread(L, i))).append("\n");
				break;
			default:
				break;
			}
		}

		std::cout << str << std::endl;
	}

	static void error(std::string err) {
		std::cout << "error: " << err << std::endl;
	}
};

template<typename T>
class lua_op_stack {
	static void push_stack(lua_State* L, T arg) {}
};

template<>
class lua_op_stack<c_void_t> {
	static int to_value(lua_State* L) {
		return 0;
	}
};

template<>
class lua_op_stack<lua_nil_t> {
	static void push_stack(lua_State* L) {
		lua_pushnil(L);
	}
};

template<>
class lua_op_stack<std::string> {
	static void push_stack(lua_State* L, std::string s) {
		lua_pushstring(L, s.c_str());
	}

	static int to_value(lua_State* L, int idx, std::string& ret) {
		const char* s = luaL_checkstring(L, idx);
		ret = std::string(s);
		return 0;
	}
};

template<>
class lua_op_stack<char*> {
	static void push_stack(lua_State* L, char* s) {
		lua_pushstring(L, s);
	}

	static int to_value(lua_State* L, int idx, char*& ret) {
		const char* s = luaL_checkstring(L, idx);
		ret = (char*)s;
		return 0;
	}
};

template<>
class lua_op_stack<const char*> {
	static void push_stack(lua_State* L, const char* s) {
		lua_pushstring(L, s);
	}

	static int to_value(lua_State* L, int idx, char*& ret) {
		const char* s = luaL_checkstring(L, idx);
		ret = (char*)s;
		return 0;
	}
};

template<>
class lua_op_stack<int> {
public:
	static void push_stack(lua_State* L, int n) {
		lua_pushinteger(L, n);
	}

	static int to_value(lua_State* L, int idx, int& ret) {
		ret = luaL_checkinteger(L, idx);
		return 0;
	}
};

template<>
class lua_op_stack<float> {
	static void push_stack(lua_State* L, float n) {
		lua_pushnumber(L, (lua_Number)n);
	}

	static int to_value(lua_State* L, int idx, float& ret) {
		ret = (float)luaL_checknumber(L, idx);
		return 0;
	}
};

template<>
class lua_op_stack<double> {
	static void push_stack(lua_State* L, double n) {
		lua_pushnumber(L, (lua_Number)n);
	}

	static int to_value(lua_State* L, int idx, double& ret) {
		ret = (double)luaL_checknumber(L, idx);
		return 0;
	}
};

template<>
class lua_op_stack<bool> {
public:
	static void push_stack(lua_State* L, bool n) {
		lua_pushboolean(L, n);
	}

	static int to_value(lua_State* L, int idx, bool& ret) {
		luaL_checktype(L, idx, LUA_TBOOLEAN);
		ret = lua_toboolean(L, idx) != 0;
		return 0;
	}
};