
/* 
 *
 * Multi line comment 
 *
*/



#define DD()\
	X(ONE)\
	X(TWO)


#define X(name) name,

enum{
	DD()
	TEST_ENUM_LIMIT
}test_enum

#undef X


void foo(){
	printf("foo() called");
}


int a = 34;
int b = 35;

test_enum = ONE;

printf("Hello, World: %d\n", a+b);
print("Hello ${a+b}\n"); // custom printf wrapper 
foo();



