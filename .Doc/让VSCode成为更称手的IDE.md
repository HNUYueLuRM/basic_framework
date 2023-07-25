# 让VSCode变成你熟悉的形状

VSCode的各种配置如快捷键、高亮颜色、主题、界面形状和位置等（也包括各种插件的设置）可以通过在`ctrl+,`打开的设置页面修改，更好的方式是你已经尝试过的——通过xxx.json文件进行配置。VSCode的配置系统同样通过`.json`文件完成，且存在继承和覆盖的关系。VSCode的底层配置`setting.json`是针对整个VSCode进行设置的，而工作区目录下创建的`.vscode/setting.json`则可以覆盖底层配置。了解了基本的配置结构后，这里将介绍一些入手必备的设置和推荐安装的插件。

[TOC]

## 快捷键





 

## 提高效率





## 代码高亮





## 终端工具







## 插件

> 学习一个插件的使用，最好的方式是阅读插件的wiki和说明文档，而不是在搜索引擎里面搜索！询问ChatGPT或者Copilot Chat也是一个比较好的办法，初级的问题它们几乎不会犯错。

- **Better C++ Syntax**

用于静态解析C++代码，为intellisense以及language server提供代码高亮和完整的智能提示选项。

- **Blockman**

为代码分块。不同的作用域会被浅色背景边框包围，同时高亮当前focus的代码块，方便在大段代码中定位程序控制流。

- **Bookmarks**

为代码添加书签，可以在左侧tab页中跳转到对应位置，方便阅读代码时往复查看，也有助于阅读理解。右键点击代码行号左侧（即打断点的地方）可以添加书签或带label的书签。

- **C/C++ Snippets**

为基本的语句（关键字）提供代码补全，如输入`for`自动生成下面的代码：

```c
for (size_t i = 0; i < count; i++)
{
  /* code */
}
```

补全之后，按下tab会进入不同的位置，方便进一步修改snippet。你也可以在VSCode中自定义常用的snippet补全。

- **Code Issue Manager**

可以在代码的任意地方插入“便签”和“评注”，支持多人协作。是一个比注释更好的TODO list和注意事项提醒。插入的issue支持markdown格式。

- **Doxygen Documention Generator**

为你的代码生成doxygen文档格式的注释。同时也支持一些基本的注释块生成。输入`/**`再按下回车会根据当前注释的位置自动生成合适的注释块。本框架中的注释均通过该插件生成。

- **Github Copilot / Copilot Labs / Copilot Chat**

体验大模型的威力。尤其是Copilot Chat非常适合为你提供一门语言的入门级咨询。

- **Gitlens**

方便地通过图形化的方式管理你的git仓库。

- **HexEditor & Hex Hover Converter**

以十六进制编辑文件 & 鼠标悬停在任意数值类型上时自动提供2/8/16进制的转换。

- **Live Share**

和你的伙伴一起coding。

- **SonarLint**

VSCode上最强大的静态检查工具，在VSCode自动静态检查的基础上提供更严格的代码建议，尽可能降低出错的可能。其中也包含了不同语言的最佳实践。

cmake可以通过添加`DCMAKE_EXPORT_COMPILE_COMMANDS=True` 的指令（直接在cmakelists中通过`set()`设定也可以），makefile则通过Makefile Tools插件设置生成`compile_commands.json`的路径。

