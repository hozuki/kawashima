**From early October 2016, kawashima is merging into [libcgss](https://github.com/hozuki/libcgss).**

---

# kawashima

[kawashima](https://github.com/Hozuki/kawashima) is a library to decode [CRI HCA audio](http://www.criware.com/en/products/adx2.html).
It is named after [Mizuki Kawashima](http://www.project-imas.com/wiki/Mizuki_Kawashima).

Its original code is from [here](https://mega.co.nz/#!Fh8FwKoB!0xuFdrit3IYcEgQK7QIqFKG3HMQ6rHKxrH3r5DJlJ3M), in public
domain. Some adaptations are made to pack the functions into a shared library.

For how-tos, please see the [API docs](https://github.com/Hozuki/kawashima/tree/master/docs/api.md) and
[example](https://github.com/Hozuki/kawashima/tree/master/docs/example.md).

Beware that, if you want to integrate the library or any piece of code, **DO AT YOUR OWN RISK**.

There is a C# P/Invoke wrapper for kawashima at [DereTore.HCA.Native](https://github.com/Hozuki/DereTore/tree/master/DereTore.HCA.Native).

## Building kawashima

### Visual C++

kawashima requires Visual C++ 2010 as the minimum version to build. Open `kawashima.sln` to start building.

### Cygwin

kawashima requires CMake 3.3 (in fact, it can be lower) as the minimum version to build. Rename
`CMakeLists-example-cygwin.txt` to `CMakeLists.txt` and use CMake to start building.

### MinGW

The CMake version requirement is the same as building under Cygwin. Rename `CMakeLists-example-mingw.txt`
to `CMakeLists.txt` and use CMake to start building.

------

[Disclaimer](DISCLAIMER.md)

[WTFPL](LICENSE.txt)
<a href="http://www.wtfpl.net/txt/copying/">![WTFPL Badge](http://www.wtfpl.net/wp-content/uploads/2012/12/wtfpl-badge-2.png)</a>
