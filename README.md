# MyLang – A Simple C-Based Programming Language

MyLang is a small experimental programming language written in C.  
It supports:

- Numeric variables
- Arithmetic operations (`+` and `-`)
- Strings (normal, raw `r"..."`, and triple-quoted `"""..."""`)
- Multi-type `print` statements (mix text and expressions)

---

## Features

### 1. Variables

```text
let x = 10;  // numeric variable
let y = 5;
```
### 2. Arithmetic
```text
print x + y;  // Outputs: 15
```
### 3. Strings
```text
print "Hello World";
print r"C:\Users\Projects";
print """Line1
Line2
Line3""";
```
### 4. Multi-type Print
```text
print "x + y: " x + y;  // Outputs: x + y: 15
```
### Folder Structure
```text
MyLang/
├── src/                  # Source files
│   ├── main.c
│   ├── lexer.c
│   ├── lexer.h
│   ├── interpreter.c
│   ├── interpreter.h
│   ├── symbol.c
│   └── symbol.h
├── programs/             # Sample programs
│   └── program.txt
├── build/                # Compiled binaries (ignored by git)
├── Makefile              # Optional build automation
└── README.md
```
### How to Compile
#### Option 1: Using GCC
```text
cd MyLang/src
gcc main.c lexer.c interpreter.c symbol.c -o ../build/mylang
./../build/mylang
```
#### Option 2: Using Make

##### From the project root
```text
make
make run
```
#### Sample program.txt
```text
let x = 13;
let y = 15;
print x;
print y;
print "x + y : " x + y;
print "Hello World";
print r"C:\Users\Projects";
print """Line1
Line2
Line3""";
```
#### Expected Output
```text
Program Starting....
13
15
x + y : 28
Hello World
C:\Users\Projects
Line1
Line2
Line3
Program Completed.
```