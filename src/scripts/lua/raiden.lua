--定义了Lua端的各种类型

function rgb(r,g,b)
    return {type="rgb",value={r,g,b}}
end

function texture(name)
    return {type="texture",value=name}
end

function float_array( ... )
    temp = { ... }  
    arg = select("1",temp) 
    return {type="float[]",value=arg} 
end

function int_array( ... )
    temp = { ... }  
    arg = select("1",temp) 
    return {type="int[]",value=arg} 
end
