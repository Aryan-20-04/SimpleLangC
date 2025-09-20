# SimpleLangC

A small interpreter for a **C-like scripting language** written in C.  
Supports variables, arithmetic, control flow, arrays, and built-in functions like `length()`.

---

## ✨ Features

### Variables & Arithmetic

```text
let x = 5;
let y = 10;
print x + y;   // 15
```

### If Statements

```text
let n = 7;
if (n > 5) {
    print "Greater than 5";
} else {
    print "Less or equal to 5";
}
```

### For Loops

```text
for (let i=0; i<5; i=i+1) {
    print i;
}
```

### Arrays

```text
let arr = [10, 23, 2, 21];
print arr;     // [10, 23, 2, 21]
print arr[2];  // 2
Arrays are 0-based indexed.
Use length(arr) to get the number of elements.
```

### Array Assignment

```text
let arr = [1, 2, 3];
arr[1] = 99;
print arr;     // [1, 99, 3]

Sorting Example (Bubble Sort)
Copy code
let arr=[10,23,2,21,4,6,1,3,9,7,90,22];

for (let i=0; i<length(arr); i=i+1){
    for (let j=i+1; j<length(arr); j=j+1){
        if (arr[i] > arr[j]) {
            let temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
}

print arr;      // [1, 2, 3, 4, 6, 7, 9, 10, 21, 22, 23, 90]
print arr[2];   // 3
```

### Functions

```text

function add(a, b) {
    return a + b;
}

let result = add(5, 10);
print result;
```

```text
Explanation:

function name(params) { ... } defines a function.

Functions can return numeric values using return.

Parameters are passed by value.
```

### Recursion

```text
Functions can call themselves:

function factorial(n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

print factorial(5); // 120
```

```text
Explanation:

Recursion works by maintaining a separate local scope in the symbol table.

Each recursive call has its own parameter bindings.
```

#### Bubble Sort Example

```text
Sort an array using a function:

let arr = [40, 10, 20, 12, 43, 9, 86, 23];
print arr;

function Bubble(arr) {
    for (let i = 0; i < length(arr); i = i + 1) {
        for (let j = 0; j < length(arr) - i - 1; j = j + 1) {
            if (arr[j] > arr[j + 1]) {
                let t = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = t;
            }
        }
    }
}

Bubble(arr);
print arr;
```

### Build & Run (Only Linux)

#### Install

```text
make all      # build the interpreter
sudo make install      # run sample program
```

#### Uninstall

```text
sudo make uninstall
```

### Run (at any folder)

```text
Make sure to have a .slc file extension
```

```bash
slangc filename.slc
```

### Language Grammar (Simplified)

#### Variables

```text
let x = 10;
let arr = [1, 2, 3];
```

#### Expressions

```text
+, -, *, /, ==, !=, <, <=, >, >=
```

#### Control Flow

```text
if (expr) { ... } else { ... }
for (init; cond; incr) { ... }
```

#### Arrays

```text
arr[i] = value;
print arr;       # pretty-prints entire array
print arr[2];    # prints element
length(arr);
```

### Notes & Limitations

```text
Arrays are numeric only (no strings inside arrays yet).

Out-of-bounds array access will print an error and return 0.

print always adds a newline at the end.

Strings must use double quotes "text".
```

### Example Program

```text
programs/program.txt

let arr=[10,23,2,21,4,6,1,3,9,7,90,22];

for (let i=0; i<length(arr); i=i+1){
    for (let j=i+1; j<length(arr); j=j+1){
        if (arr[i] > arr[j]) {
            let temp=arr[i];
            arr[i]=arr[j];
            arr[j]=temp;
        }
    }
}

print "Sorted Array:";
print arr;
print "3rd Element:";
print arr[2];
```

### Output

```text
Sorted Array:
[1, 2, 3, 4, 6, 7, 9, 10, 21, 22, 23, 90]
3rd Element:
3
```

### Author

```text
Built as a toy interpreter project in C to learn parsing, ASTs, and interpreters.
```

### By- Aryan Sinha
