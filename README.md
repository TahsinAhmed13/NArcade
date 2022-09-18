# NArcade

This is a repo for learning C and Ncurses. Most of the stuff will be games but there might be other things too. The code is not meant to be super optimized so there probably will be global variables and dangling pointers, but who cares it's C! 

# What is Ncurses?

Ncurses is a really cool library written for C, with wrappers in other languages, that adds more terminal functionality such as moving the cursor and using the mouse. It's great for writing terminal based applications. 

# Why Ncurses?

Most people will learn C and pointers and all that stuff, but when they're tasked with writing something in C they'll find themselves frustrated because they didn't really learn C. I wanted to really learn C and the best way to do that would be to program with it. However, without any kind of graphics engine the things one can make are often limited and mundane. But at the same time most graphics libraries, like SFML, are bloated and using pure OpenGL is way harder than C itself and distracts from the purpose of the project. Ncurses on the other hand is simple but powerful and is the perfect library to use to learn C. 

# How to use

Each program has its own Makefile.\
To run use the following command.

```
make && make clean
```
