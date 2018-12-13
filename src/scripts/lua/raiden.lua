--定义了Lua端的各种类型

function rgb(r,g,b)
    return {"rgb",{r,g,b}}
end

function point3f(x,y,z)
    return {"point3f",{x,y,z}}
end

function texture(name)
    return {"texture",name}
end

function float_array( ... )
    temp = { ... }  
    arg = select("1",temp) 
    return {"float[]",arg} 
end

function int_array( ... )
    temp = { ... }  
    arg = select("1",temp) 
    return {"int[]",arg} 
end
