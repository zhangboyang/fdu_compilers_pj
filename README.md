# MiniJava Compiler
Compilers course project

## Introduction
* Implemented all MiniJava features (including inheritance and virtual function)
* **Generate EXE executable directly (a fully operational compiler)**
* Based on flex and bison (lexer and parser)
* Hand-written backend (including code generator and linker)

# MiniJava 编译器
2014 级编译课程大作业

## 简介
* 实现了 MiniJava 的全部内容（包括继承、虚函数）
* **直接生成 EXE 可执行文件（能够真正使用）**
* 使用 flex 和 bison 进行词法、语法分析
* 全手写的编译器后端（包括代码生成、链接器）

## 组员分工
姓名|学号|分工
---|---|---
张博洋|14307130078|前端类层次设计、后端设计
曹景辰|14307130003|前端语法文件编写、图形界面设计


## 编译方式
本工程使用 C++ 语言编写，Visual Studio 2017 作为开发环境。因为在 git 里已经配置好了 *win_flex_bison*，所以无须对 flex 和 bison 进行额外的配置。

**已有编译好的可执行文件**在 `src\minijavac\Release\minijavac.exe` 处，若要自己重新编译，只要打开 `src\minijavac\minijavac.sln`，然后选择“Release”解决方案配置，再选择“生成”菜单中的“重新生成解决方案”即可。

## 运行方式
命令行语法为 `minijavac [源代码文件名]`，运行后若成功编译，则会生成以下文件：

文件名|说明
----|----
`out.ast.txt`|文本形式的语法树输出（使用记事本查看）
`out.ast.json`|**JSON 格式的语法树输出（使用编译器图形界面查看）**
`out.var.txt`|变量地址分配表
`out.asm.txt`|机器码与反汇编输出
`out.exe`|**EXE 可执行文件，可以直接运行**

## 自动测试
`test` 目录下带有 MiniJava 网站上的几个样例测试程序，也有几个自己编写的测试程序。

* 运行 `test\run_tests.bat` 可以执行自动测试。若全部显示 `OK` 则说明通过了测试。
* 运行 `test\make_answer.bat` 可以生成标准答案（需要安装并配置好 JDK）。
