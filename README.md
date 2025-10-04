# KOT: A portable C like scripting language

This is an experiment made because bash has a pretty bad syntax, so why can't I just elaborate 
something that is interpreted and also have a normal syntax with some macro options?

This support the main C standard syntax for normal source code file, this means that syntactical preprocessor 
or linking keyword or statement are not implemented, and example is the storage classes for function ( there is no 
static, extern ). 

The executable will accept input from a provided source code or it will read from the standard input the informations 
provided. This interpretest is similar to a stack based machine, each line of code is evaluated and translated to an 
intermediate representation.
Working with the standard C syntax allow a pretty simple and intuitive writing process for the scripting code, and example 
can be found inside "ex.c", but to cut it short if a function is defined it's possible to call it by name like in normal C:

    void foo(){
        printf("hello");
    }

    foo();

This machine can accept a "configuration file" that can be sourced just like .bashrc files. This can contain interpreter global variable
or configuration line for the interpreter. This will be loaded as a file from the filesystem and this is important: been able to read 
and/or write file for KOT is omportant and it will use the standard C function fread() and fwrite() to do that, this means that trying 
to compile this interpreter for a machine that is not based on a POSIX standard or that didn't use an operative system with 
a minimum of runtime environment it's not possible, make sure that your final runtime environment has the proper standard function 
and runtime/kernel call's to read or write a file system. 



