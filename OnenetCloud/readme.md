远端：

NB模块进行鸿蒙化的移植，能通过AT指令实现NB模块使用CoAP协议与OneNet云平台连接，

开发板通过uart连接nb-iot模块，通过nb-iot模块，接收来自OneNet云平台的消息，实现云平台控制，同时将水表传感器接收到的数据上报给OneNet。

APP能读取华为云储存的数据，并下发指令控制水表。

近端：

开发板连接手机二点，实现手机的快速读取，更快速的控制。

![425fdf0f6926d3b746103ea55222c2c](C:\GitHub\OnenetCloud\Device\425fdf0f6926d3b746103ea55222c2c.png)