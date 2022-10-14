
test_var = 99
root = nil

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
    local model = Model:new("../resources/objects/cyborg/cyborg.obj")
    -- model 为userdata，仅保存Model实例的指针，该函数返回后，
    -- root 为userdata，仅保存Node指针
    root = model.root
    return true
end

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
