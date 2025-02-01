//////////////////////////////////////////////////////////////////////
// ADC Interrupt (without DMA)  using SCAN mode
// ADC1:CH1 (PA1, pin 41) : ���������������
// ADC1:CH0 (PA0, pin 40) : �Ÿ������������
// ��ȯ����: SW start
// ��ȯ�Ϸᶧ EOC ���ͷ�Ʈ �߻��Ͽ� �Ƴ��α� ���а��� �����а��� LCD�� ǥ��
//////////////////////////////////////////////////////////////////////
#include "stm32f4xx.h"
#include "GLCD.h"

#define SW0_PUSH        0xFE00  //PH8
#define SW1_PUSH        0xFD00  //PH9
#define SW2_PUSH        0xFB00  //PH10
#define SW3_PUSH        0xF700  //PH11
#define SW4_PUSH        0xEF00  //PH12
#define SW5_PUSH        0xDF00  //PH13
#define SW6_PUSH        0xBF00  //PH14
#define SW7_PUSH        0x7F00  //PH15

void _GPIO_Init(void);
void DisplayTitle(void);

void _ADC_Init(void);
uint16_t KEY_Scan(void);

void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);
void BEEP(void);

uint16_t ADC_Value, Voltage;
char str[20];
uint8_t count;

void ADC_IRQHandler(void)
{
	ADC1->SR &= ~(1<<1);		// EOC flag clear

	ADC_Value = ADC1->DR;		// Reading ADC result
	
	count++;
	if (count==2) count= 0;
 	if (count==1) {			// ADC1-CH1 (�������װ�)
		sprintf(str,"%4d",ADC_Value);	// ADC result : 12bits (0~4095)
		LCD_DisplayText(0,6,str);
		Voltage = (uint16_t )(ADC_Value * (3.3 * 100) / 4095);   // 3.3 : 4095 =  Volatge : ADC_Value 
                                                    // 100:  �Ҽ����Ʒ� ���ڸ����� ǥ���ϱ� ���� ��  
		LCD_DisplayChar(1,9,Voltage/100 + 0x30);
		LCD_DisplayChar(1,11,Voltage%100/10 + 0x30);
		LCD_DisplayChar(1,12,Voltage%10 + 0x30);
	}
	else 					// ADC1-CH0 (�Ÿ�������)
	{
		sprintf(str,"%4d",ADC_Value);	// ADC result : 12bits (0~4095)
		LCD_DisplayText(3,6,str);
		Voltage = (uint16_t )(ADC_Value * (3.3 * 100) / 4095);   // 3.3 : 4095 =  Volatge : ADC_Value 
                                                    // 100:  �Ҽ����Ʒ� ���ڸ����� ǥ���ϱ� ���� ��  
		LCD_DisplayChar(4,9,Voltage/100 + 0x30);
		LCD_DisplayChar(4,11,Voltage%100/10 + 0x30);
		LCD_DisplayChar(4,12,Voltage%10 + 0x30);

		ADC1->CR2 |= ADC_CR2_SWSTART; 	// 0x40000000 (1<<30)
	}
}

int main(void)
{
	LCD_Init();	// LCD ���� �Լ�
	DelayMS(10);	// LCD���� ������
 	DisplayTitle();	//LCD �ʱ�ȭ�鱸�� �Լ�
        
	_GPIO_Init();
	_ADC_Init();
       
 	//Starts conversion of regular channels
       ADC1->CR2 |= ADC_CR2_SWSTART; // 0x40000000 (1<<30) 
	while(1)
	{
           	GPIOG->ODR ^= 0x01;
		DelayMS(300);
 	}
}

void _ADC_Init(void)
{
	/* 1st Analog signal */
	RCC->AHB1ENR |= 0x01;  	// RCC_AHB1ENR GPIOA Enable
	GPIOA->MODER |= 0x0C;	// GPIOA PIN1(PA1) �������� : Analog mode
  
	/* 2nd Analog signal */
	GPIOA->MODER |= 0x03;	// GPIOA PIN0(PA0) �Ÿ�����: Analog mode

	/* ADC Common Init **********************************************************/
	RCC->APB2ENR |= 0x0100;	// RCC_APB2ENR ADC1 Enable
  
	ADC->CCR &= ~0X0000001F;// ADC_Mode_Independent
	ADC->CCR |= 0x00010000;	// ADC_Prescaler_Div4 (ADC MAX Clock 36Mhz, 84Mhz(APB2)/4 = 21Mhz
	ADC->CCR |= 0x00000F00;	// ADC_TwoSamplingDelay_20Cycles

	/* ADC1 Init ****************************************************************/
	ADC1->CR1 &= ~(3<<24);	// RES[1:0]=0b00 : 12bit Resolution
	ADC1->CR1 |= 0x00000100;	// ADC_ScanCovMode Enable (SCAN=1)
	ADC1->CR2 &= ~0x00000002;	// ADC_ContinuousConvMode Disable (CONT=0)
	ADC1->CR2 &= ~(3<<28);	// EXTEN[1:0]=0b00: ADC_ExternalTrigConvEdge_None (�ܺ�Ʈ���� ������)

	ADC1->CR1 |=  (1<<5);		// EOCIE=1: Interrupt enable for EOC
  	NVIC->ISER[0] |= (1<<18);	// Enable ADC global Interrupt  

	ADC1->CR2 &= ~(1<<11);	// ALIGN=0: ADC_DataAlign_Right

//	ADC1->CR2 &= ~(1<<10);	// EOCS=0: The EOC bit is set at the end of each sequence of regular conversions
	ADC1->CR2 |= (1<<10);	// EOCS=1: The EOC bit is set at the end of each regular conversion

	ADC1->SQR1 |= (1<<20); //0x00100000; // ADC Regular channel sequece length = 2 conversion  0x01 --> ä�� 2�� ���

	/* ADC_RegularChannelConfig *********************************************/
	ADC1->SMPR2 |= 0x07 << (3*1);	// ADC1_CH1 Sample TIme_480Cycles (3*Channel_1)
	ADC1->SQR3 |= 0x01 << (5*(1-1));	// ADC1_CH1 << (5 * (Rank - 1)),  Rank = 1 (1������ ��ȯ: ��������)
    
	ADC1->SMPR2 |= 0x07 << (3*0);	//ADC1_CH0 Sample Time_480Cycles (3*Channel_0)
	ADC1->SQR3 &= ~(0x1F << (5*(2-1)));//ADC1_CH0 << (5*(Rank-1)), Rank = 2 (2������ ��ȯ: �Ÿ�����)

        
        
	/* Enable DMA request after last transfer (Single-ADC mode) */
	ADC1->CR2 |= 0x00000001;	// Enable ADC1:  ADON=1

}


void _GPIO_Init(void)
{
	// LED (GPIO G) ����
    	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
 	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
    
	// SW (GPIO H) ���� 
	RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	// Buzzer (GPIO F) ���� 
    	RCC->AHB1ENR	|=  0x00000020; // RCC_AHB1ENR : GPIOF(bit#5) Enable							
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
	GPIOF->OTYPER 	&= ~0x0200;	// GPIOF 9 : Push-pull  	
 	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 
	
	//NAVI.SW(PORT I) ����
	RCC->AHB1ENR 	|= 0x00000100;	// RCC_AHB1ENR GPIOI Enable
	GPIOI->MODER 	= 0x00000000;	// GPIOI PIN8~PIN15 Input mode (reset state)
	GPIOI->PUPDR    = 0x00000000;	// GPIOI PIN8~PIN15 Floating input (No Pull-up, pull-down) (reset state)
}	

void BEEP(void)			// Beep for 20 ms 
{ 	GPIOF->ODR |= (1<<9);	// PF9 'H' Buzzer on
	DelayMS(20);		// Delay 20 ms
	GPIOF->ODR &= ~(1<<9);	// PF9 'L' Buzzer off
}

void DelayMS(unsigned short wMS)
{
	register unsigned short i;
	for (i=0; i<wMS; i++)
		DelayUS(1000);   // 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
	volatile int Dly = (int)wUS*17;
		for(; Dly; Dly--);
}

void DisplayTitle(void)
{
	LCD_Clear(RGB_WHITE);
	LCD_SetFont(&Gulim10);		//��Ʈ 
	LCD_SetBackColor(RGB_GREEN);	//���ڹ���
	LCD_SetTextColor(RGB_BLACK);	//���ڻ�
       	LCD_DisplayText(0,0,"ADC1: 0000");
      	LCD_DisplayText(1,0,"Voltage: 0.00V");
       	LCD_DisplayText(3,0,"ADC2: 0000");
      	LCD_DisplayText(4,0,"Voltage: 0.00V");

	LCD_SetBackColor(RGB_YELLOW);	//���ڹ���
}

uint8_t key_flag = 0;
uint16_t KEY_Scan(void)	// input key SW0 - SW7 
{ 
	uint16_t key;
	key = GPIOH->IDR & 0xFF00;	// any key pressed ?
	if(key == 0xFF00)		// if no key, check key off
	{  	if(key_flag == 0)
        		return key;
      		else
		{	DelayMS(10);
        		key_flag = 0;
        		return key;
        	}
    	}
  	else				// if key input, check continuous key
	{	if(key_flag != 0)	// if continuous key, treat as no key input
        		return 0xFF00;
      		else			// if new key,delay for debounce
		{	key_flag = 1;
			DelayMS(10);
 			return key;
        	}
	}
}
