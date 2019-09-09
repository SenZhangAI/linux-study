## linux-study
linux5.2.7源码阅读笔记

## 说明

`usr/include` 文件取自 Linux系统 `/usr/include` 包含我感兴趣的函数声明，例如`#include <sys/select.h>` 来自`/usr/include/sys/select.h` 头文件，其实现取自linux源码。

## 内容

### select 剖析

参考 <http://www.pandademo.com/2016/11/linux-kernel-select-source-dissect/>
