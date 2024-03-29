import build;
import fs;
import console as cs;

fn #build(opt: build_opts)
{
    opt.build_module("/lib/");
    fs::copy("/assets/", opt.out_dir + "/assets/");
    opt.emit_executable("program", main);
}


Foo: struct
{
    num: i32;
    happy: bool;
}

Bar: struct
{
    angry: bool;
    fooby: Foo*!; // Non nullable, owning ptr
}

fn Foo::add(x: i32)     // implicit '-> void' return type
{
    num += x;
}


trait ops::Add<Foo>
{
    fn add(a: Foo, b: Foo)
    {
        return Foo { a.num + b.num, a.happy || b.happy };
    }
}


fn min<T: numeric>(a: T, b: T)
{
    // numerics are guarranteed to have <, >, +, - ... operators defined
    if (a > b)
    {
        return b;
    }
    return a;
}

fn main() -> i32
{
    fo: Foo = { 10, true };
    fo.add(5);

    cs::printf("fo.num: {}", fo.num); // Prints 15

    // baz: Bar;        error: Bar::fooby is not nullable
    
    baz: Bar = { true, alloc(Foo) { 3, true } }; // heap allocates a Foo
        

    
    return 0;
    // baz.fooby is automatically freed    
}
