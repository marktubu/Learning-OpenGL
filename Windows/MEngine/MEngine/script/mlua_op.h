#pragma once

#include <iostream>
#include <string>

#include <lua/lua.hpp>

#define INHERIT_TABLE "inherit_table"

struct c_void_t {};
class lua_nil_t {};

template <typename T>
class userdata_obj
{
public:
	userdata_obj(T* arg): ptr(arg) {}
	T* ptr;
};

// 用来表达类概念的 metatable名，及基类名
template <typename T>
class lua_typeinfo_t
{
public:
	static void set_name(std::string _name, std::string inherit) {
		name = _name;
		inherit_name = inherit;
	}

	inline static const char* get_name() {
		return name.c_str();
	}

	inline static const char* get_inherit_name() {
		return inherit_name.c_str();
	}

	inline static bool is_registed() {
		return !name.empty();
	}

	inline static bool is_inherit() {
		return !inherit_name.empty();
	}

	static std::string name;
	static std::string inherit_name;
};
template <typename T>
std::string lua_typeinfo_t<T>::name = std::string();
template <typename T>
std::string lua_typeinfo_t<T>::inherit_name = std::string();

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

template<typename T>
struct basetype_ptr_t;

template<>
struct basetype_ptr_t<const string&>
{
	typedef string arg_type;
};
template<>
struct basetype_ptr_t<string&>
{
	typedef string arg_type;
};
template<>
struct basetype_ptr_t<string>
{
	typedef string arg_type;
};
template<>
struct basetype_ptr_t<const char*>
{
	typedef char* arg_type;
};
template<>
struct basetype_ptr_t<char*>
{
	typedef char* arg_type;
};
template<>
struct basetype_ptr_t<char>
{
	typedef char arg_type;
};
template<>
struct basetype_ptr_t<unsigned char>
{
	typedef unsigned char arg_type;
};
template<>
struct basetype_ptr_t<short>
{
	typedef short arg_type;
};
template<>
struct basetype_ptr_t<int>
{
	typedef int arg_type;
};

template<>
struct basetype_ptr_t<long>
{
	typedef long arg_type;
};
template<>
struct basetype_ptr_t<long long>
{
	typedef long long arg_type;
};
template<>
struct basetype_ptr_t<unsigned long>
{
	typedef unsigned long arg_type;
};
template<>
struct basetype_ptr_t<unsigned long long>
{
	typedef unsigned long long arg_type;
};
template<>
struct basetype_ptr_t<bool>
{
	typedef bool arg_type;
};
template<>
struct basetype_ptr_t<float>
{
	typedef float arg_type;
};
template<>
struct basetype_ptr_t<double>
{
	typedef double arg_type;
};
template<typename T>
struct basetype_ptr_t<T*>
{
	typedef T* arg_type;
};
template<typename T>
struct basetype_ptr_t<const T*>
{
	typedef T* arg_type;
};
template<typename T>
struct basetype_ptr_t<T&>
{
	typedef T* arg_type;
};
template<typename T>
struct basetype_ptr_t<const T&>
{
	typedef T* arg_type;
};

template<typename ARG_TYPE>
struct p_t;

template<typename ARG_TYPE>
struct p_t
{
	static ARG_TYPE r(ARG_TYPE a) { return a; }
	static ARG_TYPE& r(ARG_TYPE* a) { return *a; }
};
template<typename ARG_TYPE>
struct p_t<ARG_TYPE&>
{
	static ARG_TYPE& r(ARG_TYPE& a) { return a; }
	static ARG_TYPE& r(ARG_TYPE* a) { return *a; }
};


template<typename T>
struct lua_op_stack {
	static void push_stack(lua_State* L, const char* arg) { 
		lua_pushstring(ls_, arg_); 
	}
};

template<>
struct lua_op_stack<c_void_t> {
	static int to_value(lua_State* L, int idx, c_void_t& ret) {
		return 0;
	}
};

template<>
struct lua_op_stack<lua_nil_t> {
	static void push_stack(lua_State* L) {
		lua_pushnil(L);
	}
};

template<>
struct lua_op_stack<char> {
	static void push_stack(lua_State* L, char arg) {
		lua_pushnumber(L, (lua_Number)arg);
	}

	static int to_value(lua_State* L, int idx, char& ret) {
		ret = (char)luaL_checknumber(L, idx);
		return 0;
	}
};

template<>
struct lua_op_stack<char*> {
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
struct lua_op_stack<const char*> {
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
struct lua_op_stack<std::string> {
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
struct lua_op_stack<const std::string&> {
	static void push_stack(lua_State* L, const std::string& s) {
		lua_pushstring(L, s.c_str());
	}

	static int to_value(lua_State* L, int idx, std::string& ret) {
		const char* s = luaL_checkstring(L, idx);
		ret = std::string(s);
		return 0;
	}
};

template<>
struct lua_op_stack<int> {
public:
	static void push_stack(lua_State* L, int n) {
		lua_pushinteger(L, n);
	}

	static int to_value(lua_State* L, int idx, int& ret) {
		ret = (int)luaL_checkinteger(L, idx);
		return 0;
	}
};

template<>
struct lua_op_stack<int64_t> {
public:
	static void push_stack(lua_State* L, int64_t n) {
		lua_pushinteger(L, n);
	}

	static int to_value(lua_State* L, int idx, int64_t& ret) {
		ret = luaL_checkinteger(L, idx);
		return 0;
	}
};

template<>
struct lua_op_stack<float> {
	static void push_stack(lua_State* L, float n) {
		lua_pushnumber(L, (lua_Number)n);
	}

	static int to_value(lua_State* L, int idx, float& ret) {
		ret = (float)luaL_checknumber(L, idx);
		return 0;
	}
};

template<>
struct lua_op_stack<double> {
	static void push_stack(lua_State* L, double n) {
		lua_pushnumber(L, (lua_Number)n);
	}

	static int to_value(lua_State* L, int idx, double& ret) {
		ret = (double)luaL_checknumber(L, idx);
		return 0;
	}
};

template<>
struct lua_op_stack<bool> {
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

template<>
struct lua_op_stack<void*>
{
	static void push_stack(lua_State* L, void* arg) {
		lua_pushlightuserdata(L, arg);
	}

	static int to_value(lua_State* L, int idx, void*& ret) {
		if (!lua_isuserdata(L, idx)) {
			luaL_argerror(L, 1, "userdata param expected");
			return -1;
		}
		ret = lua_touserdata(L, idx);
		return 0;
	}
};

template<typename T>
struct lua_op_stack<T*> {
public:
	static void push_stack(lua_State* L, T& arg) {
		void* ptr = lua_newuserdata(L, sizeof(userdata_obj<T>));
		new (ptr) userdata_obj<T>(&arg);

		luaL_getmetatable(L, lua_typeinfo_t<T>::get_name());
		lua_setmetatable(L, -2);
	}

	static void push_stack(lua_State* L, const T& arg) {
		void* ptr = lua_newuserdata(L, sizeof(userdata_obj<const T>));
		new (ptr) userdata_obj<const T>(&arg);

		luaL_getmetatable(L, lua_typeinfo_t<T>::get_name());
		lua_setmetatable(L, -2);
	}

	static void push_stack(lua_State* L, T* arg) {
		void* ptr = lua_newuserdata(L, sizeof(userdata_obj<T>));
		new (ptr) userdata_obj<T>(arg);

		luaL_getmetatable(L, lua_typeinfo_t<T>::get_name());
		lua_setmetatable(L, -2);
	}

	static int to_value(lua_State* L, int idx, T*& ret) {
		if (lua_typeinfo_t<T>::is_registed() == false) {
			luaL_argerror(L, idx, "type not supported");
		}
		void* arg = lua_touserdata(L, idx);
		if (arg == NULL) {
			mlua_tool::error("expect " + lua_typeinfo_t<T>::name + " but " + lua_typename(L, lua_type(L, idx)));
			return -1;
		}
		if (lua_getmetatable(L, idx) == 0) {
			return -1;
		}

		// T可能是 class 也可能是 base_class
		luaL_getmetatable(L, lua_typeinfo_t<T>::get_name());
		if (lua_rawequal(L, -1, -2) == 0) {
			// 如果两个metatable不相等，尝试获取 metatable(-2) 的 inherit_table(也即 base_class 的 metatable)
			// 然后再进行比较
			lua_getfield(L, -2, INHERIT_TABLE);
			if (lua_rawequal(L, -1, -2) == 0) {
				mlua_tool::error("expect " + lua_typeinfo_t<T>::name + " but " + lua_typename(L, lua_type(L, idx)));
				lua_pop(L, 3);
				return -1;
			}
			lua_pop(L, 3);
		}
		else {
			lua_pop(L, 2);
		}
		T* ret_ptr = ((userdata_obj<T>*)arg)->ptr;
		if (ret_ptr == NULL) {
			return -1;
		}
		ret = ret_ptr;
		return 0;
	}
};

template <typename T>
struct lua_op_stack<const T*>
{
	static void push_stack(lua_State* L, const T* arg) {
		lua_op_stack<T*>::push_stack(L, (T*)arg);
	}

	static int to_value(lua_State* L, int idx, T*& ret) {
		return lua_op_stack<T*>::to_value(L, idx, ret);
	}
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
