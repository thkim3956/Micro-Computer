/////////////////////////////////////////////////////////////
// Mode: Output Compare Mode
// Timer: Timer4 CH1
// 동작: Compare Match Interrupt(CC1I) 발생 & OC1을 통한 Pulse 출력
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

void _GPIO_Init(void);
void _EXTI_Init(void);
uint16_t KEY_Scan(void);
void TIMER4_OC_Init(void);	// General-purpose Timer 4 (Output Compare mode)

void DisplayInitScreen(void);

void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);
void BEEP(void);

int main(void)
{
	_GPIO_Init();
	_EXTI_Init();
	LCD_Init();	
	DelayMS(10);
	BEEP();
   	DisplayInitScreen();	// LCD 초기화면
	GPIOG->ODR &= 0xFF00;// 초기값: LED0~7 Off

	TIMER4_OC_Init();	// TIM4 Init (Output Compare mode: UI & CCI 발생, )                  
 
	while(1)
	{
		// TIMER Example SW4~6을 사용하여 LED0의 밝기 조절
		switch(KEY_Scan())
		{
			case SW4_PUSH  : 	//SW4
                		GPIOG->ODR &= 0x0F;	// LED4~7 OFF
                		GPIOG->ODR |= 0x10;	// LED4 ON
                		TIM4->CCR1 = 1000; 	// 1ms ON
 			break;

			case SW5_PUSH  : 	//SW5
                		GPIOG->ODR &= 0x0F;	// LED4~7 OFF
                		GPIOG->ODR |= 0x20;	// LED5 ON
                		TIM4->CCR1 = 4000; 	// 4ms ON
			break;

            		case SW6_PUSH  : 	//SW6
                		GPIOG->ODR &= 0x0F;	// LED4~7 OFF
                		GPIOG->ODR |= 0x40;	// LED6 ON
                		TIM4->CCR1 = 8000;  	// 8ms ON
			break;	
        	}
	}
}

void TIMER4_OC_Init(void)
{
// PD12: TIM4_CH1
// PD12을 출력설정하고 Alternate function(TIM4_CH1)으로 사용 선언
	RCC->AHB1ENR	|= (1<<3);	// 0x08, RCC_AHB1ENR GPIOD Enable : AHB1ENR.3

	GPIOD->MODER    |= (2<<24);	// 0x02000000(MODER.(25,24)=0b10), GPIOD PIN12 Output Alternate function mode 					
	GPIOD->OSPEEDR 	|= (3<<24);	// 0x03000000(OSPEEDER.(25,24)=0b11), GPIOD PIN12 Output speed (100MHz High speed)
	GPIOD->OTYPER	&= ~(1<<12);	// ~0x1000, GPIOD PIN12 Output type push-pull (reset state)
	GPIOD->PUPDR    |= (1<<24); 	// 0x01000000, GPIOD PIN12 Pull-up
  					// PD12 ==> TIM4_CH1
	GPIOD->AFR[1]	|= 0x00020000;  // (AFR[1].(19~16)=0b0010): Connect TIM4 pins(PD12) to AF2(TIM3..5)
 
// Time base 설정
	RCC->APB1ENR |= (1<<2);	// 0x04, RCC_APB1ENR TIMER4 Enable

	// Setting CR1 : 0x0000 
	TIM4->CR1 &= ~(1<<4);	// DIR=0(Up counter)(reset state)
	TIM4->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled): By one of following events
                            //  Counter Overflow/Underflow, 
                            //  Setting the UG bit Set,
                            //  Update Generation through the slave mode controller 
                            // UDIS=1 : Only Update event Enabled by  Counter Overflow/Underflow,
	TIM4->CR1 &= ~(1<<2);	// URS=0(Update event source Selection): one of following events
                            //	Counter Overflow/Underflow, 
                            // Setting the UG bit Set,
                            //	Update Generation through the slave mode controller 
                            // URS=1 : Only Update Interrupt generated  By  Counter Overflow/Underflow,
	TIM4->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
	TIM4->CR1 |= (1<<7);	// ARPE=1(ARR is buffered): ARR Preload Enalbe 
	TIM4->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
	TIM4->CR1 &= ~(3<<5); 	// CMS(Center-aligned mode Sel)=00 : Edge-aligned mode(reset state)

	// Setting the Period
	TIM4->PSC = 840-1;	// Prescaler=84, 84MHz/84 = 1MHz (1us)
	TIM4->ARR = 50000-1;	// Auto reload  : 1us * 10K = 10ms(period) : 인터럽트주기나 출력신호의 주기 결정

	// Update(Clear) the Counter
	TIM4->EGR |= (1<<0);    // UG: Update generation    

// Output Compare 설정
	// CCMR1(Capture/Compare Mode Register 1) : Setting the MODE of Ch1 or Ch2
	TIM4->CCMR1 &= ~(3<<0); // CC1S(CC1 channel) = '0b00' : Output 
	TIM4->CCMR1 &= ~(1<<2); // OC1FE=0: Output Compare 1 Fast disable 
	TIM4->CCMR1 &= ~(1<<3); // OC1PE=0: Output Compare 1 preload disable(CCR1에 언제든지 새로운 값을 loading 가능) 
	TIM4->CCMR1 |= (3<<4);	// OC1M=0b011 (Output Compare 1 Mode : toggle)
        //TIM4->CCMR1 |= (1<<4);	// OC1M=0b011 (Output Compare 1 Mode : toggle)
				// OC1REF toggles when CNT = CCR1
				
	// CCER(Capture/Compare Enable Register) : Enable "Channel 1" 
	TIM4->CCER |= (1<<0);	// CC1E=1: CC1 channel Output Enable
        //TIM4->CCER &= ~(1<<0);	// CC1E=1: CC1 channel Output Disable
				// OC1(TIM4_CH1) Active: 해당핀(100번)을 통해 신호출력
	TIM4->CCER &= ~(1<<1);	// CC1P=0: CC1 channel Output Polarity (OCPolarity_High : OC1으로 반전없이 출력)  

	// CC1I(CC 인터럽트) 인터럽트 발생시각 또는 신호변화(토글)시기 결정: 신호의 위상(phase) 결정
	// 인터럽트 발생시간(10000 펄스)의 10%(1000) 시각에서 compare match 발생
	TIM4->CCR1 = 25000;	// TIM4 CCR1 TIM4_Pulse  //ARR보다 CCR의 값이 작아야함!!

	TIM4->DIER |= (1<<0);	// UIE: Enable Tim4 Update interrupt
	TIM4->DIER |= (1<<1);	// CC1IE: Enable the Tim4 CC1 interrupt

	NVIC->ISER[0] 	|= (1<<30);	// Enable Timer4 global Interrupt on NVIC

	TIM4->CR1 |= (1<<0);	// CEN: Enable the Tim4 Counter  					
}

void TIM4_IRQHandler(void)      //RESET: 0
{
	if ((TIM4->SR & 0x01) != RESET)	// Update interrupt flag (10ms)
	{
		TIM4->SR &= ~(1<<0);	// Update Interrupt Claer
		GPIOG->ODR |= 0x01;	// LED0 On
	}
    
	if((TIM4->SR & 0x02) != RESET)	// Capture/Compare 1 interrupt flag
	{
		TIM4->SR &= ~(1<<1);	// CC 1 Interrupt Claer
		GPIOG->ODR &= ~0x01;	// LED0 Off
	}
}

void _GPIO_Init(void)
{
	// LED (GPIO G) 설정
    	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
 	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
    
	// SW (GPIO H) 설정 
	RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	// Buzzer (GPIO F) 설정 
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
	
	SYSCFG->EXTICR[2] |= 0x0077; 	// EXTI8,9에 대한 소스 입력은 GPIOH로 설정 (EXTICR3) (reset value: 0x0000)	
	
	EXTI->FTSR |= 0x000100;		// Falling Trigger Enable  (EXTI8:PH8)
    	EXTI->RTSR |= 0x000200;		// Rising Trigger  Enable  (EXTI9:PH9) 
    	EXTI->IMR  |= 0x000300;  	// EXTI8,9 인터럽트 mask (Interrupt Enable)
		
	NVIC->ISER[0] |= (1<<23);   	// Enable Interrupt EXTI8,9 Vector table Position 참조
}

void EXTI9_5_IRQHandler(void)		// EXTI 5~9 인터럽트 핸들러
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
	LCD_Clear(RGB_WHITE);		// 화면 클리어
	LCD_SetFont(&Gulim8);		// 폰트 : 굴림 8
	LCD_SetBackColor(RGB_GREEN);	// 글자배경색 : Green
	LCD_SetTextColor(RGB_BLACK);	// 글자색 : Black

	LCD_DisplayText(0,0,"TIM4(CH1) OUTPUT COMPARE MODE");  // Title

	LCD_SetBackColor(RGB_YELLOW);	//글자배경색 : Yellow
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
		{   	key_flag = 1;		
			DelayMS(10);
			return key;
		}
	}
}
