# IOCtrlIO
控制，用于LED或有源蜂鸣器等等的控制，可方便实现IO的周期与占空比控制，控制参数 （低电平时间，高电平时间，总持续时间）
# 优点
采用链表的方式动态创建IO控制资源，执行效率高占用内存小
# 使用说明
IOCtrl_CallBackProc函数必须被周期循环调用，其入口参数为每次调用的间隔时间，单位为毫秒。
如，在中断时间为10ms的定时器中断服务程序中：
 
 void 定时器中断函数(void) 
 {  
  IOCtrl_CallBackProc(10);
 }
 
或者在操作系统中创建任务周期调用。
IO控制资源的申请、删除、启动和停止见IOCtrl_demo.c中的示例，示例程序中以控制一个LED为例。

