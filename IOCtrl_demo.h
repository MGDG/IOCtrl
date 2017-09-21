/**
  ******************************************************************************
  * @file    IOCtrl_demo.h
  * @author  mgdg
  * @version V1.0.0
  * @date    2017-09-20
  * @brief   
  ******************************************************************************
 **/

#ifndef __IOCTRL_DEMO_H
#define __IOCTRL_DEMO_H

#include "IOCtrl.h"
#include "stm32f1xx_hal.h"

/**
  * @brief	LED控制初始化
  * @param	void
  *
  * @return	void
  * @remark	申请IO控制资源，申请成功后启动 （用户等级为2，灭500ms 亮500ms，持续时间0（无限循环））
  */
void LedInit(void);

/**
  * @brief	LED控制启动
  * @param	USER：用户等级
  * @param	offtime：灭offtime(ms)
  * @param	ontime：亮ontime(ms)
  * @param	ctltime：持续时间ctltime (ms)（0为无限循环）
  *
  * @return	void
  * @remark	当前有更高等级的用户在使用的话则启动失败，IO控制资源未申请也会启动失败
  */
void ledstart(IOUser_Typedef USER,uint32_t offtime,uint32_t ontime,uint32_t ctltime);

/**
  * @brief	LED控制停止
  * @param	USER：用户等级
  *
  * @return	void
  * @remark	停止对IO的控制，当前有更高等级的用户在使用的话则停止失败
  */
void ledstop(IOUser_Typedef USER);

/**
  * @brief	LED控制删除
  * @param	void
  *
  * @return	void
  * @remark	删除IO控制资源，停止对IO的控制
  */
void ledkill(void);

/**
  * @brief	命令行调试回调函数，需配合CLI使用
  * @param	paranum：命令参数个数
  * @param	*para[]：命令参数字符串地址
  *
  * @return	void
  * @remark	测试IO的基本功能 （eg: (led start 4 500 500 0) 该命令将以用户4的身份启动LED控制，高电平500ms，低电平500ms，无限循环）
  */
void CMDHandle_ioctrltest(uint8_t paranum,const char *para[]);

#endif
