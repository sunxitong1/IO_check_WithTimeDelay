typedef struct
{
//		uint16_t	uiInputValue;		//输入信号
	uint8_t		uiTargetLogic;		//目标电平
	// 延时时间=uiOutDelay*uiInerDelay*2*指令执行周期
	uint16_t	uiOutDelay;			//外循环延时周期
	uint16_t	uiInerDelay;		//内循环延时周期
	uint8_t		uiErrorFlag;
		
}IOLogicDelayCheckTag;

typedef struct
{
	uint16_t	uiAboveLowSet;		//(大于)||(小于)设置
	uint16_t	uiTargetValue;		//目标电压
	// 延时时间=uiOutDelay*uiInerDelay*2*指令执行周期
	uint16_t	uiOutDelay;			//外循环延时周期
	uint16_t	uiInerDelay;		//内循环延时周期
	uint8_t		uiErrorFlag;
}IOADDelayCheckTag;

SysFSMCtrlTag stSysFSMCtrl;
DcDcCtrlParaTag stDcDcCtrlPara;

void IOCheckInit_WithTimeDelay(void)
{
	gOCP_DRI_IOCheck.uiErrorFlag = 0;
	gOCP_DRI_IOCheck.uiInerDelay= 266;
	gOCP_DRI_IOCheck.uiOutDelay = 250;
	gOCP_DRI_IOCheck.uiTargetLogic = 0;

	gSSMCU_IOCheck.uiAboveLowSet = 1;
	gSSMCU_IOCheck.uiErrorFlag = 0;
	gSSMCU_IOCheck.uiInerDelay =250;
	gSSMCU_IOCheck.uiOutDelay = 1360;
}
//########################################################################################
// Function Name:   OCPCheck_With_TimeDelay
// Input:          延时时间(uiOutDelay*uiInerDelay)
// Output:  		uiErrorFlag 
// Return:			无
// Description:    	检测OCP端口电平是否上升或者下降为目标电平，
//					在延时时间内未到达目标电平则报错
//########################################################################################

void OCPCheck_With_TimeDelay(IOLogicDelayCheckTag* me)
{
	uint16_t nOuterTimedelay = me->uiOutDelay;
	uint16_t nInerDelay = me->uiInerDelay;
	
	while((nOuterTimedelay)&&(me->uiTargetLogic != OCP_STA()))//外延时
	{
		nOuterTimedelay--;
		//内延时
		nInerDelay = me->uiInerDelay;
		while(nInerDelay)
    	{
    		nInerDelay--;
    		if(me->uiTargetLogic == OCP_STA())
			break;
        	__asm nop;
        	__asm nop;
    	}
		if(me->uiTargetLogic == OCP_STA())
			break;
	}
	
	if(nOuterTimedelay == 0)
		me->uiErrorFlag = 1;
}

//########################################################################################
// Function Name:   SSMCUCheck_With_TimeDelay
// Input:          (1)延时时间(uiOutDelay*uiInerDelay)
//					(2)目标电压(uiTargetValue)
//					(3)上升或者下降设置(uiAboveLowSet)
// Output:  		uiErrorFlag 
// Return:			无
// Description:    	检测SS_MCU端口电平是否上升或者下降为目标电平，
//					在延时时间内未到达目标电平则报错
//########################################################################################
void SSMCUCheck_With_TimeDelay(IOADDelayCheckTag* me)
{
	uint16_t nOuterTimedelay = me->uiOutDelay;
	uint16_t nInerDelay = me->uiInerDelay;
	
	SSVoltageUpdate();
	while((nOuterTimedelay)&&(((stAnalogInput.uiSSVoltage <= me->uiTargetValue)&&(me->uiAboveLowSet))||((stAnalogInput.uiSSVoltage >= me->uiTargetValue)&&(!me->uiAboveLowSet))))//外延时
	{
		nOuterTimedelay--;
		//内延时
		nInerDelay = me->uiInerDelay;
		while(nInerDelay)
    	{
    		nInerDelay--;
    		SSVoltageUpdate();
    		if(((stAnalogInput.uiSSVoltage >= me->uiTargetValue)&&(me->uiAboveLowSet))||((stAnalogInput.uiSSVoltage <= me->uiTargetValue)&&(!me->uiAboveLowSet)))
			break;
        	__asm nop;
        	__asm nop;
    	}
		
		if(((stAnalogInput.uiSSVoltage >= me->uiTargetValue)&&(me->uiAboveLowSet))||((stAnalogInput.uiSSVoltage <= me->uiTargetValue)&&(!me->uiAboveLowSet)))
			break;
	}
	
	if(nOuterTimedelay == 0)
		me->uiErrorFlag = 1;
}

//////example/////////example/////////////example/////////////////
.
.
.
	gSSMCU_IOCheck.uiAboveLowSet = 0;
	gSSMCU_IOCheck.uiTargetValue = SS_VOLTAGE_STEP2;
	SSMCUCheck_With_TimeDelay(&gSSMCU_IOCheck);
	unFaultCode.bit.bSSChkFail2 = gSSMCU_IOCheck.uiErrorFlag;
.
.
.
