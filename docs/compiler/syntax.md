# FS Language Syntax Guide

This document describes the syntax of the **FS programming language**.  
FS is a C-like language with a few tweaks to keywords and syntax rules.

---

## 1. Functions

>  **Note:** Program should have a main entry function.
### Declaration
```fs
func main() -> int {
    /*function body*/
}
```
- **func** keyword replaces standard C declaration.
- Return type comes after **->**.
---
### Example
```fs
func add(int a, int b) -> int {
    return a + b;
}
```
## 2. Variables
```fs
int x = 5;        /* integer variable*/
int[5] arr;      /*one-dimensional array*/
int* p = &x;      /*pointer*/
int** pp = &p;    /*pointer to pointer*/
```
- Only **int** is supported as base type. (Can be expanded in the future).
- Arrays are one dimensional only. (Can be expanded in the future).

## 3. Control Flow
### If / Else
```fs
if (x > 0) { 
    x--; 
} else { 
    x++; 
}
```
- Since langugage does not support(can be supported in the future) single-line **if** or **else** statements we are not able to use **else if**. To get same behaviour as **else if** we can write:
```fs
    if( x == 123){
        /*body for x == 123*/
    }
    else{
        if( x == 124){
            /*body for x == 124*/
        }
        else{
            if( x == 126){
                /*body for x == 126*/
            }
            else{
                /*default else body*/
            }
        }
    }
```
### For Loop 
- The iterator must be declared and initialized before the loop.
```fs
int i = 0;
for (i...<=10; i++) {
    /*loop body*/
}
```
- Equivalent to C’s **for(i = 0; i <= 10; i++)**.

### While Loop
```fs
while (x > 0) { x--; }
```
### Skip / Break
- **skip** replaces C's **continue**.
- **break** works the same.
```fs
for (i...=5; i++) {
    if (i == 2) { skip;  }
    if (i == 4) { break; } 
}
```
## 4. Operators

- Arithmetic: **+** **-** **\*** **/** **%**

- Bitwise : **<<** **>>** **|** **&** **~**
- Comparison: **== != < > <= >=**

- Logical: **&& || ! and or**

- Assignment: **= += -= \*= /=**

- Pointers: **\*ptr** (dereference), **&var** (address-of) , **\*(ptr+val)** (pointer arithmetic), **ptr[i]** (indexing)


