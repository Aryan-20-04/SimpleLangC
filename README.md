# MyLang – A Simple C-Based Programming Language

MyLang is a small experimental programming language written in C.  
It supports:

- Numeric variables
- Arithmetic operations (`+`, `-`, `*`, `/`)
- Comparisons (`==`, `!=`, `<`, `>`, `<=`, `>=`)
- Strings (normal, raw `r"..."`, and triple-quoted `"""..."""`)
- Multi-type `print` statements (mix text and expressions)
- Conditional execution with `if`

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
### 5. Conditionals
```text
let x = 10;

if (x == 10) {
    print "x is 10";
}

if (x > 5) {
    print "x is greater than 5";
}
```
## Folder Structure
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

## How to Compile
### Option 1: Using GCC
```text
cd MyLang/src
gcc main.c lexer.c interpreter.c symbol.c -o ../build/mylang
./../build/mylang
```

### Option 2: Using Make
From the project root
```text
make
make run
```
## Sample program.txt
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

if (x < y) {
    print "x is less than y";
}

if (x == 13) {
    print "x is exactly 13";
}
```
### Expected Output
```text
13
15
x + y : 28
Hello World
C:\Users\Projects
Line1
Line2
Line3
x is less than y
x is exactly 13
```