# Stack Buffer Overflows

Buffer overflows are the product of poor programming practices. 
They can lead to many different outcomes, from program crashes to full
user compromise.
The exploitation of them may be very obvious, or may require tricky manipulation of memory. 

A buffer overflow occurs when a buffer is overrun by input(from a file or user). Because a buffer
is just a pointer to a memory space, whatever was being loaded into the buffer when it is overrun will
spill into the memory space after the buffer. What this means will become clearer later in this writeup.

The C program file in this folder has a stack buffer overflow vulnerability in it.

#### **NOTE**: In order to be able to overflow the stack, the program must be compiled with:
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
We can see that the if statement takes the **authenticated()**  function as its condition.
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
we can authenticate with that.

```
MyComp:Desktop MyUser$ ./overflow hunter2
***********
* I'm in. *
***********
MyComp:Desktop MyUser$
```

We did it! But let's find another way to do it... 

Taking a look back at the conditional in the main function, the **if** statement only
needs to be given a **true** boolean to authenticate us. The authenticate function does not
return **true**, but instead returns **1** when the password is correct. This works because
any non-zero number in C is **true**, and **0** is false.

What this means is that if we can somehow change the **auth** integer to anything other than **0**,
we will be authenticated. Let's try something else.

```
MyComp:Desktop MyUser$ ./overflow AAAA
Sorry, that ain't it. :(
MyComp:Desktop MyUser$ ./overflow AAAAAAAAAA
Sorry, that ain't it. :(
MyComp:Desktop MyUser$ ./overflow AAAAAAAAAAAAAAAAAAA
Sorry, that ain't it. :(
MyComp:Desktop MyUser$ ./overflow AAAAAAAAAAAAAAAAAAAAAAAAAAAAA
Segmentation fault: 11
MyComp:Desktop MyUser$ ./overflow AAAAAAAAAAAAAAAAAAAAAAAA
***********
* I'm in. *
***********
MyComp:Desktop MyUser$ 
```

What happened there?

Let's look at the variables on the stack in the **authenticated()** function.

|      |0x0000|0x0004|0x0008|0x000C|
|------|------|------|------|------|
| 0x0  | buf  | buf  | buf  | buf  |
| 0x4  | buf  | auth | sfp  | ret  |

<br>

### Before input:
|      |0x0000|0x0004|0x0008|0x000C|
|------|------|------|------|------|
| 0x0  | buf  | buf  | buf  | buf  |
| 0x4  | buf  | 0x0  | sfp  | ret  |


### After input:
|      |0x0000|0x0004|0x0008|0x000C|
|------|------|------|------|------|
| 0x0  | AAAA | AAAA | AAAA | AAAA |
| 0x4  | AAAA | AAAA | A??? | ret  |

<br>

In the before chart, you can see that the **buf** 
character array takes up 20 bytes, and the
**auth** integer takes 4 bytes. The reason that they appear in this order in the stack is because
they are loaded onto the stack in reverse order. When the function is called, the stack pointer (the pointer
that keeps track of where the stack is) makes room for **auth** by moving itself back 4 bytes, then moves
itself back 20 bytes to make room for **buf**.

<br>

Looking back at the program, the `strcpy(buf, password);` line
does not check that the length of what we are putting into the **buf** variable is at most 20 bytes. Because of this,
we are able to give the program more than it can store in **buf**, and because the next variable over is 
**auth**, it will flow into that.

<br>

You'll notice that the input in the **after** chart also overrode what was previously labeled **sfp**. 
This is called the *saved frame pointer*, and it keeps track of where the current stack frame 
is so that the program knows where to work. After the **sfp** is the return address (**ret**) which tells the 
function where to go when it exits. In this case, the return addres points 
to the main function, so when the function exits it will return to main().

<br>

So why did overflowing the **buf** array give us authorization?

<br>

When the buffer flowed into the **auth** variable, it replaced what **auth** was (0x0) with the
buffer(0x41414141)(Note: it is \x41 because this is hex for 'A').

Even though the password wasn't correct, the function returns the **auth** variable at the end.
The authenticating conditional, `if (authenticated(argv[1]))`, only checks if the returned value
is non-zero. 0x41414141 is non-zero, so we make the **if** statement true!

<br>
<br>

## Overwriting the Return Address

You might have noticed earlier that when the buffer was too long, we got a 
segmentation fault.
```
MyComp:Desktop MyUser$ ./overflow AAAAAAAAAAAAAAAAAAAAAAAAAAAAA
Segmentation fault: 11
```

You can see in the charts below that when we input 30 A's, the return pointer was overwriten,
and so the program doesn't know where to go after the function finishes.

### Before input:
|      |0x0000|0x0004|0x0008|0x000C|
|------|------|------|------|------|
| 0x0  | buf  | buf  | buf  | buf  |
| 0x4  | buf  | 0x0  | sfp  | ret  |


### After input:
|      |0x0000|0x0004|0x0008|0x000C|
|------|------|------|------|------|
| 0x0  | AAAA | AAAA | AAAA | AAAA |
| 0x4  | AAAA | AAAA | AAAA | AA?? |

<br>

If the function tried to jump to this return address, it would be jumping to 0x????4141. If we can 
control where the program goes after the function ends, then we could make the program jump to functions or run shellcode, allowing code execution.
