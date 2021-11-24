# KDevDlang project

Adds D language features and Dub project management to kdevelop.

KDevDlang is a plugin which adds D language ([dlang](https://dlang.org/)) features to KDevelop. See Features for what's supported.

This project is in beta stage. It is work in progress. Expect things to fail. Help by posting issues or sending pull requests is greatly appreciated.

## Features

* Autocompletion and Problem reporting
* Dub project management integration
* Source Formatting (depends on dfmt)
* Source Code analysis (depends on dscanner)
* Project and file templates

## Installation

Run the following in a terminal:

```
$ git clone --recurse submodules https://github.com/jaapgeurts/kdev-dlang
$ cd kdev-dlang/dlang/parser
$ ./build.sh
$ cd ../../
$ mkdir build
$ cd build
$ cmake ..
$ make
# make install
```

Note: This procedure hasn't been tested yet. Please open an issue in case of problems or, better yet, fix issues and issue a pull request.

## Why?

But there is Visual Studio Code??? Why did you make this?

1. For learning and fun.
2. Electron is slow and huge!
3. Control of our own computers. I think Microsoft has long term plans to move Visual Studio Code into the cloud. They will support VS Code for as long as is needed to reach that goal. Once developers are in the cloud, support will stop.
4. KDevelop is a fantastic program that deserves more love!

## Thanks

These plugins are based on the work of others. A huge thanks to:
Thomas Brix Larsen for the language plugin(completion), Astyle plugin developers, qmake plugin developers, cppcheck developers.

## Autocompletion support
## Dub project management support
## Source Formatting support
## Source Code analysis support
## Project and file templates support
