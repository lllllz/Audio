/*******************************************************
* File:			uart.h
* Date:			2016-10-24
* Author:		lllllz
* Description:	uart串口通信通头文件
*******************************************************/


#ifndef __UART_H__
#define __UART_H__

#ifdef __cplusplus
extern "C" {
#endif /* C++ */


/* ------------------------------------------------------------------------
** Types
** ------------------------------------------------------------------------ */
typedef void ( * uart_rec_fn)(const void* data, unsigned int datalen, void* user_data);
typedef void* UART_HANDLE;


/* ------------------------------------------------------------------------
** Function
** ------------------------------------------------------------------------ */
/*******************************************************
* 函数名:			uartInit
* 函数说明:		串口初始化,创建消息接收线程
* 参数:
*	uart_hd 	串口文件描述符
*	device      串口速度
*	rate  		串口速度
*	uart_rec_cb	消息接收函数地址(用于创建线程)
*	user_data	用户数据(附属参数)
* 返回值:			成功返回为1,失败返回为0
*******************************************************/
int uartInit(UART_HANDLE* uart_hd, const char* device, int rate, uart_rec_fn uart_rec_cb, void* user_data);

/*******************************************************
* 函数名:			uartSend
* 函数说明:		串口消息发送
* 参数:
*	uart_hd 	串口文件描述符
*	msg      	要发送的消息内容
*	msglen  	要发送的消息长度
* 返回值:			成功返回为0,失败返回为-1
*******************************************************/
int uartSend(UART_HANDLE uart_hd, const unsigned char* msg, int msglen);

/*******************************************************
* 函数名:			uartUninit
* 函数说明:		串口设备关闭
* 参数:
*	uart_hd 	串口文件描述符
* 返回值:			无
*******************************************************/
void uartUninit(UART_HANDLE uart_hd);


#ifdef __cplusplus
} /* extern "C" */	
#endif /* C++ */

#endif /* __UART_H__ */