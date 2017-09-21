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
#include "stm32f1xx_hal.h"


typedef void (*IOCtrlCallback)(bool);

//IO输出控制用户定义，值越高等级越高
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
  * @param	ioproc:IO回调函数
  *
  * @return	IO控制序号
  * @remark	IO序号为链表节点指针，具有唯一性
  */
uint32_t IOCtrl_Create(IOCtrlCallback ioproc);

/**
  * @brief	删除IO口控制资源
  * @param	*index:指向IO序号的指针
  *
  * @return	bool
  * @remark	删除成功后IO口控制序号会被清0
  */
bool IOCtrl_Kill(uint32_t *index);

/**
  * @brief	启动IO口控制
  * @param	index:IO控制序号
  * @param	USER:用户
  * @param	offtime:间隔时间（占空比低）
  * @param	ontime:激活时间（占空比高）
  * @param	ctltime:持续时间 （总时间）
  * 
  * @return	bool
  * @remark	
  */
bool IOCtrl_Start(uint32_t index,IOUser_Typedef USER,uint32_t offtime,uint32_t ontime,uint32_t ctltime);

/**
  * @brief	停止IO口控制
  * @param	index:IO控制序号
  * @param	USER:用户
  * 
* @return	bool:是否停止成功
  * @remark	
  */
bool IOCtrl_Stop(uint32_t index,IOUser_Typedef USER);

/**
  * @brief	获取IO当前控制用户
  * @param	index:IO控制序号
  * 
  * @return	IOUser_Typedef 当前用户
  * @remark	
  */
IOUser_Typedef IOCtrl_GetUser(uint32_t index);

/**
  * @brief	IO控制周期循环调用函数
  * @param	TimeInterVal:函数被调用间隔
  * 
  * @return	bool
  * @remark	
  */
void IOCtrl_CallBackProc(const uint32_t TimeInterVal);

/**
  * @brief	获取IO控制链表的信息
  * @param	void
  * 
  * @return	void
  * @remark	
  */
void get_io_list(void);


#endif
