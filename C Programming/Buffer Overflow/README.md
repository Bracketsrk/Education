# Stack Buffer Overflows

Buffer overflows are the product of poor programming practices. 
They can lead to many different outcomes, from program crashes to full
user compromise.
The exploitation of them may be very obvious, or may require tricky manipulation of memory. 

A buffer overflow occurs when a buffer is overrun by input(from a file or user). Because a buffer
is just a pointer to a memory space, whatever was being loaded into the buffer when it is overrun will
spill into the memory space after the buffer. What this means will become clearer later in this writeup.

The C program file in this folder has a stack buffer overflow vulnerability in it.

#### **NOTE**: In order for the stack to be overflown, the program must be compiled with:
```
gcc -o overflow overflow.c -fno-stack-protector
```
<br>
The purpose of the overflow.c program is to take someone's password from the command line and give them authorization if their password is correct.
<br><br><br>

```
if (authenticated(argv[1])) {
    printf("***********\n");
    printf("* I'm in. *\n");
    printf("***********\n");
}
else {
    printf("Sorry, that ain't it. :(\n");
}
```
We can see that the if statement takes the authenticated() function as its condition.
Let's see what the function does so we know what the program needs to authenticate.
<br><br>

```
int authenticated(char *password) {
	int auth = 0;
	char buf[20];

	strcpy(buf, password);

	if (strcmp(password, "hunter2") == 0) {
		auth = 1;
	}

	return auth;	
}
```
The authenticated function takes a char array, copies it to a buffer,
then compares the password to the buffer. As the password is plaintext, 
we can authenticat with that.

```
MyComp:Desktop MyUser$ ./overflow hunter2
***********
* I'm in. *
***********
MyComp:Desktop MyUser$
```

We did it! But let's find another way to do it... 

Taking a look back at the conditional in the main function, the 'if' statement only
needs to be given a 'true' boolean to authenticate us. The authenticate function does not
return 'true', but instead returns '1' when the password is correct. This works because
any non-zero number in C is 'true', and '0' is false.

What this means is that if we can somehow change the 'auth' integer to anything other than 0,
we will be authenticated. Let's try something else.

```
MyComp:Desktop MyUser$ ./overflow AAAA
Sorry, that ain't it. :(
MyComp:Desktop MyUser$ ./overflow AAAAAAAAAA
Sorry, that ain't it. :(
MyComp:Desktop MyUser$ ./overflow AAAAAAAAAAAAAAAAAAA
Sorry, that ain't it. :(
MyComp:Desktop MyUser$ ./overflow AAAAAAAAAAAAAAAAAAAAAAAAAAA
Segmentation fault: 11
MyComp:Desktop MyUser$ ./overflow AAAAAAAAAAAAAAAAAAAAAAAA
***********
* I'm in. *
***********
MyComp:Desktop MyUser$ 
```

What happened there?

### Before:
|0x0|0x4|0x8|0xC|0xF|
----|---|---|---|---|
| A | A | A | A | A |
| A | A | A | A | A |
| A | A | A | A | A |
| A | A | A | A | A |
| A | A | A | A | A |

|0x0|0x4|0x8|0xC|0xF|
----|---|---|---|---|
| A | A | A | A | A |
| A | A | A | A | A |
| A | A | A | A | A |
| A | A | A | A | A |
| A | A | A | A | A |
|
