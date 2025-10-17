# KOT: C like scripting language

This is an experiment made because bash has a pretty bad syntax, so why can't I just elaborate 
something that is interpreted and also have a normal syntax with some macro options?

The idea is to create some sort of scripting language that can support a syntax close to the C one, allowing 
a simples and more immediate way to write script and execute things. My intention is to come up with 
something that can be used like bash, that doesn't need to depend on too many things beside the source 
codes binded within this repo. 

The makefile will generate an executable that can be used as a "shell" by itself, this means that by just executing 
it without providing any type of source code it will start a shell-like environment that let you type instructions. 
Since having this way of writing may cause some trouble inside the shell mode there is a "supervisor" mode that can 
be enabled by typing "kot" inside the shell mode. 
In "supervisor" mode you can get the helper by typing "h" to list the available commands. 

The console mode and the file execution mode support a "compilation process" that have a proper error notification system 
that will tell you what you did wrong, this make kot more like a "interpreted" languare rather that just a scripting 
language, it have an interpretation stage that compiler down each instrution in a special machine code that is 
executed inside a simple virtual machine to create a turing complete language, scope definition and proper 
function separation. 


Kot allow the definition of new function, but it already has a set of primitive ones already defined to simplify
input/output and general operation from the standard library. For more specific usage it's possible to define 
new custom functions that serve as waytrough from kot to the machine where your are executing the vm. For example 
if you need to add support for "memcpy" or "memset" you can create a new function bind inside the source code 
that wrap those function and provide the frontend api to kot, then from kot your're will be able to acces those 
functions without problems. 

I used "memcpy" as an example for a reason, since this is an interpreted language it doesn't really have a proper 
dedicated memory, it has a pre-allocated amount of "RAM" that is used as a stack and the main heap. Of course 
if you define support for memcpy you need to access this byte-array without problem and kot provide standard 
api to directly manipulate the virtual machine memory inside predefined array to allocate inside the stack or 
to allocate inside the heap.

This still is under development, so many things may change in future.
