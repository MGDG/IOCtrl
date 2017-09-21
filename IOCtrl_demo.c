/**
  ******************************************************************************
  * @file    IOCtrl_demo.h
  * @author  mgdg
  * @version V1.0.0
  * @date    2017-09-20
  * @brief   
  ******************************************************************************
 **/

#include "IOCtrl_demo.h"


//获取链表详细信息

uint32_t LedIOIndex = NULL;

/**
  * @brief	IO口控制函数
  * @param	state:IO电平
  *
  * @return	void
  * @remark	底层函数，IO口必须初始化过
  */
static void GPIO_SetLED(bool state)
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,(state ? GPIO_PIN_SET:GPIO_PIN_RESET));
}

/**
  * @brief	LED控制初始化
  * @param	void
  *
  * @return	void
  * @remark	申请IO控制资源，申请成功后启动 （用户等级为2，灭500ms 亮500ms，持续时间0（无限循环））
  */
void LedInit(void)
{
	if(LedIOIndex)
	{
		printf("Led control creat error, already created");
		return;
	}
	LedIOIndex = IOCtrl_Create(GPIO_SetLED);

	if(LedIOIndex != NULL)
	{
		if(IOCtrl_Start(LedIOIndex,IOUser_Level2,500,500,0))				//,设置用户等级为2，灭500ms 亮500ms，持续时间0（无限循环）
		{
			printf("Led control start , (500,500)");
		}
		else
		{
			printf("Led control start error");
		}
	}
	else
	{
		printf("Led control create error");
	}
}

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
void ledstart(IOUser_Typedef USER,uint32_t offtime,uint32_t ontime,uint32_t ctltime)
{
	if(LedIOIndex != NULL)
	{
		if(IOCtrl_Start(LedIOIndex,USER,offtime,ontime,ctltime))				//灭100ms 亮500ms
		{
			printf("Led control start , (%u,%u,%u)",offtime,ontime,ctltime);
		}
		else
		{
			printf("Led control start error");
		}
	}
	else
	{
		printf("Led control create error");
	}
}

/**
  * @brief	LED控制停止
  * @param	USER：用户等级
  *
  * @return	void
  * @remark	停止对IO的控制，当前有更高等级的用户在使用的话则停止失败
  */
void ledstop(IOUser_Typedef USER)
{
	if(LedIOIndex != NULL)
	{
		if(IOCtrl_Stop(LedIOIndex,USER))
		{
			printf("Led control stoped");
		}
		else
		{
			printf("Led control stop error");
		}
	}
	else
	{
		printf("Led control stop error, not create");
	}
}

/**
  * @brief	LED控制删除
  * @param	void
  *
  * @return	void
  * @remark	删除IO控制资源，停止对IO的控制
  */
void ledkill(void)
{
	if(LedIOIndex != NULL)
	{
		if(IOCtrl_Kill(&LedIOIndex))
		{
			printf("Led control killed");
		}
		else
		{
			printf("Led control kill error");
		}
	}
	else
	{
		printf("Led control kill error, not create");
	}
}


/**
  * @brief	命令行调试回调函数，需配合CLI使用
  * @param	paranum：命令参数个数
  * @param	*para[]：命令参数字符串地址
  *
  * @return	void
  * @remark	测试IO的基本功能 （eg: (led start 4 500 500 0) 该命令将以用户4的身份启动LED控制，高电平500ms，低电平500ms，无限循环）
  */
void CMDHandle_ioctrltest(uint8_t paranum,const char *para[])
{
	IOUser_Typedef user;
	uint32_t interval,beeptime,ctltime;
	
	if(paranum < 2)
		printf("Para error");
	
	if(strcmp("start",para[1]) == 0)
	{
		if(paranum != 6)
		{
			printf("Para error");
			return;
		}
		
		user = (IOUser_Typedef)atoi(para[2]);
		interval = (uint32_t)atoi(para[3]);
		beeptime = (uint32_t)atoi(para[4]);
		ctltime = (uint32_t)atoi(para[5]);
		
		ledstart(user,interval,beeptime,ctltime);
	}
	else if(strcmp("stop",para[1]) == 0)
	{
		if(paranum != 3)
		{
			printf("Para error");
			return;
		}
		
		user = (IOUser_Typedef)atoi(para[2]);
		
		ledstop(user);
	}
	else if(strcmp("kill",para[1]) == 0)
	{
		if(paranum != 2)
		{
			printf("Para error");
			return;
		}
		
		ledkill();
	}
	else if(strcmp("init",para[1]) == 0)
	{
		if(paranum != 2)
		{
			printf("Para error");
			return;
		}
		
		LedInit();
	}
	else if(strcmp("info",para[1]) == 0)
	{
		if(paranum != 2)
		{
			printf("Para error");
			return;
		}
		
		get_io_list();
	}
	
	else if(strcmp("user",para[1]) == 0)
	{
		if(paranum != 2)
		{
			printf("Para error");
			return;
		}
		
		printf("IO cur user : %u",IOCtrl_GetUser(LedIOIndex));
	}
	else
	{
		printf("Para error");
	}
}
