/**
  ******************************************************************************
  * @file    IOCtrl_Demo.c
  * @author  mgdg
  * @version V1.0.0
  * @date    2017-09-20
  * @brief   
  ******************************************************************************
 **/


#include "IOCtrl.h"


static MyIOHandle LedIOIndex = NULL;


/**
  * @brief	IO口控制函数
  * @param	state:IO电平
  *
  * @return	void
  * @remark	底层函数，IO口必须初始化过
  */
static void GPIO_SetLED(bool state)
{
	//控制led亮灭的函数
}


//初始化一个LED
void LedInit(void)
{
	if(IOCtrl_Create(&LedIOIndex,GPIO_SetLED))
	{
		if(IOCtrl_Start(LedIOIndex,IOUser_Level2,950,50,0))				//灭950ms 亮50ms
		{
			DEBUGOUT("Led start , (950,50)");
		}
		else
		{
			DEBUGOUT("Led start error");
		}
	}
	else
	{
		DEBUGOUT("Led create error");
	}
}


void ledstart(IOUser_Typedef USER,uint32_t offtime,uint32_t ontime,uint32_t ctltime)
{
	if(LedIOIndex != NULL)
	{
		if(IOCtrl_Start(LedIOIndex,USER,offtime,ontime,ctltime))				//灭100ms 亮500ms
		{
			DEBUGOUT("Led control start , (%u,%u,%u)",offtime,ontime,ctltime);
		}
		else
		{
			DEBUGOUT("Led control start error");
		}
	}
	else
	{
		DEBUGOUT("Led control not create");
	}
}


void ledstop(IOUser_Typedef USER)
{
	if(LedIOIndex != NULL)
	{
		if(IOCtrl_Stop(LedIOIndex,USER))
		{
			DEBUGOUT("Led control stoped");
		}
		else
		{
			DEBUGOUT("Led control stop error");
		}
	}
	else
	{
		DEBUGOUT("Led control stop error, not create");
	}
}


void ledkill(void)
{
	if(LedIOIndex != NULL)
	{
		if(IOCtrl_Kill(&LedIOIndex))
		{
			DEBUGOUT("Led control killed");
		}
		else
		{
			DEBUGOUT("Led control kill error");
		}
	}
	else
	{
		DEBUGOUT("Led control kill error, not create");
	}
}