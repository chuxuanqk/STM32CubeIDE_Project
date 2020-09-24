<!--

 * @Author: your name
 * @Date: 2020-09-17 16:28:33
 * @LastEditTime: 2020-09-19 11:09:47
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \Demo_Std_Project\doc\README.md
-->
# 编译链工具

arm-none-eabi-gcc

# 系统模块

- [x] uart驱动模块
    * 添加串口总线发送控制功能
    * 串口中断中接收缓存区控制功能
    
- [x] can驱动模块

    * 中断中添加接收处理钩子函数

    * 采用循环发送，由应用层控制发送时的处理逻辑

- [x] Flash模块
    * 读取配置数据
    * 写入配置数据
    
- [ ] 按键模块
    * 控制按键 (4路)                                 
    * 配置按键 (1路)   
    
- [ ] 指示灯模块
    * 网络指示灯 (CAN 、USART)
    * 继电器指示灯(4路)
* 提供动态配置LED灯刷新率
    * 完成功能测试
    
- [x] 系统定时器模块

    * 定时器调度ms级
    * 系统滴答时钟ms级
    * 完成功能测试

- [ ] BL0940模块

    * 模块初始化
    * 数据接收
    * 数据传输

- [ ] BL0940业务逻辑

    * 数据采集
    * 数据处理
    * 异常数据处理及上报

    
    
- [ ]                                                                  Modbus协议模块