#pragma once

#include <lua/lua.hpp>
#include <lua/lauxlib.h>

#include <string>
#include <unordered_map> 

#include "mlua_op.h"

#define metatable_name(n) std::string("mlua."+n).c_str()

#define virtual_ctor int
#define ctor void
#define LUA_ARG_POS(x) (x)

typedef int (*mt_index_func_t)(lua_State*, void*, const char*);
typedef int (*mt_newindex_func_t)(lua_State*, void*, const char*, int);

struct op_tool_t
{
    
};

template<typename T>
struct class_property_info_t
{
	class_property_info_t() :property_pos(NULL) {}
	T property_pos;
};

//! 记录类中字段的指针
struct real_class_property_processor_t
{
	real_class_property_processor_t() :
		index_impl_func(NULL),
		newindex_impl_func(NULL),
		property_pos(NULL)
	{}
	mt_index_func_t    index_impl_func;
	mt_newindex_func_t newindex_impl_func;

	void* property_pos;
};

template<typename FUNC_TYPE>
struct userdata_for_function_t
{
	userdata_for_function_t(FUNC_TYPE func_) : real_func(func_) {}
	FUNC_TYPE   real_func;
};
template<typename PROPERTY_TYPE>
struct userdata_for_class_property_t : public real_class_property_processor_t
{
	typedef class_property_info_t<PROPERTY_TYPE> real_class_property_info_t;
	real_class_property_info_t                   property_info;
};

template<typename CLASS_TYPE>
class metatable_register_impl_t {
public:
	/*
	* p2 key
	* p1 userdata
	* index_function
	*/
	static int index_function(lua_State* L) {
		const char* key = luaL_checkstring(L, 2);
		luaL_getmetatable(L, lua_typeinfo_t<CLASS_TYPE>::get_name());
		int mtable_idx = lua_gettop(L);
		lua_getfield(L, -1, key);
		lua_remove(L, mtable_idx);

		if (lua_isnil(L, -1) && lua_typeinfo_t<CLASS_TYPE>::is_inherit()) {
			lua_pop(L, 1);
			luaL_getmetatable(L, lua_typeinfo_t<CLASS_TYPE>::get_inherit_name());
			mtable_idx = lua_gettop(L);
			lua_getfield(L, -1, key);
			lua_remove(L, mtable_idx);
		}

		if (lua_isuserdata(L, -1)) {
			auto p = (real_class_property_processor_t*)lua_touserdata(L, -1);
			lua_pop(L, 1);
			return (*(p->index_impl_func))(L, p->property_pos, key);
		}
		return 1;
	}

	static int newindex_function(lua_State * L) {
		const char* key = luaL_checkstring(L, 2);
		luaL_getmetatable(L, lua_typeinfo_t<CLASS_TYPE>::get_name());
		int mtable_idx = lua_gettop(L);
		lua_getfield(L, -1, key);
		lua_remove(L, mtable_idx);

		if (lua_isnil(L, -1) && lua_typeinfo_t<CLASS_TYPE>::is_inherit()) {
			lua_pop(L, 1);
			luaL_getmetatable(L, lua_typeinfo_t<CLASS_TYPE>::get_inherit_name());
			mtable_idx = lua_gettop(L);
			lua_getfield(L, -1, key);
			lua_remove(L, mtable_idx);
		}

		if (lua_isuserdata(L, -1)) {
			auto p = (real_class_property_processor_t*)lua_touserdata(L, -1);
			lua_pop(L, 1);
			return (*(p->newindex_impl_func))(L, p->property_pos, key, 3);
		}
		return 1;
	}

	static CLASS_TYPE** userdata_to_object_ptr_address(lua_State* L) {
		if (lua_typeinfo_t<CLASS_TYPE>::is_registed() == false) {
			luaL_argerror(L, 1, "can not convert to class*, not registed");
		}

		void* arg = luaL_checkudata(L, 1, lua_typeinfo_t<CLASS_TYPE>::get_name());
		if (arg == NULL) {
			luaL_argerror(L, 1, "expect arg 1, but arg == null");
		}
		CLASS_TYPE** ret_ptr = &(((userdata_obj<CLASS_TYPE>*)arg)->ptr);
		return ret_ptr;
	}

	static CLASS_TYPE* userdata_to_object(lua_State* L) {
		if (lua_typeinfo_t<CLASS_TYPE>::is_registed() == false) {
			luaL_argerror(L, 1, "can not convert to class*, not registed");
		}
		void* arg = lua_touserdata(L, 1);
		if (arg == NULL) {
			luaL_argerror(L, 1, "expect arg 1, but arg == null");
		}
		if (lua_getmetatable(L, 1) == 0) {
			luaL_argerror(L, 1, "arg 1 has no metatable, its not cpp type");
		}
		luaL_getmetatable(L, lua_typeinfo_t<CLASS_TYPE>::get_name());
		if (lua_rawequal(L, -1, -2) == 0) {
            // 如果不相等，可能时子类调用了基类的函数或者使用了基类的字段，
            // 检查子类的基类metatable是否与当前lua_typeinfo_t<CLASS_TYPE>::get_name()获得的metatable一致
			lua_getfield(L, -2, INHERIT_TABLE);
			if (lua_rawequal(L, -1, -2) == 0) {
				lua_pop(L, 3);
				luaL_argerror(L, 1, "type convert failed");
			}
			lua_pop(L, 3);
		}
		else {
			lua_pop(L, 2);
		}
		CLASS_TYPE* ret_ptr = ((userdata_obj<CLASS_TYPE>*)arg)->ptr;
		if (ret_ptr == NULL) {
			luaL_argerror(L, 1, "object ptr can not be null");
		}
		return ret_ptr;
	}

	static int get_pointer(lua_State* ls_)
	{
		CLASS_TYPE** obj_ptr = userdata_to_object_ptr_address(ls_);
		int64_t  addr = int64_t(*obj_ptr);
		lua_op_stack<int64_t>::push_stack(ls_, addr);
		return 1;
	}
};

template <typename CLASS_TYPE, typename FUNC_TYPE>
struct new_traits_t;

template <typename CLASS_TYPE>
struct delete_traits_t
{
	static  int lua_function(lua_State* ls_)
	{
		CLASS_TYPE** obj_ptr = metatable_register_impl_t<CLASS_TYPE>::userdata_to_object_ptr_address(ls_);

		delete* obj_ptr;
		*obj_ptr = NULL;
		return 0;
	}
};

template <typename FUNC_TYPE>
struct class_function_traits_t;
template <typename PROPERTY_TYPE, typename RET>
struct class_property_traits_t;
template <typename FUNC_TYPE>
struct function_traits_t;

//! CLASS_TYPE 为要注册的类型, CTOR_TYPE为构造函数类型
template<typename T>
struct mlua_register_router_t;
template<typename T>
struct mlua_register_router_t
{
	template<typename REG_TYPE>
	static void call(REG_TYPE* reg_, T arg_, const string& s_)
	{
		reg_->def_class_property(arg_, s_);
	}
};

template<typename CLASS_TYPE = op_tool_t, typename CTOR_TYPE = void()>
class mlua_reg {
public:
	mlua_reg(lua_State* l): ls(l) {}
    mlua_reg(lua_State* ls_, const std::string& class_name_, std::string inherit_name_ = "")
        : ls(ls_),
        m_class_name(class_name_)
    {
        // metatable_name:fflua.+class_name_
        // static lua_type_info_t<T>::name = metatable_name
        lua_typeinfo_t<CLASS_TYPE>::set_name(metatable_name(class_name_), metatable_name(inherit_name_));

        // metatable
        luaL_newmetatable(ls_, metatable_name(class_name_)); // registry.metatable_name = metatable
        int  metatable_index = lua_gettop(ls_);
        if (false == inherit_name_.empty())//! 设置基类
        {
            luaL_getmetatable(ls_, metatable_name(inherit_name_));
            if (lua_istable(ls_, -1))
            {
                lua_setfield(ls_, metatable_index, INHERIT_TABLE);
            }
            else
            {
                lua_pop(ls_, 1);
            }
        }

        // index_function
        // "__index"
        // fflua.class_name_(metatable)
        lua_pushstring(ls_, "__index");
        lua_CFunction index_function = &metatable_register_impl_t<CLASS_TYPE>::index_function;
        lua_pushcclosure(ls_, index_function, 0);
        lua_settable(ls_, -3); // metatable.__index = index_function

        lua_pushstring(ls_, "get_pointer");
        lua_CFunction pointer_function = &metatable_register_impl_t<CLASS_TYPE>::get_pointer;
        lua_pushcclosure(ls_, pointer_function, 0);
        lua_settable(ls_, -3); // metatable.get_pointer = pointer_function

        lua_pushstring(ls_, "__newindex");
        lua_CFunction newindex_function = &metatable_register_impl_t<CLASS_TYPE>::newindex_function;
        lua_pushcclosure(ls_, newindex_function, 0);
        lua_settable(ls_, -3); // metatable.__newindex = newindex_function

        lua_CFunction function_for_new = &new_traits_t<CLASS_TYPE, CTOR_TYPE>::lua_function;
        lua_pushcclosure(ls_, function_for_new, 0);

        lua_newtable(ls_);

        // "new"
        // table
        // function_for_new
        lua_pushstring(ls_, "new");

        lua_pushvalue(ls_, -3);

        // function_for_new
        // "new"
        // table
        // function_for_new
        // metatable
        lua_settable(ls_, -3); // table.new = function_for_new

        // table
        // function_for_new
        // metatable
        lua_setglobal(ls_, class_name_.c_str()); // _G[class_name_] = table

        // function_for_new
        // metatable
        lua_pop(ls_, 1);

        lua_CFunction function_for_delete = &delete_traits_t<CLASS_TYPE>::lua_function;
        // function_for_delete
        // metatable
        lua_pushcclosure(ls_, function_for_delete, 0);

        // "delete
        // function_for_delete
        // metatable
        lua_pushstring(ls_, "delete");
        // function_for_delete
        // "delete
        // function_for_delete
        // metatable
        lua_pushvalue(ls_, -2);

        lua_settable(ls_, metatable_index);
        // function_for_delete
        // metatable


        // function_for_delete
        // metatable
        lua_pop(ls_, 2);
        // empty stack

        /*
        registry.metatable_name = metatable
        _G[class_name_] = table
        table = {
            new = function_for_new
        }
        metatable = {
            "INHERIT_TABLE" = inherit_metatable
            __index = index_function
            get_pointer = pointer_function
            __newindex = newindex_function
            delete = function_for_delete
        }
        */
    }

    template<typename FUNC_TYPE>
    mlua_reg& def(FUNC_TYPE func, const std::string& s_)
    {
        //func type为class:func时，调用def_class_func
        //func type为静态函数时，调用def_func
        //func type为class:property时，调用def_class_property
        //func type为静态字段，调用什么？？？？？
        mlua_register_router_t<FUNC_TYPE>::call(this, func, s_);
        return *this;
    }

    template<typename FUNC_TYPE>
    mlua_reg& def_class_func(FUNC_TYPE func_, const std::string& func_name_)
    {
        lua_CFunction class_function = &class_function_traits_t<FUNC_TYPE>::lua_function;
        typedef typename class_function_traits_t<FUNC_TYPE>::userdata_for_function_info userdata_for_function_t;
        void* user_data_ptr = lua_newuserdata(ls, sizeof(userdata_for_function_t));
        // stack:
        // userdata userdata作用：保存real_func_type, associate values with a C function, create a c closure,类似up value
        // class_function 不是实际的类函数，而是调用类函数的工具函数，userdata存储的才是真正的类函数，将userdata作为
        // 调用 class_function 时的 upvalue，然后将 class_function 作为 cclosure 设置给 metatable
        new(user_data_ptr) userdata_for_function_t(func_);
        lua_pushcclosure(ls, class_function, 1);
        // class_function

        luaL_getmetatable(ls, metatable_name(m_class_name));
        // metatable
        // class_function

        lua_pushstring(ls, func_name_.c_str());
        // func_name
        // metatable
        // class_function

        lua_pushvalue(ls, -3);
        // class_function
        // func_name
        // metatable
        // class_function

        lua_settable(ls, -3); // metatable.func_name = class_function
        // metatable
        // class_function

        lua_pop(ls, 2);
        return *this;
    }

    // example: .def(&foo_t::a, "a") p_ = &foo_t::a
    template<typename RET>
    mlua_reg& def_class_property(RET CLASS_TYPE::* p_, const std::string& property_name_)
    {
        typedef typename class_property_traits_t<CLASS_TYPE, RET>::process_index_func_t process_index_func_t;
        typedef typename class_property_traits_t<CLASS_TYPE, RET>::process_newindex_func_t process_newindex_func_t;
        process_index_func_t process_index = &class_property_traits_t<CLASS_TYPE, RET>::process_index;
        process_newindex_func_t process_newindex = &class_property_traits_t<CLASS_TYPE, RET>::process_newindex;

        typedef userdata_for_class_property_t<RET CLASS_TYPE::*> udata_t;
        /**
         * mt_index_func_t    index_impl_func;
         * mt_newindex_func_t newindex_impl_func;
         * void* property_pos;
         * real_class_property_info_t property_info;
         *     T（指针类型） property_pos
         */

        udata_t* pu = (udata_t*)lua_newuserdata(ls, sizeof(udata_t));
        pu->property_info.property_pos = p_;
        int udata_index = lua_gettop(ls);
        pu->index_impl_func = process_index;
        pu->newindex_impl_func = process_newindex;
        pu->property_pos = (void*)(&(pu->property_info));

        luaL_getmetatable(ls, lua_typeinfo_t<CLASS_TYPE>::get_name());
        lua_pushstring(ls, property_name_.c_str());
        lua_pushvalue(ls, udata_index);
        lua_settable(ls, -3);
        // metatable.a = pu

        lua_pop(ls, 1);
        lua_remove(ls, udata_index);
        return *this;
    }
    template<typename FUNC>
    mlua_reg& def_func(FUNC func_, const std::string& func_name_)
    {
        if (m_class_name.empty())
        {
            lua_CFunction lua_func = function_traits_t<FUNC>::lua_function;

            void* user_data_ptr = lua_newuserdata(ls, sizeof(func_));
            new(user_data_ptr) FUNC(func_);

            lua_pushcclosure(ls, lua_func, 1);
            lua_setglobal(ls, func_name_.c_str());
        }
        else
        {
            lua_CFunction lua_func = function_traits_t<FUNC>::lua_function;

            void* user_data_ptr = lua_newuserdata(ls, sizeof(func_));
            new(user_data_ptr) FUNC(func_);

            lua_pushcclosure(ls, lua_func, 1);
            //lua_setglobal(ls, func_name_.c_str());

            lua_getglobal(ls, (m_class_name).c_str());
            lua_pushstring(ls, func_name_.c_str());
            lua_pushvalue(ls, -3);
            lua_settable(ls, -3);

            lua_pop(ls, 2);
        }
        return *this;
    }


private:
	lua_State* ls;
    std::string m_class_name;
};




template <typename CLASS_TYPE>
struct new_traits_t<CLASS_TYPE, int()>
{
    static  int lua_function(lua_State* ls_)
    {
        return 0;
    }
};

template <typename CLASS_TYPE>
struct new_traits_t<CLASS_TYPE, void()>
{
    static  int lua_function(lua_State* ls_)
    {
        void* user_data_ptr = lua_newuserdata(ls_, sizeof(userdata_obj<CLASS_TYPE>));
        luaL_getmetatable(ls_, lua_typeinfo_t<CLASS_TYPE>::get_name());
        lua_setmetatable(ls_, -2);

        new(user_data_ptr) userdata_obj<CLASS_TYPE>(new CLASS_TYPE());
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1>
struct new_traits_t<CLASS_TYPE, void(ARG1)>
{
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);

        void* user_data_ptr = lua_newuserdata(ls_, sizeof(userdata_obj<CLASS_TYPE>));
        luaL_getmetatable(ls_, lua_typeinfo_t<CLASS_TYPE>::get_name());
        lua_setmetatable(ls_, -2);

        new(user_data_ptr) userdata_obj<CLASS_TYPE>(new CLASS_TYPE(arg1));
        return 1;
    }
};


template <typename CLASS_TYPE, typename ARG1, typename ARG2>
struct new_traits_t<CLASS_TYPE, void(ARG1, ARG2)>
{
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);

        void* user_data_ptr = lua_newuserdata(ls_, sizeof(userdata_obj<CLASS_TYPE>));
        luaL_getmetatable(ls_, lua_typeinfo_t<CLASS_TYPE>::get_name());
        lua_setmetatable(ls_, -2);

        new(user_data_ptr) userdata_obj<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3>
struct new_traits_t<CLASS_TYPE, void(ARG1, ARG2, ARG3)>
{
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);

        void* user_data_ptr = lua_newuserdata(ls_, sizeof(userdata_obj<CLASS_TYPE>));

        luaL_getmetatable(ls_, lua_typeinfo_t<CLASS_TYPE>::get_name());
        lua_setmetatable(ls_, -2);

        new(user_data_ptr) userdata_obj<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2, arg3));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3,
    typename ARG4>
struct new_traits_t<CLASS_TYPE, void(ARG1, ARG2, ARG3, ARG4)>
{
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);

        void* user_data_ptr = lua_newuserdata(ls_, sizeof(userdata_obj<CLASS_TYPE>));
        luaL_getmetatable(ls_, lua_typeinfo_t<CLASS_TYPE>::get_name());
        lua_setmetatable(ls_, -2);

        new(user_data_ptr) userdata_obj<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2, arg3, arg4));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3,
    typename ARG4, typename ARG5>
struct new_traits_t<CLASS_TYPE, void(ARG1, ARG2, ARG3, ARG4,
    ARG5)>
{
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);

        void* user_data_ptr = lua_newuserdata(ls_, sizeof(userdata_obj<CLASS_TYPE>));
        luaL_getmetatable(ls_, lua_typeinfo_t<CLASS_TYPE>::get_name());
        lua_setmetatable(ls_, -2);

        new(user_data_ptr) userdata_obj<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2, arg3, arg4, arg5));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3,
    typename ARG4, typename ARG5, typename ARG6>
struct new_traits_t<CLASS_TYPE, void(ARG1, ARG2, ARG3,
    ARG4, ARG5, ARG6)>
{
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg6);

        void* user_data_ptr = lua_newuserdata(ls_, sizeof(userdata_obj<CLASS_TYPE>));
        luaL_getmetatable(ls_, lua_typeinfo_t<CLASS_TYPE>::get_name());
        lua_setmetatable(ls_, -2);

        new(user_data_ptr) userdata_obj<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2, arg3, arg4, arg5, arg6));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3,
    typename ARG4, typename ARG5, typename ARG6, typename ARG7>
struct new_traits_t<CLASS_TYPE, void(ARG1, ARG2, ARG3,
    ARG4, ARG5, ARG6, ARG7)>
{
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        typename basetype_ptr_t<ARG7>::arg_type arg7 = init_value_t<typename basetype_ptr_t<ARG7>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_stack<typename basetype_ptr_t<ARG7>::arg_type>::to_value(ls_, LUA_ARG_POS(8), arg7);

        void* user_data_ptr = lua_newuserdata(ls_, sizeof(userdata_obj<CLASS_TYPE>));

        luaL_getmetatable(ls_, lua_typeinfo_t<CLASS_TYPE>::get_name());
        lua_setmetatable(ls_, -2);

        new(user_data_ptr) userdata_obj<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2, arg3, arg4, arg5,
            arg6, arg7));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3,
    typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct new_traits_t<CLASS_TYPE, void(ARG1, ARG2, ARG3,
    ARG4, ARG5, ARG6, ARG7, ARG8)>
{
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        typename basetype_ptr_t<ARG7>::arg_type arg7 = init_value_t<typename basetype_ptr_t<ARG7>::arg_type>::value();
        typename basetype_ptr_t<ARG8>::arg_type arg8 = init_value_t<typename basetype_ptr_t<ARG8>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_stack<typename basetype_ptr_t<ARG7>::arg_type>::to_value(ls_, LUA_ARG_POS(8), arg7);
        lua_op_stack<typename basetype_ptr_t<ARG8>::arg_type>::to_value(ls_, LUA_ARG_POS(9), arg8);

        void* user_data_ptr = lua_newuserdata(ls_, sizeof(userdata_obj<CLASS_TYPE>));
        luaL_getmetatable(ls_, lua_typeinfo_t<CLASS_TYPE>::get_name());
        lua_setmetatable(ls_, -2);

        new(user_data_ptr) userdata_obj<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2, arg3, arg4, arg5, arg6,
            arg7, arg8));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3,
    typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8,
    typename ARG9>
struct new_traits_t<CLASS_TYPE, void(ARG1, ARG2, ARG3,
    ARG4, ARG5, ARG6, ARG7, ARG8, ARG9)>
{
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        typename basetype_ptr_t<ARG7>::arg_type arg7 = init_value_t<typename basetype_ptr_t<ARG7>::arg_type>::value();
        typename basetype_ptr_t<ARG8>::arg_type arg8 = init_value_t<typename basetype_ptr_t<ARG8>::arg_type>::value();
        typename basetype_ptr_t<ARG9>::arg_type arg9 = init_value_t<typename basetype_ptr_t<ARG9>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_stack<typename basetype_ptr_t<ARG7>::arg_type>::to_value(ls_, LUA_ARG_POS(8), arg7);
        lua_op_stack<typename basetype_ptr_t<ARG8>::arg_type>::to_value(ls_, LUA_ARG_POS(9), arg8);
        lua_op_stack<typename basetype_ptr_t<ARG9>::arg_type>::to_value(ls_, LUA_ARG_POS(10), arg9);

        void* user_data_ptr = lua_newuserdata(ls_, sizeof(userdata_obj<CLASS_TYPE>));
        luaL_getmetatable(ls_, lua_typeinfo_t<CLASS_TYPE>::get_name());
        lua_setmetatable(ls_, -2);

        new(user_data_ptr) userdata_obj<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2, arg3, arg4, arg5, arg6,
            arg7, arg8, arg9));

        return 1;
    }
};

template <typename FUNC_CLASS_TYPE>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)()>
{
    typedef void (FUNC_CLASS_TYPE::* dest_func_t)();
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);
        (obj_ptr->*(registed_data.real_func))();
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1)>
{
    typedef void (FUNC_CLASS_TYPE::* dest_func_t)(ARG1);
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1));
        return 0;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2)>
{
    typedef void (FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2);
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3)>
{
    typedef void (FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3);
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4)>
{
    typedef void (FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4);
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5)>
{
    typedef void (FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5);
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5));
        return 0;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
    ARG6)>
{
    typedef void (FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg6);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6, typename ARG7>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
    ARG6, ARG7)>
{
    typedef void (FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        typename basetype_ptr_t<ARG7>::arg_type arg7 = init_value_t<typename basetype_ptr_t<ARG7>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_stack<typename basetype_ptr_t<ARG7>::arg_type>::to_value(ls_, LUA_ARG_POS(8), arg7);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
    ARG6, ARG7, ARG8)>
{
    typedef void (FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        typename basetype_ptr_t<ARG7>::arg_type arg7 = init_value_t<typename basetype_ptr_t<ARG7>::arg_type>::value();
        typename basetype_ptr_t<ARG8>::arg_type arg8 = init_value_t<typename basetype_ptr_t<ARG8>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_stack<typename basetype_ptr_t<ARG7>::arg_type>::to_value(ls_, LUA_ARG_POS(8), arg7);
        lua_op_stack<typename basetype_ptr_t<ARG8>::arg_type>::to_value(ls_, LUA_ARG_POS(9), arg8);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7), p_t<ARG8>::r(arg8));
        return 0;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
    ARG6, ARG7, ARG8, ARG9)>
{
    typedef void (FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        typename basetype_ptr_t<ARG7>::arg_type arg7 = init_value_t<typename basetype_ptr_t<ARG7>::arg_type>::value();
        typename basetype_ptr_t<ARG8>::arg_type arg8 = init_value_t<typename basetype_ptr_t<ARG8>::arg_type>::value();
        typename basetype_ptr_t<ARG9>::arg_type arg9 = init_value_t<typename basetype_ptr_t<ARG9>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_stack<typename basetype_ptr_t<ARG7>::arg_type>::to_value(ls_, LUA_ARG_POS(8), arg7);
        lua_op_stack<typename basetype_ptr_t<ARG8>::arg_type>::to_value(ls_, LUA_ARG_POS(9), arg8);
        lua_op_stack<typename basetype_ptr_t<ARG9>::arg_type>::to_value(ls_, LUA_ARG_POS(10), arg9);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7), p_t<ARG8>::r(arg8), p_t<ARG9>::r(arg9));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)() const>
{
    typedef void (FUNC_CLASS_TYPE::* dest_func_t)() const;
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);
        (obj_ptr->*(registed_data.real_func))();
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1) const>
{
    typedef void (FUNC_CLASS_TYPE::* dest_func_t)(ARG1) const;
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1));
        return 0;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2) const>
{
    typedef void (FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2) const;
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3) const>
{
    typedef void (FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3) const;
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4) const>
{
    typedef void (FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4) const;
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5) const>
{
    typedef void (FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5) const;
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5));
        return 0;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
    ARG6) const>
{
    typedef void (FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) const;
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg6);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6, typename ARG7>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
    ARG6, ARG7) const>
{
    typedef void (FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) const;
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        typename basetype_ptr_t<ARG7>::arg_type arg7 = init_value_t<typename basetype_ptr_t<ARG7>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_stack<typename basetype_ptr_t<ARG7>::arg_type>::to_value(ls_, LUA_ARG_POS(8), arg7);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
    ARG6, ARG7, ARG8) const>
{
    typedef void (FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8) const;
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        typename basetype_ptr_t<ARG7>::arg_type arg7 = init_value_t<typename basetype_ptr_t<ARG7>::arg_type>::value();
        typename basetype_ptr_t<ARG8>::arg_type arg8 = init_value_t<typename basetype_ptr_t<ARG8>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_stack<typename basetype_ptr_t<ARG7>::arg_type>::to_value(ls_, LUA_ARG_POS(8), arg7);
        lua_op_stack<typename basetype_ptr_t<ARG8>::arg_type>::to_value(ls_, LUA_ARG_POS(9), arg8);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7), p_t<ARG8>::r(arg8));
        return 0;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
    ARG6, ARG7, ARG8, ARG9) const>
{
    typedef void (FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9) const;
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        typename basetype_ptr_t<ARG7>::arg_type arg7 = init_value_t<typename basetype_ptr_t<ARG7>::arg_type>::value();
        typename basetype_ptr_t<ARG8>::arg_type arg8 = init_value_t<typename basetype_ptr_t<ARG8>::arg_type>::value();
        typename basetype_ptr_t<ARG9>::arg_type arg9 = init_value_t<typename basetype_ptr_t<ARG9>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_stack<typename basetype_ptr_t<ARG7>::arg_type>::to_value(ls_, LUA_ARG_POS(8), arg7);
        lua_op_stack<typename basetype_ptr_t<ARG8>::arg_type>::to_value(ls_, LUA_ARG_POS(9), arg8);
        lua_op_stack<typename basetype_ptr_t<ARG9>::arg_type>::to_value(ls_, LUA_ARG_POS(10), arg9);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7), p_t<ARG8>::r(arg8), p_t<ARG9>::r(arg9));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename RET>
struct class_function_traits_t<RET(FUNC_CLASS_TYPE::*)()>
{
    typedef RET(FUNC_CLASS_TYPE::* dest_func_t)();
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);
        RET ret = (obj_ptr->*(registed_data.real_func))();
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename RET>
struct class_function_traits_t<RET(FUNC_CLASS_TYPE::*)(ARG1)>
{
    typedef RET(FUNC_CLASS_TYPE::* dest_func_t)(ARG1);
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1));
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2,
    typename RET>
struct class_function_traits_t<RET(FUNC_CLASS_TYPE::*)(ARG1, ARG2)>
{
    typedef RET(FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2);
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2));
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3,
    typename RET>
struct class_function_traits_t<RET(FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3)>
{
    typedef RET(FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3);
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3));
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename RET>
struct class_function_traits_t<RET(FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4)>
{
    typedef RET(FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4);
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4));
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename RET>
struct class_function_traits_t<RET(FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5)>
{
    typedef RET(FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5);
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5));
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6, typename RET>
struct class_function_traits_t<RET(FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
    ARG6)>
{
    typedef RET(FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg6);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6));
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6, typename ARG7, typename RET>
struct class_function_traits_t<RET(FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
    ARG6, ARG7)>
{
    typedef RET(FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        typename basetype_ptr_t<ARG7>::arg_type arg7 = init_value_t<typename basetype_ptr_t<ARG7>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_stack<typename basetype_ptr_t<ARG7>::arg_type>::to_value(ls_, LUA_ARG_POS(8), arg7);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7));
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename RET>
struct class_function_traits_t<RET(FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
    ARG6, ARG7, ARG8)>
{
    typedef RET(FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        typename basetype_ptr_t<ARG7>::arg_type arg7 = init_value_t<typename basetype_ptr_t<ARG7>::arg_type>::value();
        typename basetype_ptr_t<ARG8>::arg_type arg8 = init_value_t<typename basetype_ptr_t<ARG8>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_stack<typename basetype_ptr_t<ARG7>::arg_type>::to_value(ls_, LUA_ARG_POS(8), arg7);
        lua_op_stack<typename basetype_ptr_t<ARG8>::arg_type>::to_value(ls_, LUA_ARG_POS(9), arg8);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7), p_t<ARG8>::r(arg8));
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9, typename RET>
struct class_function_traits_t<RET(FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
    ARG6, ARG7, ARG8, ARG9)>
{
    typedef RET(FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        typename basetype_ptr_t<ARG7>::arg_type arg7 = init_value_t<typename basetype_ptr_t<ARG7>::arg_type>::value();
        typename basetype_ptr_t<ARG8>::arg_type arg8 = init_value_t<typename basetype_ptr_t<ARG8>::arg_type>::value();
        typename basetype_ptr_t<ARG9>::arg_type arg9 = init_value_t<typename basetype_ptr_t<ARG9>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_stack<typename basetype_ptr_t<ARG7>::arg_type>::to_value(ls_, LUA_ARG_POS(8), arg7);
        lua_op_stack<typename basetype_ptr_t<ARG8>::arg_type>::to_value(ls_, LUA_ARG_POS(9), arg8);
        lua_op_stack<typename basetype_ptr_t<ARG9>::arg_type>::to_value(ls_, LUA_ARG_POS(10), arg9);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7), p_t<ARG8>::r(arg8), p_t<ARG9>::r(arg9));
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};


template <typename FUNC_CLASS_TYPE, typename RET>
struct class_function_traits_t<RET(FUNC_CLASS_TYPE::*)() const>
{
    typedef RET(FUNC_CLASS_TYPE::* dest_func_t)() const;
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);
        RET ret = (obj_ptr->*(registed_data.real_func))();
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename RET>
struct class_function_traits_t<RET(FUNC_CLASS_TYPE::*)(ARG1) const>
{
    typedef RET(FUNC_CLASS_TYPE::* dest_func_t)(ARG1) const;
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1));
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2,
    typename RET>
struct class_function_traits_t<RET(FUNC_CLASS_TYPE::*)(ARG1, ARG2) const>
{
    typedef RET(FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2) const;
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2));
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3,
    typename RET>
struct class_function_traits_t<RET(FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3) const>
{
    typedef RET(FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3) const;
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3));
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename RET>
struct class_function_traits_t<RET(FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4) const>
{
    typedef RET(FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4) const;
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4));
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename RET>
struct class_function_traits_t<RET(FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5) const>
{
    typedef RET(FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5) const;
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5));
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6, typename RET>
struct class_function_traits_t<RET(FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
    ARG6) const>
{
    typedef RET(FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) const;
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg6);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6));
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6, typename ARG7, typename RET>
struct class_function_traits_t<RET(FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
    ARG6, ARG7) const>
{
    typedef RET(FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) const;
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        typename basetype_ptr_t<ARG7>::arg_type arg7 = init_value_t<typename basetype_ptr_t<ARG7>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_stack<typename basetype_ptr_t<ARG7>::arg_type>::to_value(ls_, LUA_ARG_POS(8), arg7);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7));
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename RET>
struct class_function_traits_t<RET(FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
    ARG6, ARG7, ARG8) const>
{
    typedef RET(FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8) const;
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        typename basetype_ptr_t<ARG7>::arg_type arg7 = init_value_t<typename basetype_ptr_t<ARG7>::arg_type>::value();
        typename basetype_ptr_t<ARG8>::arg_type arg8 = init_value_t<typename basetype_ptr_t<ARG8>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_stack<typename basetype_ptr_t<ARG7>::arg_type>::to_value(ls_, LUA_ARG_POS(8), arg7);
        lua_op_stack<typename basetype_ptr_t<ARG8>::arg_type>::to_value(ls_, LUA_ARG_POS(9), arg8);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7), p_t<ARG8>::r(arg8));
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9, typename RET>
struct class_function_traits_t<RET(FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
    ARG6, ARG7, ARG8, ARG9) const>
{
    typedef RET(FUNC_CLASS_TYPE::* dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9) const;
    typedef userdata_for_function_t<dest_func_t>        userdata_for_function_info;

    static  int lua_function(lua_State* ls_)
    {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        typename basetype_ptr_t<ARG7>::arg_type arg7 = init_value_t<typename basetype_ptr_t<ARG7>::arg_type>::value();
        typename basetype_ptr_t<ARG8>::arg_type arg8 = init_value_t<typename basetype_ptr_t<ARG8>::arg_type>::value();
        typename basetype_ptr_t<ARG9>::arg_type arg9 = init_value_t<typename basetype_ptr_t<ARG9>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_stack<typename basetype_ptr_t<ARG7>::arg_type>::to_value(ls_, LUA_ARG_POS(8), arg7);
        lua_op_stack<typename basetype_ptr_t<ARG8>::arg_type>::to_value(ls_, LUA_ARG_POS(9), arg8);
        lua_op_stack<typename basetype_ptr_t<ARG9>::arg_type>::to_value(ls_, LUA_ARG_POS(10), arg9);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7), p_t<ARG8>::r(arg8), p_t<ARG9>::r(arg9));
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename CLASS_TYPE, typename RET>
struct class_property_traits_t
{
    typedef int (*process_index_func_t)(lua_State*, void*, const char*);
    typedef int (*process_newindex_func_t)(lua_State*, void*, const char*, int);

    typedef RET property_t;
    typedef RET CLASS_TYPE::* property_ptr_t;
    static int process_index(lua_State* ls_, void* field_info_, const char* key_)
    {
        /* stack status:
         * p1 (userdata) 类实例
         * mt_index_function
         */
        typedef class_property_info_t<property_ptr_t> class_property_info_t;
        CLASS_TYPE* obj_ptr = metatable_register_impl_t<CLASS_TYPE>::userdata_to_object(ls_);

        class_property_info_t* reg = (class_property_info_t*)field_info_;
        property_ptr_t ptr = reg->property_pos;

        if (ptr)
        {
            lua_op_stack<property_t>::push_stack(ls_, (obj_ptr->*ptr));
            return 1;
        }
        else
        {
            printf("none this field<%s>\n", key_);
            return 0;
        }

        return 0;
    }

    static int process_newindex(lua_State* ls_, void* field_info_, const char* key_, int value_index_)
    {
        typedef class_property_info_t<property_ptr_t> class_property_info_t;
        CLASS_TYPE* obj_ptr = metatable_register_impl_t<CLASS_TYPE>::userdata_to_object(ls_);

        class_property_info_t* reg = (class_property_info_t*)field_info_;
        property_ptr_t ptr = reg->property_pos;

        if (ptr)
        {
            property_t  value = init_value_t<property_t>::value();
            lua_op_stack<property_t>::to_value(ls_, value_index_, value);
            (obj_ptr->*ptr) = value;
            return 0;
        }
        else
        {
            printf("none this field<%s>\n", key_);
            return 0;
        }

        return 0;
    }
};

template<>
struct function_traits_t<void(*)()>
{
    typedef void (*dest_func_t)();
    static  int lua_function(lua_State* ls_)
    {
        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));

        dest_func_t& registed_func = *((dest_func_t*)user_data);
        registed_func();
        return 0;
    }
};


template <typename ARG1>
struct function_traits_t<void(*)(ARG1)>
{
    typedef void (*dest_func_t)(ARG1);
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(1), arg1);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        registed_func(arg1);
        return 0;
    }
};

template <typename ARG1, typename ARG2>
struct function_traits_t<void(*)(ARG1, ARG2)>
{
    typedef void (*dest_func_t)(ARG1, ARG2);
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg2);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        registed_func(arg1, arg2);
        return 0;
    }
};

template <typename ARG1, typename ARG2, typename ARG3>
struct function_traits_t<void(*)(ARG1, ARG2, ARG3)>
{
    typedef void (*dest_func_t)(ARG1, ARG2, ARG3);
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg3);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        registed_func(arg1, arg2, arg3);
        return 0;
    }
};

template <typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct function_traits_t<void (*)(ARG1, ARG2, ARG3, ARG4)>
{
    typedef void (*dest_func_t)(ARG1, ARG2, ARG3, ARG4);
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg4);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        registed_func(arg1, arg2, arg3, arg4);
        return 0;
    }
};

template <typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5>
struct function_traits_t<void (*)(ARG1, ARG2, ARG3, ARG4, ARG5)>
{
    typedef void (*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5);
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg5);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        registed_func(arg1, arg2, arg3, arg4, arg5);
        return 0;
    }
};
template <typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6>
struct function_traits_t<void (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6)>
{
    typedef void (*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg6);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        registed_func(arg1, arg2, arg3, arg4, arg5, arg6);
        return 0;
    }
};

template <typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6, typename ARG7>
struct function_traits_t<void (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)>
{
    typedef void (*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        typename basetype_ptr_t<ARG7>::arg_type arg7 = init_value_t<typename basetype_ptr_t<ARG7>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg6);
        lua_op_stack<typename basetype_ptr_t<ARG7>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg7);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        registed_func(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
        return 0;
    }
};

template <typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct function_traits_t<void (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8)>
{
    typedef void (*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        typename basetype_ptr_t<ARG7>::arg_type arg7 = init_value_t<typename basetype_ptr_t<ARG7>::arg_type>::value();
        typename basetype_ptr_t<ARG8>::arg_type arg8 = init_value_t<typename basetype_ptr_t<ARG8>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg6);
        lua_op_stack<typename basetype_ptr_t<ARG7>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg7);
        lua_op_stack<typename basetype_ptr_t<ARG8>::arg_type>::to_value(ls_, LUA_ARG_POS(8), arg8);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        registed_func(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
        return 0;
    }
};


template <typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct function_traits_t<void (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9)>
{
    typedef void (*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        typename basetype_ptr_t<ARG7>::arg_type arg7 = init_value_t<typename basetype_ptr_t<ARG7>::arg_type>::value();
        typename basetype_ptr_t<ARG8>::arg_type arg8 = init_value_t<typename basetype_ptr_t<ARG8>::arg_type>::value();
        typename basetype_ptr_t<ARG9>::arg_type arg9 = init_value_t<typename basetype_ptr_t<ARG9>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg6);
        lua_op_stack<typename basetype_ptr_t<ARG7>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg7);
        lua_op_stack<typename basetype_ptr_t<ARG8>::arg_type>::to_value(ls_, LUA_ARG_POS(8), arg8);
        lua_op_stack<typename basetype_ptr_t<ARG9>::arg_type>::to_value(ls_, LUA_ARG_POS(9), arg9);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        registed_func(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
        return 0;
    }
};


template <typename RET>
struct function_traits_t<RET(*)()>
{
    typedef RET(*dest_func_t)();
    static  int lua_function(lua_State* ls_)
    {
        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        RET ret = registed_func();
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);

        return 1;
    }
};

template <typename RET, typename ARG1>
struct function_traits_t<RET(*)(ARG1)>
{
    typedef RET(*dest_func_t)(ARG1);
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(1), arg1);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        RET ret = registed_func(arg1);
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2>
struct function_traits_t<RET(*)(ARG1, ARG2)>
{
    typedef RET(*dest_func_t)(ARG1, ARG2);
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg2);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        RET ret = registed_func(arg1, arg2);
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3>
struct function_traits_t<RET(*)(ARG1, ARG2, ARG3)>
{
    typedef RET(*dest_func_t)(ARG1, ARG2, ARG3);
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg3);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        RET ret = registed_func(arg1, arg2, arg3);
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct function_traits_t<RET(*)(ARG1, ARG2, ARG3, ARG4)>
{
    typedef RET(*dest_func_t)(ARG1, ARG2, ARG3, ARG4);
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg4);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        RET ret = registed_func(arg1, arg2, arg3, arg4);
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5>
struct function_traits_t<RET(*)(ARG1, ARG2, ARG3, ARG4, ARG5)>
{
    typedef RET(*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5);
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg5);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        RET ret = registed_func(arg1, arg2, arg3, arg4, arg5);
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};


template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6>
struct function_traits_t<RET(*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6)>
{
    typedef RET(*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg6);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        RET ret = registed_func(arg1, arg2, arg3, arg4, arg5, arg6);
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6, typename ARG7>
struct function_traits_t<RET(*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)>
{
    typedef RET(*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        typename basetype_ptr_t<ARG7>::arg_type arg7 = init_value_t<typename basetype_ptr_t<ARG7>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg6);
        lua_op_stack<typename basetype_ptr_t<ARG7>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg7);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        RET ret = registed_func(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct function_traits_t<RET(*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8)>
{
    typedef RET(*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        typename basetype_ptr_t<ARG7>::arg_type arg7 = init_value_t<typename basetype_ptr_t<ARG7>::arg_type>::value();
        typename basetype_ptr_t<ARG8>::arg_type arg8 = init_value_t<typename basetype_ptr_t<ARG8>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg6);
        lua_op_stack<typename basetype_ptr_t<ARG7>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg7);
        lua_op_stack<typename basetype_ptr_t<ARG8>::arg_type>::to_value(ls_, LUA_ARG_POS(8), arg8);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        RET ret = registed_func(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
    typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct function_traits_t<RET(*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9)>
{
    typedef RET(*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    static  int lua_function(lua_State* ls_)
    {
        typename basetype_ptr_t<ARG1>::arg_type arg1 = init_value_t<typename basetype_ptr_t<ARG1>::arg_type>::value();
        typename basetype_ptr_t<ARG2>::arg_type arg2 = init_value_t<typename basetype_ptr_t<ARG2>::arg_type>::value();
        typename basetype_ptr_t<ARG3>::arg_type arg3 = init_value_t<typename basetype_ptr_t<ARG3>::arg_type>::value();
        typename basetype_ptr_t<ARG4>::arg_type arg4 = init_value_t<typename basetype_ptr_t<ARG4>::arg_type>::value();
        typename basetype_ptr_t<ARG5>::arg_type arg5 = init_value_t<typename basetype_ptr_t<ARG5>::arg_type>::value();
        typename basetype_ptr_t<ARG6>::arg_type arg6 = init_value_t<typename basetype_ptr_t<ARG6>::arg_type>::value();
        typename basetype_ptr_t<ARG7>::arg_type arg7 = init_value_t<typename basetype_ptr_t<ARG7>::arg_type>::value();
        typename basetype_ptr_t<ARG8>::arg_type arg8 = init_value_t<typename basetype_ptr_t<ARG8>::arg_type>::value();
        typename basetype_ptr_t<ARG9>::arg_type arg9 = init_value_t<typename basetype_ptr_t<ARG9>::arg_type>::value();

        lua_op_stack<typename basetype_ptr_t<ARG1>::arg_type>::to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_stack<typename basetype_ptr_t<ARG2>::arg_type>::to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_stack<typename basetype_ptr_t<ARG3>::arg_type>::to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_stack<typename basetype_ptr_t<ARG4>::arg_type>::to_value(ls_, LUA_ARG_POS(4), arg4);
        lua_op_stack<typename basetype_ptr_t<ARG5>::arg_type>::to_value(ls_, LUA_ARG_POS(5), arg5);
        lua_op_stack<typename basetype_ptr_t<ARG6>::arg_type>::to_value(ls_, LUA_ARG_POS(6), arg6);
        lua_op_stack<typename basetype_ptr_t<ARG7>::arg_type>::to_value(ls_, LUA_ARG_POS(7), arg7);
        lua_op_stack<typename basetype_ptr_t<ARG8>::arg_type>::to_value(ls_, LUA_ARG_POS(8), arg8);
        lua_op_stack<typename basetype_ptr_t<ARG9>::arg_type>::to_value(ls_, LUA_ARG_POS(9), arg9);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        RET ret = registed_func(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
        lua_op_stack<typename basetype_ptr_t<RET>::arg_type>::push_stack(ls_, ret);
        return 1;
    }
};

template<typename RET>
struct mlua_register_router_t<RET(*)()>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(*arg_)(), const string& s_)
    {
        reg_->def_func(arg_, s_);
    }
};
template<typename RET, typename ARG1>
struct mlua_register_router_t<RET(*)(ARG1)>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(*arg_)(ARG1), const string& s_)
    {
        reg_->def_func(arg_, s_);
    }
};
template<typename RET, typename ARG1, typename ARG2>
struct mlua_register_router_t<RET(*)(ARG1, ARG2)>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(*arg_)(ARG1, ARG2), const string& s_)
    {
        reg_->def_func(arg_, s_);
    }
};
template<typename RET, typename ARG1, typename ARG2, typename ARG3>
struct mlua_register_router_t<RET(*)(ARG1, ARG2, ARG3)>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(*arg_)(ARG1, ARG2, ARG3), const string& s_)
    {
        reg_->def_func(arg_, s_);
    }
};
template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct mlua_register_router_t<RET(*)(ARG1, ARG2, ARG3, ARG4)>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(*arg_)(ARG1, ARG2, ARG3, ARG4), const string& s_)
    {
        reg_->def_func(arg_, s_);
    }
};
template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
struct mlua_register_router_t<RET(*)(ARG1, ARG2, ARG3, ARG4, ARG5)>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(*arg_)(ARG1, ARG2, ARG3, ARG4, ARG5), const string& s_)
    {
        reg_->def_func(arg_, s_);
    }
};
template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
struct mlua_register_router_t<RET(*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6)>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(*arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6), const string& s_)
    {
        reg_->def_func(arg_, s_);
    }
};
template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
struct mlua_register_router_t<RET(*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(*arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7), const string& s_)
    {
        reg_->def_func(arg_, s_);
    }
};template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct mlua_register_router_t<RET(*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8)>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(*arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8), const string& s_)
    {
        reg_->def_func(arg_, s_);
    }
};
template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct mlua_register_router_t<RET(*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9)>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(*arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9), const string& s_)
    {
        reg_->def_func(arg_, s_);
    }
};

template<typename RET, typename CLASS_TYPE>
struct mlua_register_router_t<RET(CLASS_TYPE::*)()>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(CLASS_TYPE::* arg_)(), const string& s_)
    {
        reg_->def_class_func(arg_, s_);
    }
};
template<typename RET, typename CLASS_TYPE, typename ARG1>
struct mlua_register_router_t<RET(CLASS_TYPE::*)(ARG1)>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(CLASS_TYPE::* arg_)(ARG1), const string& s_)
    {
        reg_->def_class_func(arg_, s_);
    }
};
template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2>
struct mlua_register_router_t<RET(CLASS_TYPE::*)(ARG1, ARG2)>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(CLASS_TYPE::* arg_)(ARG1, ARG2), const string& s_)
    {
        reg_->def_class_func(arg_, s_);
    }
};
template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3>
struct mlua_register_router_t<RET(CLASS_TYPE::*)(ARG1, ARG2, ARG3)>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(CLASS_TYPE::* arg_)(ARG1, ARG2, ARG3), const string& s_)
    {
        reg_->def_class_func(arg_, s_);
    }
};
template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct mlua_register_router_t<RET(CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4)>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(CLASS_TYPE::* arg_)(ARG1, ARG2, ARG3, ARG4), const string& s_)
    {
        reg_->def_class_func(arg_, s_);
    }
};
template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
struct mlua_register_router_t<RET(CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5)>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(CLASS_TYPE::* arg_)(ARG1, ARG2, ARG3, ARG4, ARG5), const string& s_)
    {
        reg_->def_class_func(arg_, s_);
    }
};
template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
struct mlua_register_router_t<RET(CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6)>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(CLASS_TYPE::* arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6), const string& s_)
    {
        reg_->def_class_func(arg_, s_);
    }
};
template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
struct mlua_register_router_t<RET(CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(CLASS_TYPE::* arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7), const string& s_)
    {
        reg_->def_class_func(arg_, s_);
    }
};template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct mlua_register_router_t<RET(CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8)>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(CLASS_TYPE::* arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8), const string& s_)
    {
        reg_->def_class_func(arg_, s_);
    }
};
template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct mlua_register_router_t<RET(CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9)>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(CLASS_TYPE::* arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9), const string& s_)
    {
        reg_->def_class_func(arg_, s_);
    }
};

template<typename RET, typename CLASS_TYPE>
struct mlua_register_router_t<RET(CLASS_TYPE::*)() const>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(CLASS_TYPE::* arg_)() const, const string& s_)
    {
        reg_->def_class_func(arg_, s_);
    }
};
template<typename RET, typename CLASS_TYPE, typename ARG1>
struct mlua_register_router_t<RET(CLASS_TYPE::*)(ARG1) const>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(CLASS_TYPE::* arg_)(ARG1) const, const string& s_)
    {
        reg_->def_class_func(arg_, s_);
    }
};
template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2>
struct mlua_register_router_t<RET(CLASS_TYPE::*)(ARG1, ARG2) const>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(CLASS_TYPE::* arg_)(ARG1, ARG2) const, const string& s_)
    {
        reg_->def_class_func(arg_, s_);
    }
};
template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3>
struct mlua_register_router_t<RET(CLASS_TYPE::*)(ARG1, ARG2, ARG3) const>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(CLASS_TYPE::* arg_)(ARG1, ARG2, ARG3) const, const string& s_)
    {
        reg_->def_class_func(arg_, s_);
    }
};
template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct mlua_register_router_t<RET(CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4) const>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(CLASS_TYPE::* arg_)(ARG1, ARG2, ARG3, ARG4) const, const string& s_)
    {
        reg_->def_class_func(arg_, s_);
    }
};
template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
struct mlua_register_router_t<RET(CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5) const>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(CLASS_TYPE::* arg_)(ARG1, ARG2, ARG3, ARG4, ARG5) const, const string& s_)
    {
        reg_->def_class_func(arg_, s_);
    }
};
template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
struct mlua_register_router_t<RET(CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) const>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(CLASS_TYPE::* arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) const, const string& s_)
    {
        reg_->def_class_func(arg_, s_);
    }
};
template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
struct mlua_register_router_t<RET(CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) const>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(CLASS_TYPE::* arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) const, const string& s_)
    {
        reg_->def_class_func(arg_, s_);
    }
};template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct mlua_register_router_t<RET(CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8) const>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(CLASS_TYPE::* arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8) const, const string& s_)
    {
        reg_->def_class_func(arg_, s_);
    }
};
template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct mlua_register_router_t<RET(CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9) const>
{
    template<typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET(CLASS_TYPE::* arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9) const, const string& s_)
    {
        reg_->def_class_func(arg_, s_);
    }
};

