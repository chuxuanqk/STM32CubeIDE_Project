## IAP简介

IAP（In Application Programming）即在应用编程，IAP 是用户自己的程序在运行过程中对
User Flash 的部分区域进行烧写，目的是为了在产品发布后可以方便地通过预留的通信口对产
品中的固件程序进行更新升级。 通常实现 IAP 功能时，即用户程序运行中作自身的更新操作，
需要在设计固件程序时编写两个项目代码，第一个项目程序不执行正常的功能操作，而只是通
过某种通信方式(如 USB、USART)接收程序或数据，执行对第二部分代码的更新；第二个项目
代码才是真正的功能代码。这两部分项目代码都同时烧录在 User Flash 中，当芯片上电后，首
先是第一个项目代码开始运行，它作如下操作：
1）检查是否需要对第二部分代码进行更新
2）如果不需要更新则转到 4）
3）执行更新操作
4）跳转到第二部分代码执行

我们将第一个项目代码称之为 Bootloader 程序，第二个项目代码称之为 APP 程序，他们存
放在 STM32 FLASH 的不同地址范围，一般从最低地址区开始存放 Bootloader，紧跟其后的就
是 APP 程序。

<img src="D:\Saber_Workshop\Personal\Doc\Markdown_Note\嵌入式相关笔记\STM32_IAP.assets\image-20210224114206995.png" alt="image-20210224114206995 " style="zoom:75%;" />

<img src="D:\Saber_Workshop\Personal\Doc\Markdown_Note\嵌入式相关笔记\STM32_IAP.assets\image-20210224114238832.png" alt="image-20210224114238832 " style="zoom:75%;" />



<img src="D:\Saber_Workshop\Personal\Doc\Markdown_Note\嵌入式相关笔记\STM32_IAP.assets\Boot_App.png" alt="img" style="zoom:70%;" />

**中断向量表的偏移量设置方法**

```c
NVIC_SetVectorTable(NVIC_VectTab_FLASH, APP_BASE_ADDR);
```

**程序烧录方式**

采用[STM32CubeProgrammer](https://www.st.com/zh/development-tools/stm32cubeprog.html)软件进行程序烧录，选择好Address(程序运行地址)和Size(烧录程序大小)。

1.进行BootLoader烧录

<img src="D:\Saber_Workshop\Personal\Doc\Markdown_Note\嵌入式相关笔记\STM32_IAP.assets\image-20210224135617948.png" alt="image-20210224135617948" style="zoom:50%;" />

2.进行APP1烧录

<img src="D:\Saber_Workshop\Personal\Doc\Markdown_Note\嵌入式相关笔记\STM32_IAP.assets\image-20210224135831563.png" alt="image-20210224135831563" style="zoom:50%;" />

运行，通过串口打印输出，会看到以下打印消息。说明BootLoader和APP1已经成功运行。

![image-20210224135821503](D:\Saber_Workshop\Personal\Doc\Markdown_Note\嵌入式相关笔记\STM32_IAP.assets\image-20210224135821503.png)

3.生成BootLoader+APP完整的应用程序，通过该工具的读取Device memory功能生成App.bin文件用于程序发布。

<img src="D:\Saber_Workshop\Personal\Doc\Markdown_Note\嵌入式相关笔记\STM32_IAP.assets\image-20210224140224324.png" alt="image-20210224140224324" style="zoom:50%;" />![image-20210224140351993](D:\Saber_Workshop\Personal\Doc\Markdown_Note\嵌入式相关笔记\STM32_IAP.assets\image-20210224140351993.png)

![image-20210224140351993](D:\Saber_Workshop\Personal\Doc\Markdown_Note\嵌入式相关笔记\STM32_IAP.assets\image-20210224140351993.png)

## 参考文献

[STM32在线升级OTA](https://mculover666.blog.csdn.net/article/details/106658908)

[在线升级OTA—(另一种思路)](https://blog.csdn.net/weixin_41294615/article/details/105122644)