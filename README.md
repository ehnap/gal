Gal
===

[![License](https://img.shields.io/github/license/ehnap/gal.svg)](https://github.com/ehnap/gal/blob/master/LICENSE)
[![Code size](https://img.shields.io/github/languages/code-size/ehnap/gal.svg)](https://github.com/Wox-launcher/Wox/releases)
[![RamenBless](https://cdn.rawgit.com/LunaGao/BlessYourCodeTag/master/tags/alpaca.svg)](https://github.com/LunaGao/BlessYourCodeTag)

**Gal == Gal is A Launcher**

**Gal** is a launcher for Windows that simply works. It's an alternative to [WoX](http://wox.one)、[Alfred](https://www.alfredapp.com/) and [Launchy](http://www.launchy.net/). You can call it Windows omni-eXecutor if you want a long name.


Features
--------

- Search for everything—applications, files and more.
- Use *pinyin* to search for programs / 支持用 **拼音** 搜索程序
  - biying → 必应缤纷桌面
- Keyword plugin search 
  - search bing with `bing search_term`


Installation
------------

Download `gal-xxx.exe` from [releases](https://github.com/ehnap/gal/releases). Latest as of now is [`2.0.0`](https://github.com/ehnap/gal/releases/download/v2.0.0/gal-2.0.0-x64.exe) 


- Requirements:
  - [everything](https://www.voidtools.com/): `.exe` installer + use x64 if your windows is x64 + everything service is running

Usage
-----

- Launch: <kbd>Alt</kbd>+<kbd>Space</kbd>
- Extend Menu: <kbd>Alt</kbd>+<kbd>Right</kbd>

Build
-----

1. Install Visual Studio 2017 and tick all Windows 10 sdk options
2. in project dir. 
```
cd ..
mkdir galproject
qmake -t vcapp ..\gal\galapp\gal.pro
msbuild gal.vcxproj
```

Documentation
-------------
- Just ask questions in [issues](https://github.com/ehnap/gal/issues) for now.