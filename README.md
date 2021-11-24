# KDevDlang project

Adds D language features to kdevelop.

Why when there is Visual Studio Code?
1. For learning and fun.
2. Electron is slow and huge!
3. Control of our own computers. I think Microsoft has long term plans to move Visual Studio Code into the cloud. They will support VS Code for as long as is needed to reach that goal. Once developers are in the cloud, support will stop.
4. KDevelop is a fantastic program that deserves more love!

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
$ cd kdev-dlang
$ mkdir build
$ cd build
$ cmake ..
$ make
# make install
```

Note: This procedure hasn't been tested yet. Please open an issue in case of problems or, better yet, fix issues and issue a pull request.

## Autocompletion support
## Dub project management support
## Source Formatting support
## Source Code analysis support
## Project and file templates support
