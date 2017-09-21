/**
  ******************************************************************************
  * @file    IOCtrl.c
  * @author  mgdg
  * @version V1.0.0
  * @date    2017-09-20
  * @brief   
  ******************************************************************************
 **/

#include "IOCtrl.h"

//static const uint32_t IOMINCTRLTIME = 10;					//最小控制间隔，该间隔由定时器调用间隔确定
//包含所有IO序号的链表
typedef struct AllIONode
{
	uint32_t 			IOIndex;			//IO口序号
	struct AllIONode*	next_io;			//下一个节点指针
}ALL_IO_LIST;
static ALL_IO_LIST *AllIOList = NULL;


//包含已经激活的IO序号的链表
typedef struct ActivIONode
{
	uint32_t 			OffTime;			// 间隔时间（占空比低）
	uint32_t 			OnTime;				// 持续时间（占空比高）
	uint32_t 			CtlTime;			// 控制总时间
	uint32_t 			CurCount;			// 单周期计数 
	uint32_t 			SumCount;			// 已运行的总时间
	IOCtrlCallback 		handle;				// IO电平控制回调函数指针
	bool  				CtlFlag;			// 控制变化标志
	IOUser_Typedef 		CurIOUser;			// IO当前的用户
	struct ActivIONode *next_io;			// 下一个节点指针
}ACTIV_IO_LIST;
static ACTIV_IO_LIST *ActivIOList = NULL;


/**
  * @brief	申请完IO控制后放入列表
  * @param	**ListHead：指向链表头指针的指针
  * @param	index：IO序号
  *
  * @return	bool
  * @remark	链表头为空的话则创建新链表并更新链表头指针
  */
static bool AllIOList_Put(ALL_IO_LIST **ListHead,const uint32_t index)
{
	ALL_IO_LIST* p = NULL;
 	ALL_IO_LIST* q = NULL;
	
	if(index == NULL)				//加入的是空的IO序号
		return false;

	if(*ListHead==NULL)				//链表为空，创建新链表
	{
		*ListHead = (ALL_IO_LIST *)malloc(sizeof(ALL_IO_LIST));
		if(*ListHead != NULL)
		{
			(*ListHead)->IOIndex = index;
			(*ListHead)->next_io = NULL;
			return true;
		}
		return false;
	}

	q = *ListHead;
 	p = (*ListHead)->next_io;
 	while(p != NULL)
 	{
		if(q->IOIndex == index)				//链表中已经存在
			return true;
 		q = p;
 		p = p->next_io;
 	}
	
	//链表中不存在，创建一个新节点
	p = (ALL_IO_LIST *)malloc(sizeof(ALL_IO_LIST));
	if(p != NULL)
	{
		//初始化节点数据
		p->IOIndex = index;
		p->next_io = NULL;
		//加入链表
		q->next_io = p;			
		return true;
	}
	return false;	
}

/**
  * @brief	IO控制删除后从列表中删除
  * @param	**ListHead：指向链表头指针的指针
  * @param	index：IO序号
  *
  * @return	bool
  * @remark	删除后更新链表头指针
  */
static bool AllIOList_Pop(ALL_IO_LIST **ListHead,const uint32_t index)
{
	ALL_IO_LIST* p = NULL;
	ALL_IO_LIST* q = *ListHead;

	//无效链表头节点、无效定时器序号
	if(ListHead==NULL || index==NULL)
		return false;

	while( (q->IOIndex != index) && (q != NULL) )
	{
		p = q;
		q = q->next_io;
	}

	if(q->IOIndex != index)		//链表中不存在该节点
		return false;

	if(q==*ListHead)				//删除的是第一个节点
		*ListHead = (*ListHead)->next_io;	
	else
		p->next_io = q->next_io;
	
	free(q);						//释放掉被删除的节点
	return true;
}

/**
  * @brief	查询IO列表中是否存在该IO
  * @param	*ListHead：链表头指针
  * @param	index：IO序号
  *
  * @return	bool
  * @remark	
  */
static bool AllIOList_Get(ALL_IO_LIST *ListHead,const uint32_t index)
{
	if(ListHead == NULL)			//空链表
		return false;
	
	while(ListHead != NULL)
	{
		if(ListHead->IOIndex == index)
			return true;
		ListHead = ListHead->next_io;
	}
	return false;
}

/**
  * @brief	IO控制启动后添加到激活列表
  * @param	**ListHead：指向链表头指针的指针
  * @param	*NewNode：节点指针
  *
  * @return	bool
  * @remark	链表头为空的话则创建新链表并更新链表头指针
  */
static bool ActivIOList_Put(ACTIV_IO_LIST **ListHead,ACTIV_IO_LIST *NewNode)
{
	ACTIV_IO_LIST* p = NULL;
 	ACTIV_IO_LIST* q = NULL;
	
	if(NewNode == NULL)
		return false;
	
	NewNode->next_io = NULL;
	
	if(*ListHead==NULL)
	{
		*ListHead = NewNode;			//空链表，链接的节点设为头节点
		return true;
	}

	q = *ListHead;
 	p = (*ListHead)->next_io;
 	while(p != NULL)
 	{
 		q = p;
 		p = p->next_io;
 	}
	
	q->next_io = NewNode;

 	return true;
}

/**
  * @brief	IO控制停止后从激活列表移除
  * @param	**ListHead：指向链表头指针的指针
  * @param	*TempNode：节点指针
  *
  * @return	bool
  * @remark	删除后更新链表头指针
  */
static bool ActivIOList_Pop(ACTIV_IO_LIST **ListHead,ACTIV_IO_LIST *TempNode)
{
	ACTIV_IO_LIST* p = NULL;
	ACTIV_IO_LIST* q = *ListHead;

	//无效链表头节点、无效指定节点
	if(ListHead==NULL || TempNode==NULL)
		return false;

	while(q != TempNode && q != NULL)
	{
		p = q;
		q = q->next_io;
	}

	if(q != TempNode)				//链表中不存在该节点
		return false;

	if(q==*ListHead)				//删除的是第一个节点
	{
		*ListHead = (*ListHead)->next_io;	
	}
	else
	{
		p->next_io = q->next_io;
	}
	return true;
}

/**
  * @brief	查找IO控制是否已经在激活列表里
  * @param	*ListHead：链表头指针
  * @param	*TempNode：节点指针
  *
  * @return	bool
  * @remark	
  */
static bool ActivIOList_Get(ACTIV_IO_LIST *ListHead,ACTIV_IO_LIST *TempNode)
{
	//无效链表头节点、无效指定节点
	if(ListHead==NULL || TempNode==NULL)
		return false;
	
	while(ListHead != TempNode && ListHead != NULL)
	{
		ListHead = ListHead->next_io;
	}

	return (ListHead == TempNode);
}


/**
  * @brief	IO控制释放后从激活列表中删除
  * @param	**ListHead：指向链表头指针的指针
  * @param	*TempNode：节点指针
  *
  * @return	bool
  * @remark	删除后更新链表头指针
  */
static bool ActivIOList_Delete(ACTIV_IO_LIST **ListHead,ACTIV_IO_LIST *TempNode)
{
	ACTIV_IO_LIST* p = NULL;
	ACTIV_IO_LIST* q = *ListHead;
	
	if(TempNode==NULL)				//无效指定节点
		return false;
	
	if(*ListHead==NULL)				//无效链表头节点
	{
		free(TempNode);				//释放掉被删除的节点
		return true;
	}
		
	while(q != TempNode && q != NULL)
	{
		p = q;
		q = q->next_io;
	}

	if(q != TempNode)				//链表中不存在该节点
	{
		TempNode->handle(false);
		free(TempNode);				//释放掉被删除的节点
		return true;
	}

	if(q==*ListHead)				//删除的是第一个节点
	{
		(*ListHead) = (*ListHead)->next_io;				
	}
	else
	{
		p->next_io = q->next_io;
	}
	q->handle(false);
	free(q);						//释放掉被删除的节点
	return true;
}


/**
  * @brief	申请IO口控制资源
  * @param	timproc:IO回调函数
  *
  * @return	IO控制序号
  * @remark	IO序号为链表节点指针，具有唯一性
  */
uint32_t IOCtrl_Create(IOCtrlCallback ioproc)
{
	ACTIV_IO_LIST *temp_index = NULL;

	if(ioproc == NULL)
		return NULL;

	temp_index = (ACTIV_IO_LIST *)malloc(sizeof(ACTIV_IO_LIST));

	if(temp_index != NULL)
	{
		//初始化IO控制数据
		temp_index->OffTime			= 0;						// 间隔时间（占空比低）
		temp_index->OnTime			= 0;						// 持续时间（占空比高）
		temp_index->CtlTime			= 0;						// 总时间
		temp_index->CurCount		= 0;						// 单周期计数
		temp_index->SumCount		= 0;						// 总时间计数 
		temp_index->handle			= ioproc;					// 回调函数指针
		temp_index->CtlFlag			= false;					// 控制变化标志
		temp_index->CurIOUser		= IOUser_None;				// IO当前的用户
		temp_index->next_io			= NULL;						// 下一个节点指针
		
		//放入IO列表	
		if(AllIOList_Put(&AllIOList,(uint32_t)temp_index))
		{
			return (uint32_t)temp_index;
		}
		else
		{
			free(temp_index);
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}

/**
  * @brief	删除IO口控制资源
  * @param	*index:指向IO序号的指针
  *
  * @return	bool
  * @remark	
  */
bool IOCtrl_Kill(uint32_t *index)
{
	if(!AllIOList_Get(AllIOList,*index))
		return false;

	//无论链表中是否存在该节点都free掉
	if(ActivIOList_Delete(&ActivIOList,(ACTIV_IO_LIST *)(*index)))
	{
		if(AllIOList_Pop(&AllIOList,*index))
		{
			*index = 0;
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

/**
  * @brief	启动IO口控制
  * @param	index:IO控制序号
  * @param	USER:用户
  * @param	interval:间隔时间（占空比低）
  * @param	beeptime:激活时间（占空比高）
  * @param	ctltime:持续时间 （总时间）
  * 
  * @return	bool
  * @remark	
  */
bool IOCtrl_Start(uint32_t index,IOUser_Typedef USER,uint32_t offtime,uint32_t ontime,uint32_t ctltime)
{
	//检查是否已经加入激活链表
	if(ActivIOList_Get(ActivIOList,(ACTIV_IO_LIST *)index))
	{
		//当前有更高等级的用户在使用
		if( USER < (((ACTIV_IO_LIST *)index)->CurIOUser) )
		{
			return false;
		}
		else
		{
			// if((offtime%IOMINCTRLTIME) || (ontime%IOMINCTRLTIME) || (ctltime%IOMINCTRLTIME)) //非10ms倍数为无效设置
			// 	return false;
			// else
			// {
				//已经激活了，更改控制
				((ACTIV_IO_LIST *)index)->OffTime		= offtime;					// 间隔时间（占空比低）
				((ACTIV_IO_LIST *)index)->OnTime		= ontime;					// 持续时间（占空比高）
				((ACTIV_IO_LIST *)index)->CtlTime		= ctltime;					// 总时间
				((ACTIV_IO_LIST *)index)->CurCount		= 0;
				((ACTIV_IO_LIST *)index)->SumCount		= 0;						// 总时间计数 
				((ACTIV_IO_LIST *)index)->CurIOUser		= USER;						// IO当前的用户

				((ACTIV_IO_LIST *)index)->handle(ontime);								// 开始鸣叫
				((ACTIV_IO_LIST *)index)->CtlFlag		= ontime;						// 控制电平设为高

				return true;
			// }
		}
	}
	else
	{
		if(!AllIOList_Get(AllIOList,index))				//判断是否存在该IO
			return false;

		// if((offtime%IOMINCTRLTIME) || (ontime%IOMINCTRLTIME) || (ctltime%IOMINCTRLTIME)) //非10ms倍数为无效设置
		// 	return false;
		else
		{
			if(ActivIOList_Put(&ActivIOList,(ACTIV_IO_LIST *)index))
			{
				((ACTIV_IO_LIST *)index)->OffTime		= offtime;					// 间隔时间（占空比低）
				((ACTIV_IO_LIST *)index)->OnTime		= ontime;					// 持续时间（占空比高）
				((ACTIV_IO_LIST *)index)->CtlTime		= ctltime;					// 总时间
				((ACTIV_IO_LIST *)index)->CurCount		= 0;
				((ACTIV_IO_LIST *)index)->SumCount		= 0;						// 总时间计数 
				((ACTIV_IO_LIST *)index)->CurIOUser		= USER;						// IO当前的用户
				
				((ACTIV_IO_LIST *)index)->handle(ontime);								// 开始鸣叫
				((ACTIV_IO_LIST *)index)->CtlFlag		= ontime;						// 控制电平设为高
				return true;
			}
			else
			{
				return false;
			}
		}
	}
}

/**
  * @brief	停止IO口控制
  * @param	index:IO控制序号
  * @param	USER:用户
  * 
  * @return	bool
  * @remark	
  */
bool IOCtrl_Stop(uint32_t index,IOUser_Typedef USER)
{
	//检查是否已经加入激活链表
	if(ActivIOList_Get(ActivIOList,(ACTIV_IO_LIST *)index))
	{
		//当前有更高等级的用户在使用
		if( USER < (((ACTIV_IO_LIST *)index)->CurIOUser) )
		{
			return false;
		}
		else
		{
			((ACTIV_IO_LIST *)index)->OffTime		= 0;					// 间隔时间（占空比低）
			((ACTIV_IO_LIST *)index)->OnTime		= 0;					// 持续时间（占空比高）
			((ACTIV_IO_LIST *)index)->CtlTime		= 0;					// 总时间
			((ACTIV_IO_LIST *)index)->CurCount		= 0;					// 总时间计数 
			((ACTIV_IO_LIST *)index)->SumCount		= 0;					// 总时间计数 
			((ACTIV_IO_LIST *)index)->handle(false);
			((ACTIV_IO_LIST *)index)->CurIOUser		= IOUser_None;			// IO当前的用户

			return ActivIOList_Pop(&ActivIOList,(ACTIV_IO_LIST *)index);
		}
	}
	else
		return true;
}

/**
  * @brief	获取IO当前控制用户
  * @param	index:IO控制序号
  * 
  * @return	IOUser_Typedef 当前用户
  * @remark	
  */
IOUser_Typedef IOCtrl_GetUser(uint32_t index)
{
	if(!AllIOList_Get(AllIOList,index))				//判断是否存在该IO
		return IOUser_None;
	
	return ((ACTIV_IO_LIST *)index)->CurIOUser;
}

/**
  * @brief	IO控制定时器调用函数
  * @param	TimeInterVal:函数调用间隔
  * 
  * @return	bool
  * @remark	
  */
void IOCtrl_CallBackProc(const uint32_t TimeInterVal)
{
	ACTIV_IO_LIST *q = NULL;
	ACTIV_IO_LIST *p = ActivIOList;

	//扫描所有已经激活的IO
	while(p != NULL)
	{
		//无间隔叫
		if(0 == p->OffTime)
		{
			//执行回调函数
			if(!(p->CtlFlag) && (p->OnTime) )
			{
				p->handle(true);
				p->CtlFlag = true;
			}
			
			//计时
			if(p->CtlTime)				//总时间为0的话就一直叫
			{
				if( p->SumCount < p->CtlTime )
				{
					p->SumCount += TimeInterVal;
				}
				else
				{
					p->handle(false);				//禁止鸣叫
					p->CtlFlag		= false;
					p->SumCount=0;
					p->CurIOUser    = IOUser_None;	//释放权限
					
					//从激活列表中移除
					if(p == ActivIOList)
					{
						ActivIOList = ActivIOList->next_io;
					}
					else
					{
						q->next_io = p->next_io;
					}
				}
			}
		}
		//有间隔叫
		else
		{
			//持续时间
			if(p->CtlTime)
			{
				if( p->SumCount < p->CtlTime )
				{
					p->SumCount += TimeInterVal;
				}
				else
				{
					p->handle(false);//禁止鸣叫
					p->CtlFlag = false;
					p->SumCount =0;
					p->CurCount = 0;
					p->CurIOUser = IOUser_None;		//释放权限
					
					//从激活列表中移除
					if(p == ActivIOList)
					{
						ActivIOList = ActivIOList->next_io;
					}
					else
					{
						q->next_io = p->next_io;
					}
					continue;
				}
			}

			
			//周期时间内鸣叫			
			
			if(p->CtlFlag)
			{
				if(p->CurCount>=p->OnTime)
				{
					//到达持续时期末,进入间隔周期
					p->handle(false);//禁止鸣叫
					p->CtlFlag=false;
					p->CurCount = 0;
				}
			}
			//鸣叫时间
			else
			{
				if(p->CurCount>=p->OffTime)
				{
					//到达间隔周期末，进入持续叫周期
					if(p->OnTime)
					{
						p->handle(true);//开始鸣叫
						p->CtlFlag=true;
					}
					p->CurCount = 0;
				}
			}
			p->CurCount += TimeInterVal;
		}

		q = p;
		p = p->next_io;
	}
}

/**
  * @brief	获取IO控制链表的信息
  * @param	void
  * 
  * @return	void
  * @remark	
  */
void get_io_list(void)
{
	ALL_IO_LIST *list = NULL;				//定时器列表
	ACTIV_IO_LIST *actlist = NULL;			//定时器链表头指针
	uint8_t count = 1;
	
	printf("\r\nAll IO list:\r\n");
	list = AllIOList;
	while(list != NULL)
	{
		printf("IO(%d):\t%08X",count++,list->IOIndex);
		
		if(ActivIOList_Get(ActivIOList,(ACTIV_IO_LIST *)(list->IOIndex)))
		{
			printf("\tactivated\r\n");
		}
		else
			printf("\r\n");
		
		list = list->next_io;
	}
	
	count = 1;
	printf("\r\nActivated IO list:\r\n");
	actlist = ActivIOList;
	while(actlist != NULL)
	{
		printf("IO(%d):\t%08X\r\n",count++,(uint32_t)actlist);
		actlist = actlist->next_io;
	}
}
