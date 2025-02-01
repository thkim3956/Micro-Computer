/////////////////////////////////////////////////////////////
//HW2: 디지털 시계 제작
//제출자: 2020132013, 김태환
//개요: TIMER10의 UP-Counting 모드를 사용하여 시간표시 기능을 구현하고 TIMER2의 3채널의 Output Compare mode를 사용하여 스탑워치 기능을 구현
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

void _RCC_Init(void);
void _GPIO_Init(void);
void _EXTI_Init(void);
uint16_t KEY_Scan(void);
void TIMER2_OC_Init(void);  // Timer2 초기화
void TIM10_Init(void);  // Timer10 초기화

void DisplayInitScreen(void);

void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);
void BEEP(void);
void Stopwatch_Reset(void); //스탑워치 리셋 기능

uint8_t	SW3_Flag, SW4_Flag;


uint8_t SW3_Flag = 0; // 스탑워치 작동/중단 상태
uint8_t SW4_Flag = 0; // 스탑워치 초기화 상태
uint8_t Stopwatch_Active = 0; // 스탑워치 작동 여부
uint8_t sec_1 = 0, deci_sec = 0; // 1초, 1/10초 
uint8_t running = 0; // 스탑워치가 작동 중인지 확인


int main(void)
{
	_GPIO_Init();  		// GPIO 초기화
	_EXTI_Init();		// 외부인터럽트 초기화
	LCD_Init();		// GLCD 초기화
	DelayMS(10);			
	BEEP();			// Beep 한번 

	GPIOG->ODR &= 0xFF00;	// 초기값: LED0~7 Off
	DisplayInitScreen();		// LCD 초기화면
        Stopwatch_Reset();

	
        TIM10_Init();		// 범용타이머(TIM10) 초기화 : up counting mode
        TIMER2_OC_Init();		// 범용타이머(TIM2) 초기화 : output compare mode
	while(1)
	{

	}
}

void TIMER2_OC_Init(void)
{
// PB10: TIM2_CH3
// PB10을 출력설정하고 Alternate function(TIM4_CH1)으로 사용 선언
	RCC->AHB1ENR	|= 0x02;	// 0x02, RCC_AHB1ENR GPIOB Enable : AHB1ENR

	GPIOD->MODER    |= (2<<20);	// 0x02000000(MODER.(25,24)=0b10), GPIOB PIN10 Output Alternate function mode 					
	GPIOD->OSPEEDR 	|= (3<<20);	// 0x03000000(OSPEEDER.(25,24)=0b11), GPIOB PIN10 Output speed (100MHz High speed)
	GPIOD->OTYPER	&= ~(1<<10);	// ~0x1000, GPIOB PIN10 Output type push-pull (reset state)
	GPIOD->PUPDR    |= (1<<20); 	// 0x01000000, GPIOB PIN10 Pull-up  					
	GPIOD->AFR[1]	|= 0x00000100;  // (AFR[1].(19~16)=0b0010): Connect TIM2 pins(PB10) to AF1(TIM1/TIM2)
 
// Time base 설정
	RCC->APB1ENR |= (1<<0);	// 0x01, RCC_APB1ENR TIMER2 Enable

	// Setting CR1 : 0x0000 
	TIM2->CR1 &= ~(1<<4);	// DIR=0(Up counter)(reset state)
	TIM2->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled): By one of following events
                            //  Counter Overflow/Underflow, 
                            //  Setting the UG bit Set,
                            //  Update Generation through the slave mode controller 
                            // UDIS=1 : Only Update event Enabled by  Counter Overflow/Underflow,
	TIM2->CR1 &= ~(1<<2);	// URS=0(Update event source Selection): one of following events
                            //	Counter Overflow/Underflow, 
                            // Setting the UG bit Set,
                            //	Update Generation through the slave mode controller 
                            // URS=1 : Only Update Interrupt generated  By  Counter Overflow/Underflow,
	TIM2->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
	TIM2->CR1 |= (1<<7);	// ARPE=1(ARR is buffered): ARR Preload Enalbe 
	TIM2->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
	TIM2->CR1 &= ~(3<<5); 	// CMS(Center-aligned mode Sel)=00 : Edge-aligned mode(reset state)

	// Setting the Period
	TIM2->PSC = 16800-1;	
	TIM2->ARR = 500-1;	
        

	// Update(Clear) the Counter
	TIM2->EGR |= (1<<0);    // UG: Update generation    

// Output Compare 설정
	// CCMR2(Capture/Compare Mode Register 1) : Setting the MODE of Ch3 or Ch4
	TIM2->CCMR2 &= ~(3<<0); // CC3S(CC3 channel) = '0b00' : Output 
	TIM2->CCMR2 &= ~(1<<2); // OC3FE=0: Output Compare 1 Fast disable 
	TIM2->CCMR2 &= ~(1<<3); // OC3PE=0: Output Compare 1 preload disable(CCR1에 언제든지 새로운 값을 loading 가능) 
	TIM2->CCMR2 |= (3<<4);	// OC3M=0b011 (Output Compare 1 Mode : toggle)
        
				
				
	// CCER(Capture/Compare Enable Register) : Enable "Channel 3" 
	TIM2->CCER |= (1<<8);	// CC3E=1: CC3 channel Output Enable
      
				
	TIM2->CCER &= ~(1<<9);	// CC3P=0: CC3 channel Output Polarity (OCPolarity_High : OC3으로 반전없이 출력)  

	// CC3I(CC 인터럽트) 인터럽트 발생시각 또는 신호변화(토글)시기 결정: 신호의 위상(phase) 결정
	
	TIM2->CCR1 = 250;	// TIM2 CCR3 TIM2_Pulse  
        // CC3 Interrupt(CC3I) period: 100ms
	//TIM2->DIER |= (1<<0);	// UIE: Enable Tim2 Update interrupt
	TIM2->DIER |= (1<<3);	// CC3IE: Enable the Tim2 CC3 interrupt

	NVIC->ISER[0] 	|= (1<<28);	// Enable Timer2 global Interrupt on NVIC

	TIM2->CR1 |= (1<<0);	// CEN: Enable the Tim2 Counter  					
}

void TIM10_Init(void)
{
// Enable Timer CLK 
	RCC->APB2ENR |= 0x00020000;	// RCC_APB2ENR TIMER10 Enable

// Setting CR1 : 0x0000 
	TIM10->CR1 &= ~(1<<4);	// DIR=0(Up counter)(reset state)        
	TIM10->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled): By one of following events
				//   - Counter Overflow/Underflow, 
				//   - Setting the UG bit Set,
				//   - Update Generation through the slave mode controller 
				// UDIS=1 : Only Update event Enabled by Counter Overflow/Underflow,
	TIM10->CR1 &= ~(1<<2);	// URS=0(Update Request Source  Selection): By one of following events
				//   - Counter Overflow/Underflow, 
				//   - Setting the UG bit Set,
				//   - Update Generation through the slave mode controller 
				// URS=1 : Only Update Interrupt generated By Counter Overflow/Underflow,
	TIM10->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
	TIM10->CR1 &= ~(1<<7);	// ARPE=0(ARR is NOT buffered) (reset state)
	TIM10->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
	TIM10->CR1 &= ~(3<<5); 	// CMS(Center-aligned mode Sel)=00 (Edge-aligned mode) (reset state)
				// Center-aligned mode: The counter counts UP and DOWN alternatively

// Deciding the Period
	TIM10->PSC = 840-1;	
	TIM10->ARR = 20000-1;		
        // --> Update Interrupt(Overflow) period: 100ms
// Clear the Counter
	TIM10->EGR |= (1<<0);	// UG(Update generation)=1 
				// Re-initialize the counter(CNT=0) & generates an update of registers  카운터 초기화

// Setting an UI(UEV) Interrupt 
	NVIC->ISER[0] |= (1<<25); 	// Enable Timer10 global Interrupt
 	TIM10->DIER |= (1<<0);	// Enable the Tim10 Update interrupt

	TIM10->CR1 |= (1<<0);	// Enable the Tim10 Counter (clock enable)   
}



void TIM2_IRQHandler(void) {
    if ((TIM2->SR & (1 << 3)) != 0) { // CC3 인터럽트 플래그 체크
        TIM2->SR &= ~(1 << 3); // CC3 인터럽트 플래그 클리어
        if (Stopwatch_Active) { // 스탑워치가 작동 중일 때만
            deci_sec++;
            if (deci_sec >= 10) {
                deci_sec = 0;
                sec_1++;
            }
            if (sec_1 >= 100) {
                sec_1 = 0;
            }
            // 스탑워치 시각 표시
            LCD_SetFont(&Gulim8);
            LCD_SetTextColor(RGB_BLACK);
            LCD_DisplayChar(4, 5, sec_1 / 10 + 0x30);  // 십의 자리 초
            LCD_DisplayChar(4, 6, sec_1 % 10 + 0x30);  // 일의 자리 초
            LCD_DisplayChar(4, 7, ':');
            LCD_DisplayChar(4, 8, deci_sec + 0x30);   // 1/10초
        }
    }
}

// 스탑워치 초기화 함수
void Stopwatch_Reset(void) {
      sec_1 = 0;
    deci_sec = 0;
  
    LCD_SetFont(&Gulim8);
    LCD_SetTextColor(RGB_BLACK);
    LCD_DisplayChar(4, 5, '0'); // 시각 초기화
    LCD_DisplayChar(4, 6, '0');
    LCD_DisplayChar(4, 7, ':');
    LCD_DisplayChar(4, 8, '0');
}
void TIM1_UP_TIM10_IRQHandler(void)  	// 100ms Interrupt
{
	
        static int SEC_10=2; // 초단위(십의 자리)
        static int SEC_1=0; // 초단위 (일의 자리)
        static int MSEC=0; // msec단위
        
    
	TIM10->SR &= ~(1<<0);	// Interrupt flag Clear
        
        MSEC++;  // 100ms마다 증가
        if(MSEC>=10) //1초가 지나면 초단위(일의자리) 부분 1증가
        {
          MSEC = 0;
          SEC_1++;
        }
        if(SEC_10 < 2 && SEC_1 >=10) // 초단위 10의 자리 구현
        {
          SEC_10++;
          SEC_1 = 0;
        }
        else if(SEC_10 == 2 && SEC_1 >= 4) // 23:9 초까지작동하고 무한 반복을 위한 초기화
        {
          SEC_10 = 0;
          SEC_1 = 0;
        }
        LCD_SetFont(&Gulim8);	// 폰트 : 굴림 8
	LCD_SetBackColor(RGB_YELLOW);// 글자배경색 : Yellow
	LCD_SetTextColor(RGB_BLACK);// 글자색 : Black
        LCD_DisplayChar(3,5,SEC_10+0x30);
        LCD_DisplayChar(3,6,SEC_1+0x30);
        LCD_DisplayChar(3,7,':');
        LCD_DisplayChar(3,8,MSEC+0x30);
	
	
        
}

void _GPIO_Init(void)
{
	// LED (GPIO G) 설정
    	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x00FF;	    // GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
 	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
    
	// SW (GPIO H) 설정 
	RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	// Buzzer (GPIO F) 설정 
	RCC->AHB1ENR	|=  0x00000020;     // RCC_AHB1ENR : GPIOF(bit#5) Enable							
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
	GPIOF->OTYPER 	&= ~0x0200;	    // GPIOF 9 : Push-pull  	
	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 
}	

void _EXTI_Init(void)
{
	RCC->AHB1ENR 	|= 0x0080;	// RCC_AHB1ENR GPIOH Enable
	RCC->APB2ENR 	|= 0x4000;	// Enable System Configuration Controller Clock
	
	GPIOH->MODER 	&= 0x0000FFFF;	// GPIOH PIN8~PIN15 Input mode (reset state)				 
	
        SYSCFG->EXTICR[2] &= ~0x7000;   //초기화
        SYSCFG->EXTICR[3] &= ~0x0007;   //초기화

	
        SYSCFG->EXTICR[2] |= 0x7000;	// EXTI 11 에 대한 소스 입력은 GPIOH로 설정
        SYSCFG->EXTICR[3] |= 0x0007;    // EXTI 12 에 대한 소스 입력은 GPIOH로 설정
	
        EXTI->FTSR |= 0x1800;		//11, 12falling trigger 설정 
	EXTI->IMR  |= 0x1800;		// EXTI 11, 12 인터럽트 mask (Interrupt Enable) 설정
		
	
        NVIC->ISER[1] |= ( 1 << 8  );	// Enable 'Global Interrupt'  
}

void EXTI15_10_IRQHandler(void) {
    if (EXTI->PR & 0x0800) { // SW3(PH11): 작동/중단
        EXTI->PR |= 0x0800; // 인터럽트 플래그 클리어
        Stopwatch_Active = !Stopwatch_Active; // 토글링
        BEEP();   
        if (Stopwatch_Active == 1) {
            GPIOG->ODR |= 0x08; // LED3 On
            GPIOG->ODR |= 0x10; // LED4 On
            
        } else {
            GPIOG->ODR &= ~0x08; // LED3 Off
            GPIOG->ODR |= 0x10; // LED4 On
        }
    }
    else if (EXTI->PR &= 0x1000) { // SW4(PH12): 초기화
        EXTI->PR |= 0x1000; // 인터럽트 플래그 클리어
        if (Stopwatch_Active == 0) { // 스탑워치가 중단된 상태에서만 초기화
            BEEP();
            Stopwatch_Reset();
            GPIOG->ODR &= ~0x08; // LED3 Off
            GPIOG->ODR &= ~0x10; // LED4Off
        }
    }
}

void BEEP(void)			/* beep for 30 ms */
{ 	GPIOF->ODR |= 0x0200;	// PF9 'H' Buzzer on
	DelayMS(30);		// Delay 30 ms
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
	LCD_Clear(RGB_YELLOW);	// 화면 클리어
	LCD_SetFont(&Gulim8);	// 폰트 : 굴림 8
	LCD_SetBackColor(RGB_YELLOW);// 글자배경색 : Yellow
	LCD_SetTextColor(RGB_BLUE);// 글자색 : Blue
	LCD_DisplayText(1,0,"Digital Watch");  // Title
        LCD_SetTextColor(RGB_GREEN);// 글자색 : Green
	LCD_DisplayText(2,0,"2020132013 KTH");  // Title
        LCD_SetPenColor(RGB_BLACK);       // 박스 테두리 검정
        LCD_DrawRectangle(0, 12, 113, 26);     // 박스
        LCD_SetTextColor(RGB_BLUE);// 글자색 : Blue
        LCD_DisplayText(3,0,"TIME"); 
        LCD_DisplayText(4,0,"STW"); 

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
