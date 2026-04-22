一主四从通信协议： 主控 STM32     通信方式CAN

主机->从机：

标准帧一字节

从机1 CAN ID :0x010
从机2 CAN ID :0x101
从机3 CAN ID :0x102
从机4 CAN ID :0x103

控制字ctrl 的 bit 定义：
bit0：request_send
1 = 请求从机发送一次压力/温度数据
0 = 不请求发送
bit1：pump_run
1 = 泵运行
0 = 泵停止
bit2：valve1_close
1 = 阀门1闭合
0 = 阀门1打开
bit3：valve2_close
1 = 阀门2闭合
0 = 阀门2打开

从机->主机：

标准帧4字节

Byte0：压力高字节
Byte1：压力低字节
Byte2：温度高字节
Byte3：温度低字节

发送格式：int17_t
大端
压力单位：kPa*100
温度单位：℃*100

/*****************************************************************/
上位机主机通信：串口

上位机->主机：
串口帧定义：AA 55 CMD LEN DATA... CS

开始检测： AA 55 10 00 10

暂停检测：AA 55 11 00 11

终止检测：AA 55 12 00 12

调试保留指令（仅用于调试）：AA 55 01 LEN NODE_ID CTRL CS

其中 NODE_ID 是目标从机号，CTRL是发送给从机的控制字

ctrl：
bit0：request_send
1 = 请求从机发送一次压力/温度数据
0 = 不请求发送
bit1：pump_run
1 = 泵运行
0 = 泵停止
bit2：valve1_close
1 = 阀门1闭合
0 = 阀门1打开
bit3：valve2_close
1 = 阀门2闭合
0 = 阀门2打开


主机->上位机：
AA 55 CMD LEN   原命令字  RESULT_CODE STSTUS  CS
1）命令应答帧
命令
CMD = 0x80
DATA
DATA[0] = 原命令字
DATA[1] = 结果码
DATA[2] = 主板当前状态
结果码定义
0x00：成功
0x01：失败
0x02：忙
0x03：非法命令
0x04：非法状态
主板状态定义
0x00：空闲 IDLE
0x01：检测中 RUNNING
0x02：暂停 PAUSED
0x03：终止 STOPPED
0x04：完成 FINISHED
0x05：异常 ERROR

上位机发开始检测后，主板回复：
AA 55 80 03 10 00 01 94
解释：
原命令 0x10
结果成功 0x00
当前状态 0x01 = RUNNING

2）检测结果帧
AA 55 CMD LEN DATA[18]  RESULT_CODE  CS
命令
CMD = 0x90
LEN
0x12
DATA[0]  = result_code
DATA[1]  = valid_mask

DATA[2]  = slave1_pressure_H
DATA[3]  = slave1_pressure_L
DATA[4]  = slave1_temp_H
DATA[5]  = slave1_temp_L

DATA[6]  = slave2_pressure_H
DATA[7]  = slave2_pressure_L
DATA[8]  = slave2_temp_H
DATA[9]  = slave2_temp_L

DATA[10] = slave3_pressure_H
DATA[11] = slave3_pressure_L
DATA[12] = slave3_temp_H
DATA[13] = slave3_temp_L

DATA[14] = slave4_pressure_H
DATA[15] = slave4_pressure_L
DATA[16] = slave4_temp_H
DATA[17] = slave4_temp_L

result_code 定义
0x00：全部完成
0x01：部分成功
0x02：全部失败
0x03：检测被终止
0x04：检测超时 
