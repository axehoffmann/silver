import math;

foo: struct 
{
    num: i32;
    bazzy: bool;
}

// Map a foo like an 'optional' container
// 'using' first arg treats it like a member function
map: fn(using ob: *foo, f: fn(*i32)) // implicit -> void type
{
    if (ob.bazzy)
    {
        f(*ob.num);
    }
}

main: fn() -> i32
{
    bar: foo = { 1, false };
    assert(bar.num == 1);

    mapper: fn(x: *i32) { x += 10 };
    
    map(*bar, mapper);
    assert(bar.num == 1);

    bar.bazzy = true;
    map(*bar, mapper);
    assert(bar.num == 11);
}

