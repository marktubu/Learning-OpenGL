
test_var = 99

function dump_table(tb, str)
    if nil == str then str = "" end
    for k, v in pairs(tb)
    do
        print(str, k, v)
    end
end

Mod = {}
function Mod:funcTest1(arg1, arg2)
    print("in funcTest1:", self, arg1, arg2)
    return true
end
Mod.funcTest1()
-- 测试调用lua
function test_func(arg1, arg2, arg3, arg4)
    print("in test_func:", arg1, arg2, arg3, arg4)
    mp = {["k"] = "v"}
    vc = {1,2,3}
    lt = {4,5,6}
    st = {7,8,9}
    dumy(mp, vc, lt, st)
    return arg1
end

-- 接受stl参数
function test_stl(vec, lt, st, mp)
    print("--------------dump_table begin ----------------")
    dump_table(vec, "vec")
    dump_table(lt, "lt")
    dump_table(st, "st")
    dump_table(mp, "mp")
    print("--------------dump_table end ----------------")
    return "ok"
end

-- 返回stl 参数
function test_return_stl_vector()
    return {1,2,3,4}
end
function test_return_stl_list()
    return {1,2,3,4}
end
function test_return_stl_set()
    return {1,2,3,4}
end
function test_return_stl_map()
    return {
        ["key"] = 124
    }
end
-- 测试接受C++对象
function test_object(foo_obj)
    --测试构造
    base = base_t:new()
    -- 每个对象都有一个get_pointer获取指针
    print("base ptr:", base:get_pointer())
    -- 测试C++对象函数
    foo_obj:print(12333, base)
    base:delete()
    --基类的函数
    foo_obj:dump()
    -- 测试C++ 对象属性
    print("foo property", foo_obj.a)
    print("base property", foo_obj.v)
end

-- 测试返回C++对象
function test_ret_object(foo_obj)
    return foo_obj
end

-- 测试返回C++对象
function test_ret_base_object(foo_obj)
    return foo_obj
end

    
clazz:static_func()