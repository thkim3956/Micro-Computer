/////////////////////////////////////////////////////////////
// Step motor Position/Speed Control (Output Compare Mode)
// Pulse output pin: PE13(TIM1 CH3), Direction pin: PE14(GPIO)
//	- OC3�� ���� �޽� ���(ARR�� �̿��Ͽ� �ӵ� ����) 
//	- Compare Match Interrupt(CC3I) �߻��� �̿��Ͽ� ��ġ����(����޽� �� ����)  
//	- Step driver�� �޽� ���� 
/////////////////////////////////////////////////////////////
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

#define GOAL_DEG	720
#define STEP_DEG	7.5
#define GOAL_RPM	200

void _GPIO_Init(void);
void _EXTI_Init(void);
uint16_t KEY_Scan(void);
void TIMER1_OC_Init(void);	// Timer 1 (Output Compare mode)

void DisplayInitScreen(void);

void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);
void BEEP(void);

UINT16 INT_COUNT;

int main(void)
{
	_GPIO_Init();
	_EXTI_Init();
	LCD_Init();	
	DelayMS(10);
	BEEP();
   	DisplayInitScreen();	// LCD �ʱ�ȭ��
	GPIOG->ODR &= 0xFF00;// �ʱⰪ: LED0~7 Off

	TIMER1_OC_Init();	// TIM1 Init (Output Compare mode: CCI �߻�)                  
 
	while(1)
	{
		// SW4~7�� ����Ͽ� �޽� ��� �� ���� ����
		switch(KEY_Scan())
		{
			case SW7_PUSH  : 	//SW7 (�޽� ���)
				GPIOG->ODR &= 0x0F;	// LED4~7 OFF
				GPIOG->ODR |= 0x80;	// LED7 ON
				TIM1->CCER |= (1<<4*(3-1));// CC3E Enable -->���� ON
				TIM1->CR1 |= (1<<0);	// TIM1 Enable --> �õ� ON ��� �޽��� �߻��ϴµ� �̰Ÿ� ���ͷ�Ʈ�� ���� ������
			break;
			case SW4_PUSH  : 	//SW4 (Direction ����: L-CCW)
				GPIOG->ODR &= 0x0F;	// LED4~7 OFF
				GPIOG->ODR |= 0x10;	// LED4 ON
 				GPIOE->ODR |= 0x4000; 	// Dir. CCW  ���� (PE14)
			break;
			case SW5_PUSH  : 	//SW5 (Direction ����: H-CW)
				GPIOG->ODR &= 0x0F;	// LED4~7 OFF
				GPIOG->ODR |= 0x20;	// LED5 ON
				GPIOE->ODR &= ~0x4000; 	// Dir. CW  ���� (PE14)
			break;	
			case SW6_PUSH  : 	//SW6 (RESET)
				GPIOG->ODR &= 0x0F;	// LED4~7 OFF
				GPIOG->ODR |= 0x40;	// LED6 ON
				GPIOE->ODR |= 0x8000; 	// RESET (PE15)
			break;	
		}
	}
}

float fARR;
void TIMER1_OC_Init(void) //TIM1_CH3 CC 
{
	// PE13: TIM1_CH3
	// PE13�� ��¼����ϰ� Alternate function(TIM1_CH3)���� ��� ����
	RCC->AHB1ENR   |= (1<<4);   // RCC_AHB1ENR GPIOE Enable
	GPIOE->MODER   |= (2<<2*13);   // GPIOE PIN13 Output Alternate function mode               
	GPIOE->OSPEEDR |= (3<<2*13);   // GPIOE PIN13 Output speed (100MHz High speed)
	GPIOE->OTYPER  = 0x00000000;   // GPIOE PIN13 Output type push-pull (reset state)
	GPIOE->PUPDR   |= (1<<2*13);   // GPIOE PIN13 Pull-up
	GPIOE->AFR[1]  |= (1<<4*(13-8)); // Connect TIM1 pins(PE13) to AF1(TIM1/2)

	// PE14 : Step Driver Direction (GPIO)
	GPIOE->MODER   |= (1<<(2*14));  
	GPIOE->OSPEEDR |= (3<<(2*14));
	GPIOE->PUPDR   |= (1<<(2*14));  
	GPIOE->ODR     |= (1<<14); 

	// PE15 : Step Driver Reset (GPIO)
	GPIOE->MODER   |= (1<<(2*15));  
	GPIOE->OSPEEDR |= (3<<(2*15));
	GPIOE->PUPDR   |= (1<<(2*15));  
	GPIOE->ODR     |= (1<<15); 
   
	// Timerbase Mode
	RCC->APB2ENR   |= 0x01;// RCC_APB1ENR TIMER1 Enable

	TIM1->PSC = 168-1;   // Prescaler 168,000,000Hz/168= 1MHz (1us)

	fARR = (1000000*STEP_DEG)/(12*GOAL_RPM);  // us ����
	TIM1->ARR = fARR-1;    // �ֱ� = 1us * fARR = fARR us
	
	TIM1->CR1 &= ~(1<<4);   // DIR: Countermode = Upcounter (reset state)
	TIM1->CR1 &= ~(3<<8);   // CKD: Clock division = 1 (reset state)
	TIM1->CR1 &= ~(3<<5);    // CMS(Center-aligned mode Sel): No(reset state)

	TIM1->EGR |= (1<<0);    // UG: Update generation 
    
	// Output/Compare Mode
	TIM1->CCER &= ~(1<<4*(3-1));   // CC3E: OC3 Active  ���� OFF
	TIM1->CCER |= (1<<(4*(3-1)+1));  // CC3P: OCPolarity_Active Low  �ؼ�(����)�� �������Ѽ� ����Ͽ���
//	TIM1->CCER &= ~(1<<(4*(3-1)+1));  // CC3P: OCPolarity_Active High

	TIM1->CCR3 = 100;   // TIM1_Pulse (ä��3 -> ARR���� �������� �־���� --> ARR�� ��ȭ�ϹǷ� CCR���� �ǵ��� �۰� �����ؾ��Ѵ�.)

	TIM1->BDTR |= (1<<15);  // main output enable **������� Ÿ�̸�(Ÿ�̸�1,8)�� ����� �� ������ �����ؾ��Ѵ�!! �������ϸ� �����ȵ�!!
   
	TIM1->CCMR2 &= ~(3<<8*0); // CC3S(CC1channel): Output 
	TIM1->CCMR2 &= ~(1<<3); // OC3PE: Output Compare 3 preload disable
	TIM1->CCMR2 |= (3<<4);   // OC3M: Output Compare 3 Mode : toggle

	TIM1->CR1 &= ~(1<<7);   // ARPE: Auto reload preload disable
	TIM1->DIER |= (1<<3);   // CC3IE: Enable the Tim1 CC3 interrupt
   
	NVIC->ISER[0] |= (1<<27); // TIM1_CC
	TIM1->CR1 &= ~(1<<0);   // CEN: Disable the Tim1 Counter  �õ� OFF
}


void TIM1_CC_IRQHandler(void)      //RESET: 0 --> Ÿ�̸�1�� cc���ͷ�Ʈ�� ����ϱ� ���ؼ� �� �Լ��� ����ؾ��� 
                     // but ������Ʈ ���ͷ�Ʈ�� ����ϱ����ؼ��� ���� ���̺� �����ϸ� TIM1_UP_TIM10_IRQ�Լ� ���
{
	if ((TIM1->SR & 0x08) != RESET)	// CC3 interrupt flag 
	{
		TIM1->SR &= ~0x08;	// CC3 Interrupt Claer
		INT_COUNT++;
		if (INT_COUNT >= 2*GOAL_DEG/STEP_DEG)  // ��� �޽��� ���� 
		{
 			TIM1->CCER &= ~(1<<4*(3-1));// CC3E Disable 
			TIM1->CR1 &= ~(1<<0); // TIM1 Disable
			INT_COUNT= 0;
		}
	}
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
}	

void _EXTI_Init(void)
{
	RCC->AHB1ENR 	|= 0x0080;	// RCC_AHB1ENR GPIOH Enable
	RCC->APB2ENR 	|= 0x4000;	// Enable System Configuration Controller Clock
	
	GPIOH->MODER 	&= 0x0000FFFF;	// GPIOH PIN8~PIN15 Input mode (reset state)				 
	
	SYSCFG->EXTICR[2] |= 0x0077; 	// EXTI8,9�� ���� �ҽ� �Է��� GPIOH�� ���� (EXTICR3) (reset value: 0x0000)	
	
	EXTI->FTSR |= 0x000100;		// Falling Trigger Enable  (EXTI8:PH8)
	EXTI->RTSR |= 0x000200;		// Rising Trigger  Enable  (EXTI9:PH9) 
	EXTI->IMR  |= 0x000300;  	// EXTI8,9 ���ͷ�Ʈ mask (Interrupt Enable)
		
	NVIC->ISER[0] |= (1<<23);   	// Enable Interrupt EXTI8,9 Vector table Position ����
}

void EXTI9_5_IRQHandler(void)		// EXTI 5~9 ���ͷ�Ʈ �ڵ鷯
{
	if(EXTI->PR & 0x0100) 		// EXTI8 nterrupt Pending?
	{
		EXTI->PR |= 0x0100; 	// Pending bit Clear
	}
	else if(EXTI->PR & 0x0200) 	// EXTI9 Interrupt Pending?
	{
		EXTI->PR |= 0x0200; 	// Pending bit Clear
	}
}

void BEEP(void)			// Beep for 20 ms 
{ 	GPIOF->ODR |= 0x0200;	// PF9 'H' Buzzer on
	DelayMS(20);		// Delay 20 ms
	GPIOF->ODR &= ~0x0200;	// PF9 'L' Buzzer off
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

void DisplayInitScreen(void)
{
	LCD_Clear(RGB_WHITE);		// ȭ�� Ŭ����
	LCD_SetFont(&Gulim8);		// ��Ʈ : ���� 8
	LCD_SetBackColor(RGB_GREEN);	// ���ڹ��� : Green
	LCD_SetTextColor(RGB_BLACK);	// ���ڻ� : Black

	LCD_DisplayText(0,0,"STEP Motor Control");  // Title
	LCD_DisplayText(1,0,"Position/Speed");  // Title
	LCD_DisplayText(2,0,"TIM1(CH3) OC MODE");  // Title

	LCD_SetBackColor(RGB_YELLOW);	//���ڹ��� : Yellow
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
