/**
  ******************************************************************************
  * @file    IOCtrl.h
  * @author  mgdg
  * @version V1.0.0
  * @date    2017-09-20
  * @brief   
  ******************************************************************************
 **/


#ifndef __IOCTRL_H
#define __IOCTRL_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

/**
  * @brief	io句柄
  * @remark	
  */
typedef void *MyIOHandle;

/**
  * @brief	IO状态控制函数
  * @remark	
  */
typedef void (*IOCtrlFunc)(bool);

/**
  * @brief	IO控制用户定义，值越高等级越高
  * @remark	
  */
typedef enum 
{
	IOUser_None = 0,
	IOUser_Level1,
	IOUser_Level2,
	IOUser_Level3,
	IOUser_Level4,
	IOUser_Level5,
}IOUser_Typedef;


/**
  * @brief	申请IO口控制资源
  * @param	*iohandle:IO句柄
  * @param	ioproc:IO回调函数
  *
  * @return	bool
  * @remark	
  */
bool IOCtrl_Create(MyIOHandle *iohandle,IOCtrlFunc ioproc);

/**
  * @brief	删除IO口控制资源
  * @param	*iohandle:IO句柄
  *
  * @return	bool
  * @remark	删除成功后IO口控制序号会被清0
  */
bool IOCtrl_Kill(MyIOHandle *iohandle);

/**
  * @brief	启动IO口控制
  * @param	iohandle:IO句柄
  * @param	USER:用户
  * @param	offtime:间隔时间（占空比低）
  * @param	ontime:激活时间（占空比高）
  * @param	ctltime:持续时间 （总时间）
  * 
  * @return	bool
  * @remark	
  */
bool IOCtrl_Start(MyIOHandle iohandle,IOUser_Typedef USER,size_t offtime,size_t ontime,size_t ctltime);

/**
  * @brief	停止IO口控制
  * @param	iohandle:IO句柄
  * @param	USER:用户
  * 
* @return	bool:是否停止成功
  * @remark	
  */
bool IOCtrl_Stop(MyIOHandle iohandle,IOUser_Typedef USER);

/**
  * @brief	获取IO当前控制用户
  * @param	iohandle:IO句柄
  * 
  * @return	IOUser_Typedef 当前用户
  * @remark	
  */
IOUser_Typedef IOCtrl_GetUser(MyIOHandle iohandle);

/**
  * @brief	IO控制周期循环调用函数
  * @param	TimeInterVal:函数被调用间隔
  * 
  * @return	bool
  * @remark	
  */
void IOCtrl_CallBackProc(size_t TimeInterVal);

/**
  * @brief	获取IO控制链表的信息
  * @param	void
  * 
  * @return	void
  * @remark	
  */
void get_io_list(void);


#endif
