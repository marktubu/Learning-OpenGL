#include <string>
#include <vector>
#include <map>
#include <set>

using namespace std;

#include "Lua.h"

#include "mlua.h"
#include "mlua_op.h"

#include "../graphics/Model.h"
#include "../core/Tree.h"
//#include "../core/GameObject.h"

//! lua talbe �����Զ�ת��Ϊstl ����
void dumy(std::map<std::string, std::string> ret, std::vector<int> a, std::list<std::string> b, std::set<int64_t> c)
{
    printf("in %s begin ------------\n", __FUNCTION__);
    for (map<string, string>::iterator it = ret.begin(); it != ret.end(); ++it)
    {
        printf("map:%s, val:%s:\n", it->first.c_str(), it->second.c_str());
    }
    printf("in %s end ------------\n", __FUNCTION__);
}

//
//static void lua_reg(lua_State* ls)
//{
//    //! ע����ຯ��, ctor() Ϊ���캯��������
//    fflua_register_t<base_t, ctor()>(ls, "base_t")  //! ע�ṹ�캯��
//        .def(&base_t::dump, "dump")     //! ע�����ĺ���
//        .def(&base_t::v, "v");          //! ע����������
//
////! ע�����࣬ctor(int) Ϊ���캯���� foo_tΪ�������ƣ� base_tΪ�̳еĻ�������
//    fflua_register_t<foo_t, ctor(int)>(ls, "foo_t", "base_t")
//        .def(&foo_t::print, "print")        //! ����ĺ���
//        .def(&foo_t::a, "a");               //! ������ֶ�
//
//    fflua_register_t<>(ls)
//        .def(&dumy, "dumy");                //! ע�ᾲ̬����
//
//
//    fflua_register_t<clazz, ctor()>(ls, "clazz")
//        .def(&clazz::static_func, "static_func");
//
//    fflua_register_t<Node, ctor()>(ls, "Node");
//
//    /*fflua_register_t<GameObject, ctor(std::string)>(ls, "GameObject", "Node")
//        .def(&GameObject::name, "name");*/
//
//    fflua_register_t<Model, ctor(std::string)>(ls, "Model")
//        .def(&Model::root, "root");
//}

int test_func(lua_State* L) {
    return 0;
}

void Lua::Test1() {
    mlua mlua;
    lua_pushstring(mlua.ls, "test string");
    lua_pushstring(mlua.ls, "test string 222");
    lua_pushnumber(mlua.ls, 111);
    lua_pushboolean(mlua.ls, false);
    lua_pushlightuserdata(mlua.ls, mlua.ls);
    lua_pushcfunction(mlua.ls, &test_func);
    lua_pushnil(mlua.ls);
    lua_newtable(mlua.ls);
    lua_pushinteger(mlua.ls, 1);
    lua_pushstring(mlua.ls, "tvalue");
    lua_settable(mlua.ls, -3);
    lua_pushinteger(mlua.ls, 2);
    lua_pushinteger(mlua.ls, 2);
    lua_settable(mlua.ls, -3);
    lua_pushstring(mlua.ls, "tkey");
    lua_pushstring(mlua.ls, "tvalue");
    lua_settable(mlua.ls, -3);

    mlua_tool::dump_stack(mlua.ls);
}

Node* Lua::Test() {
    Node* root = nullptr;
    mlua fflua;
    try
    {
        //fflua.setModFuncFlag(true);
        //! ע��C++ ����lua��
        //fflua.reg(lua_reg);

        //! ����lua�ļ�
        fflua.add_package_path("../resources/lua/");
#ifdef _WIN32
        fflua.do_file("../resources/lua/test.lua");
#else
        fflua.do_file("test.lua");
#endif
        int arg1 = 12;
        float arg2 = 2;
        bool ret = fflua.call<bool>("Mod:funcTest1", arg1);
        //fflua.get_global_variable("root", root);
        std::cout << "func ret " << ret << std::endl;
        //std::cout << "root " << root << "   name " << root->Children().size() << std::endl;
    }
    catch (exception& e)
    {
        printf("exception:%s\n", e.what());
    }

    return root;
}

//
//void Lua::Test2()
//{
//    fflua_t fflua;
//    try
//    {
//        fflua.setModFuncFlag(true);
//        //! ע��C++ ����lua��
//        fflua.reg(lua_reg);
//
//        //! ����lua�ļ�
//        fflua.add_package_path("../resources/lua/");
//#ifdef _WIN32
//        fflua.load_file("../resources/lua/test.lua");
//#else
//        fflua.load_file("test.lua");
//#endif
//        //! ��ȡȫ�ֱ���
//        int var = 0;
//        assert(0 == fflua.get_global_variable("test_var", var));
//        //! ����ȫ�ֱ���
//        assert(0 == fflua.set_global_variable("test_var", ++var));
//
//        //! ִ��lua ���
//        fflua.run_string("print(\"exe run_string!!\")");
//
//        //! ����lua����, ����������Ϊ����
//        int32_t arg1 = 1;
//        float   arg2 = 2;
//        double  arg3 = 3;
//        string  arg4 = "4";
//        fflua.call<int32_t>("test_func", arg1, arg2, arg3, arg4);
//        fflua.call<bool>("Mod:funcTest1", arg1, arg2);
//
//        //! ����lua������stl������Ϊ������ �Զ�ת��Ϊlua talbe
//        vector<int> vec;        vec.push_back(100);
//        list<float> lt;         lt.push_back((float)99.99);
//        set<string> st;         st.insert("OhNIce");
//        map<string, int> mp;    mp["key"] = 200;
//        fflua.call<string>("test_stl", vec, lt, st, mp);
//
//        //! ����lua �������� talbe���Զ�ת��Ϊstl�ṹ
//        vec = fflua.call<vector<int> >("test_return_stl_vector");
//        lt = fflua.call<list<float> >("test_return_stl_list");
//        st = fflua.call<set<string> >("test_return_stl_set");
//        mp = fflua.call<map<string, int> >("test_return_stl_map");
//
//        //! ����lua������c++ ������Ϊ����, foo_t ���뱻ע���
//        foo_t* foo_ptr = new foo_t(456);
//        fflua.call<void>("test_object", foo_ptr);
//
//        //! ����lua������c++ ������Ϊ����ֵ, foo_t ���뱻ע��� 
//        assert(foo_ptr == fflua.call<foo_t*>("test_ret_object", foo_ptr));
//        //! ����lua������c++ ������Ϊ����ֵ, �Զ�ת��Ϊ����
//        base_t* base_ptr = fflua.call<base_t*>("test_ret_base_object", foo_ptr);
//        assert(base_ptr == foo_ptr);
//
//    }
//    catch (exception& e)
//    {
//        printf("exception:%s\n", e.what());
//
//
//    }
//#ifdef _WIN32
//    system("pause");
//#endif
//}
