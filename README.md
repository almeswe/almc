## almc
![license MIT](https://img.shields.io/badge/license-MIT-blue>)
![status](https://www.repostatus.org/badges/latest/active.svg)
 
### About
   x86-64 compiler written on C from scratch as hobby project (without flex, bison etc.)                            <br />
   Targets Linux platform only, uses GNU toolchain to compile itself, and for compiling programs on this language.  <br />
   There are not user-friendly interface yet, this project mostly serves as educational purpose.     				<br />

### Brief structure

```bash
── almc
   ├── back-end
   │   └── x86_64
   │      ├── ir  (not fully impl yet)
   │      └── gen (not fully impl yet)
   └── front-end
       ├── syntactic analysis
       │   ├── lexer  (NFA)
       │   ├── parser (recursive descent)
       │   └── ast structure n evaluator 
       ├── semantic analysis
       │   ├── symbol table
       │   ├── ast-resolver
       │   └── type-resolver
       └── utils
          ├── os
          ├── memory          (libc wrappers)
          └── data structures (stretchy buffers)
```

### TODO
- Full support for initializers


### Compilation
   Compiled with **gcc 9.4.0**

### Licence
   This project was released under [MIT](LICENSE) license.
