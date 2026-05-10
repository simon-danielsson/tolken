<h1 align="center">tolken</h1>
  
<p align="center">
    <img src="https://img.shields.io/badge/C_version-23-cyan?style=flat-square" alt="C version" />
    <img src="https://img.shields.io/badge/license-MIT-green?style=flat-square" alt="MIT License" />
  <img src="https://img.shields.io/github/last-commit/simon-danielsson/tolken/main?style=flat-square&color=blue" alt="Last commit" />
</p>
  
<p align="center">
  <a href="#info">Info</a> •
  <a href="#usage">Usage</a> •
  <a href="#license">License</a>
</p>  

---
<div id="info"></div>

## Info
    
Header-only C library for building simple TUI applications for unix systems.
  
> [!IMPORTANT]  
> This library is not trying to compete with libraries such as ncurses or crossterm - this is only meant to be a fun project and nothing more.  
  
Inspirations, sources and references:
- [nob.h](https://github.com/tsoding/nob.h)  
- [stb](https://github.com/nothings/stb)  
- [ANSI cheatsheet](https://gist.github.com/ConnerWill/d4b6c776b509add763e17f9f113fd25b)  
  
---
<div id="usage"></div>

## Usage
  
Copy [tlk.h](./tlk.h) into your project and include it like this:  
  
```c
#define TLK_IMPLEMENTATION
#include "./tlk.h"
```
  
To get an idea of what this library is capable of, clone this repo and run the
[example programs](./examples):  
``` bash
$ cenv example browser # read-only file browser
$ cenv example todo    # todo app
$ cenv example snake   # snake game
$ cenv example conway  # game of life
```
  
---
<div id="license"></div>

## License
This project is licensed under the [MIT License](https://github.com/simon-danielsson/tolken/blob/main/LICENSE).  
