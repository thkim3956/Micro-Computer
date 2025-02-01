//////////////////////////////////////////////////////////////////////
// IWDT(Independent Watch Dog Timer)
// EXTI8(PH8,SW0): ������ ���ѷ��� �����ϴ� ����
// Timer3: 500ms ���ͷ�Ʈ �߻�(LED0�� toggle ���� �����۵����� ǥ��)
// IWDG timer reload counter ��: 0x7D0(2sec) 
//////////////////////////////////////////////////////////////////////
#include "stm32f4xx.h"
#include "GLCD.h"

void GPIO_Init_Test(void);
void EXTI_Init_Test(void);
void DisplayTitle(void);
void TIMER3_Init_Test(void);

void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

uint8_t	SW0_Flag, SW1_Flag;

int main(void)
{
	RCC->CSR |= 0x01;	//LSI ENABLE (32kHz)
	
	GPIO_Init_Test();
	EXTI_Init_Test();
	TIMER3_Init_Test();
	LCD_Init();		// LCD ���� �Լ�
	DelayMS(1000);		// LCD���� ������
	GPIOG->ODR &= 0x00;	// LED0~7 Off 
       
	//LCD �ʱ�ȭ�鱸�� �Լ�
	DisplayTitle();
	IWDG->KR = 0x5555;	//IWDG_Resistor Write Enable (Access ���)
	//IWDG->PR = 0x03;	//IWDG_Prescaler_32 --> 1kHz --> 1ms
        IWDG->PR = 0x04;	//IWDG_Prescaler_64 --> 0.5kHz --> 2ms
	//IWDG->RLR= 0x7D0;	//IWDG_Reload Counter Value(2sec)
        IWDG->RLR= 2500;	//IWDG_Reload Counter Value(5sec)
	IWDG->KR = 0xCCCC;	//IWDG_Enable (IWDG ����)
	
	if((RCC->CSR & 0x20000000) != RESET)// IWDG_Reset Flag
	{
		GPIOG->ODR |= 0x02;	// LED1 On
		RCC->CSR |= 0x01000000; // Reset Flag Clear
	}
        else if((RCC->CSR & 0x4000000) != RESET) // Ext sw(Reset sw)_Reset Flag
        {
          GPIOG->ODR |= 0x04; //LED2 on
          RCC->CSR |= 0x01000000; //Reset Flag Clear
        }        
	else
	{	
		GPIOG->ODR &= ~0x02;	// LED1 Off				
                GPIOG->ODR &= ~0x04;	// LED2 Off				
	}
    
	while(1)
	{
		IWDG->KR = 0xAAAA;	//IWDG_ReloadCounter ReFresh 
					// ���� �ð����� KR�� 0xAAAA�� loading �ؾ� �ý��� reset�� �߻� ����
	}
}

void GPIO_Init_Test(void)
{
	// LED GPIO ����
	RCC->AHB1ENR	|= 0x40;  	// RCC_AHB1ENR GPIOG Enable							
	GPIOG->MODER 	|= 0x5555;	// GPIOG PIN0~PIN7 Output mode						
	GPIOG->OSPEEDR 	|= 0x5555;	// GPIOG PIN0~PIN7 Output speed (25MHZ Medium speed) 
    
	// SW GPIO ����
	RCC->AHB1ENR 	|= 0x80;	// RCC_AHB1ENR GPIOH Enable							
	GPIOH->MODER 	|= 0x00000000;	// GPIOH PIN8~PIN15 Input mode (reset state)				
}	

void EXTI_Init_Test(void)
{
	RCC->AHB1ENR 	|= 0x80;	// RCC_AHB1ENR GPIOH Enable
	RCC->APB2ENR 	|= 0x4000;	//Enable System Configuration Controller Clock
	
	GPIOH->MODER 	|= 0x0;		// GPIOH PIN8~PIN15 Input mode (reset state)				 
	
	SYSCFG->EXTICR[2] |= 0x77; 	// EXTI8,9�� ���� �ҽ� �Է��� GPIOH�� ����	
	
	EXTI->FTSR 	|= 0x100;	// Falling Trigger Enable  
	EXTI->IMR 	|= 0x300;  	// EXTI8,9 ���ͷ�Ʈ mask
		                                                                                                                                                                                                                                                                                                                                                         
	NVIC->ISER[0]	|= ( 1 << 23 );	// Enable Interrupt EXTI8,9 Vector table Position ����
}

void TIMER3_Init_Test(void)
{
	RCC->APB1ENR |= 0x02;		//RCC_APB1ENR TIMER3 Enable
    
	NVIC->ISER[0]|= ( 1 << 29 );	// Enable Timer3 global Interrupt
    
	TIM3->PSC	= 8400-1;	// Prescaler 84,000,000Hz/8400 = 10,000 Hz(0.1ms)  (1~65536)
	TIM3->ARR	= 5000-1;	// ���ͷ�Ʈ�ֱ� 0.1ms * 5000 = 500ms
	TIM3->CR1	|=(0<<4);	// Countermode = Upcounter (reset state)
	TIM3->CR1	|=(0x00<<8);	// Clock division = 1 (reset state)
	TIM3->EGR	|=(1<<0);	// Update generation    

	TIM3->DIER 	|= (1<<0);	// Enable the Tim3 Update interrupt
	TIM3->CR1 	|= (1<<0);	// Enable the Tim3 Counter    
}

void EXTI9_5_IRQHandler(void)	// EXTI 5~9 ���ͷ�Ʈ �ڵ鷯
{
	if(EXTI->PR & 0x0100) 	//Interrupt Pending?
	{
		EXTI->PR |= 0x0100;	// Pending bit Clear
		while(1);	// ���ѷ��� ����
	}	    
}

void TIM3_IRQHandler(void) // 500ms int
{
	static int LED_cnt=0;
    
	TIM3->SR &=~(1<<0);	// Interrupt flag Clear
	GPIOG->ODR ^= 0x01;	//LED0 Toggle 500ms
}

void DisplayTitle(void)
{
	LCD_Clear(RGB_WHITE);
	LCD_SetFont(&Gulim8);
	LCD_SetBackColor(RGB_GREEN);
	LCD_SetTextColor(RGB_BLACK);
	LCD_DisplayText(0,0,"MENU");

	LCD_SetFont(&Gulim8);		//��Ʈ 
	LCD_SetBackColor(RGB_RED);	//���ڹ���
	LCD_SetTextColor(RGB_BLACK);	//���ڻ�
      
	LCD_DisplayText(1,0,"1.LED0  On");
	LCD_DisplayText(2,0,"2.LED1 Off");
	LCD_DisplayText(3,0,"3.LED2 Off");
}

void DelayMS(unsigned short wMS)
{
	register unsigned short i;

	for (i=0; i<wMS; i++)
		DelayUS(1000);	// 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
	volatile int Dly = (int)wUS*17;
	for(; Dly; Dly--);
}