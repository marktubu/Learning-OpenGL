#pragma once

#include <string>

#include <lua/lua.hpp>
#include "mlua_op.h"
#include "mlua_reg.h"

//! 表示void类型，由于void类型不能return，用void_ignore_t适配
template<typename T>
struct void__t;

template<typename T>
struct void__t
{
	typedef T value_t;
};

template<>
struct void__t<void>
{
	typedef c_void_t value_t;
};

#define  RET_V typename void__t<RET>::value_t


class mlua {
public:
	mlua(bool b = false): enable_mod_func(b) {
		ls = luaL_newstate();
		luaL_openlibs(ls);
	}

	void setModFuncFlag(bool b) { enable_mod_func = b; }
	
	template<typename T>
	void reg(T a)
	{
		a(ls);
	}

	template<typename T>
	int get_global_var(const std::string& name, T& ret) {
		lua_getglobal(ls, name.c_str());
		int r = lua_op_stack<T>::to_value(ls, -1, ret);
		lua_pop(ls, 1);
		return r;
	}

	template<typename T>
	int set_global_var(const std::string& name, const T& val) {
		lua_op_stack<T>::push_stack(ls, val);
		lua_setglobal(ls, name.c_str());
		return 0;
	}

	void add_package_path(std::string path) {
		std::string new_path = "package.path = package.path .. \" ";
		if (path[0] != ';') {
			new_path += ";";
		}

		new_path += path;
		if (path[path.length() - 1] != '/') {
			new_path += "/";
		}

		new_path += "?.lua\" ";
		run_string(new_path);
	}

	void do_file(std::string file) {
		if (luaL_dofile(ls, file.c_str())) {
			mlua_tool::error("load file error " + file);
		}
	}

	void run_string(std::string str) {
		if (luaL_dostring(ls, str.c_str())) {
			mlua_tool::error("do string error " + str);
		}
	}

	// 返回值 当前已有的参数个数  :调用方式返回1，其他调用方式返回0
	int getFuncByName(std::string name) {
		if (false == enable_mod_func)
		{
			lua_getglobal(ls, name.c_str());
			return 0;
		}
		int pos = name.find(".");
		if (pos != std::string::npos) {
			auto t = name.substr(0, pos);
			lua_getglobal(ls, t.c_str());
			auto func = name.substr(pos + 1, std::string::npos);
			lua_getfield(ls, -1, func.c_str());
			lua_remove(ls, -2);
			return 0;
		}
		pos = name.find(":");
		if (pos != std::string::npos) {
			auto t = name.substr(0, pos);
			lua_getglobal(ls, t.c_str());
			auto func = name.substr(pos + 1, std::string::npos);
			lua_getfield(ls, -1, func.c_str());
			lua_pushvalue(ls, -2);
			lua_remove(ls, -3);
			return 1;
		}

		lua_getglobal(ls, name.c_str());
		return 0;
	}

	void call(std::string func_name) {
		lua_getglobal(ls, func_name.c_str());
		if (lua_pcall(ls, 0, 0, 0)) {
			mlua_tool::error("call function error " + func_name);
		}
	}

	template<typename RET>
	RET_V call(const char* func_name) {
		int nargs = getFuncByName(func_name);

		if (lua_pcall(ls, nargs + 0, 1, 0)) {
			mlua_tool::error("call function error " + std::string(func_name));
		}

		RET_V ret = init_value_t<RET_V>::value();
		if (lua_op_stack<RET_V>::to_value(ls, -1, ret)) {
			mlua_tool::error("get ret error");
		}

		lua_pop(ls, 1);

		return ret;
	}

	template<typename RET, typename ARG1>
	RET_V call(const char* func_name, ARG1 arg1) {
		int nargs = getFuncByName(func_name);
		
		lua_op_stack<ARG1>::push_stack(ls, arg1);
		if (lua_pcall(ls, nargs+1, 1, 0)) {
			mlua_tool::error("call function error " + std::string(func_name));
		}

		RET_V ret = init_value_t<RET_V>::value();
		if (lua_op_stack<RET_V>::to_value(ls, -1, ret)) {
			mlua_tool::error("get ret error");
		}

		lua_pop(ls, 1);

		return ret;
	}

	template<typename RET, typename ARG1, typename ARG2>
	RET_V call(const char* func_name, ARG1 arg1, ARG2 arg2) {
		int nargs = getFuncByName(func_name);

		lua_op_stack<ARG1>::push_stack(ls, arg1);
		lua_op_stack<ARG2>::push_stack(ls, arg2);
		
		if (lua_pcall(ls, nargs + 2, 1, 0)) {
			mlua_tool::error("call function error " + std::string(func_name));
		}

		RET_V ret = init_value_t<RET_V>::value();
		if (lua_op_stack<RET_V>::to_value(ls, -1, ret)) {
			mlua_tool::error("get ret error");
		}

		lua_pop(ls, 1);

		return ret;
	}

	template<typename RET, typename ARG1, typename ARG2, typename ARG3>
	RET_V call(const char* func_name, ARG1 arg1, ARG2 arg2, ARG3 arg3) {
		int nargs = getFuncByName(func_name);

		lua_op_stack<ARG1>::push_stack(ls, arg1);
		lua_op_stack<ARG2>::push_stack(ls, arg2);
		lua_op_stack<ARG3>::push_stack(ls, arg3);

		if (lua_pcall(ls, nargs + 3, 1, 0)) {
			mlua_tool::error("call function error " + std::string(func_name));
		}

		RET_V ret = init_value_t<RET_V>::value();
		if (lua_op_stack<RET_V>::to_value(ls, -1, ret)) {
			mlua_tool::error("get ret error");
		}

		lua_pop(ls, 1);

		return ret;
	}

	template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
	RET_V call(const char* func_name, ARG1 arg1, ARG2 arg2, ARG3 arg3, ARG4 arg4) {
		int nargs = getFuncByName(func_name);

		lua_op_stack<ARG1>::push_stack(ls, arg1);
		lua_op_stack<ARG2>::push_stack(ls, arg2);
		lua_op_stack<ARG3>::push_stack(ls, arg3);
		lua_op_stack<ARG4>::push_stack(ls, arg4);

		if (lua_pcall(ls, nargs + 4, 1, 0)) {
			mlua_tool::error("call function error " + std::string(func_name));
		}

		RET_V ret = init_value_t<RET_V>::value();
		if (lua_op_stack<RET_V>::to_value(ls, -1, ret)) {
			mlua_tool::error("get ret error");
		}

		lua_pop(ls, 1);

		return ret;
	}

	template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
	RET_V call(const char* func_name, ARG1 arg1, ARG2 arg2, ARG3 arg3, ARG4 arg4, ARG5 arg5) {
		int nargs = getFuncByName(func_name);

		lua_op_stack<ARG1>::push_stack(ls, arg1);
		lua_op_stack<ARG2>::push_stack(ls, arg2);
		lua_op_stack<ARG3>::push_stack(ls, arg3);
		lua_op_stack<ARG4>::push_stack(ls, arg4);
		lua_op_stack<ARG5>::push_stack(ls, arg5);

		if (lua_pcall(ls, nargs + 5, 1, 0)) {
			mlua_tool::error("call function error " + std::string(func_name));
		}

		RET_V ret = init_value_t<RET_V>::value();
		if (lua_op_stack<RET_V>::to_value(ls, -1, ret)) {
			mlua_tool::error("get ret error");
		}

		lua_pop(ls, 1);

		return ret;
	}

	template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
	RET_V call(const char* func_name, ARG1 arg1, ARG2 arg2, ARG3 arg3, ARG4 arg4, ARG5 arg5, ARG6 arg6) {
		int nargs = getFuncByName(func_name);

		lua_op_stack<ARG1>::push_stack(ls, arg1);
		lua_op_stack<ARG2>::push_stack(ls, arg2);
		lua_op_stack<ARG3>::push_stack(ls, arg3);
		lua_op_stack<ARG4>::push_stack(ls, arg4);
		lua_op_stack<ARG5>::push_stack(ls, arg5);
		lua_op_stack<ARG6>::push_stack(ls, arg6);

		if (lua_pcall(ls, nargs + 6, 1, 0)) {
			mlua_tool::error("call function error " + std::string(func_name));
		}

		RET_V ret = init_value_t<RET_V>::value();
		if (lua_op_stack<RET_V>::to_value(ls, -1, ret)) {
			mlua_tool::error("get ret error");
		}

		lua_pop(ls, 1);

		return ret;
	}

public:
	lua_State* ls;
	bool enable_mod_func;
};