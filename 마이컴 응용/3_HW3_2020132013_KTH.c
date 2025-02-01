//////////////////////////////////////////////////////////////////////
//HW3: 온도연동 냉방기 제어
//제출자: 2020132013, 김태환
//제출일: 2024년 11월 10일
// 개요: 온도센서(가변저항)로부터 출력되는 아날로그신호의 전압을 AD변환함(ADC3_IN1) -- Starting trigger(TIM1_CH3의 CC인터럽트 200ms주기)
//        변환 결과값(디지털값)으로부터 센서의 전압값 및 온도값을 계산하여 LCD에 표시 온도 범위:1.0~39.1  전압 범위: 0~3.3V
//         온도 변경에 따라 길이가 변경되는 막대 그래프 표시 1일 때 가장 짧고, 39일 때 가장 길게
//        온도에 비례하여 냉방기의 냉각기를 제어함(냉각 제어용 PWM신호 발생 TIM14_CH1(PF9,부저)의 PWM 기능 이용
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
void TIMER1_Init(void);
void TIMER14_PWM_Init(void);

void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);
void BEEP(void);

uint16_t ADC_Value, Voltage, TMP, DR,DR_display; //ADC값, 전압값, 온도값, Duty값 저장 변수


int flag = 0; //무음 모드 flag

uint16_t graph; //그래프 채우기(픽셀 계산)
void ADC_IRQHandler(void)
{
	ADC3->SR &= ~(1<<1);		// EOC flag clear

	ADC_Value = ADC3->DR;		// Reading ADC result
	Voltage = ADC_Value * (3.3 * 100) / 4095;   // 3.3 : 4095 =  Volatge : ADC_Value 
                                                    // 100:  소수점아래 두자리까지 표시하기 위한 값  
        
        
	LCD_DisplayChar(2,5,Voltage/100 + 0x30);
	LCD_DisplayChar(2,7,Voltage%100/10 + 0x30);
	LCD_DisplayChar(2,8,Voltage%10 + 0x30);
      
        TMP = (uint16_t)((3.5 * (Voltage / 100.0) * (Voltage / 100.0) + 1) * 10);  // TMP = 3.5 * V^2 + 1, 소수점 첫째 자리까지 계산
        
        DR = (TMP / 10) * 2;  // 듀티 비율 = TMP * 2%
        DR_display = TMP *2; //듀티 값 디스플레이
        TIM14->CCR1 = (DR / 100.0) * (TIM14->ARR + 1); // CCR1 값 업데이트
        // TMP를 소수점 첫째 자리까지 LCD에 표시
        LCD_DisplayChar(3,5, (TMP / 100) + 0x30);        // 십의 자리
        LCD_DisplayChar(3,6, (TMP % 100) / 10 + 0x30);   // 일의 자리
        LCD_DisplayChar(3,7, '.');                                 // 소수점
        LCD_DisplayChar(3,8, (TMP % 10) + 0x30);         // 소수 첫째 자리
        
        LCD_DisplayChar(4,4, (DR_display / 100) + 0x30);        // 십의 자리
        LCD_DisplayChar(4,5, (DR_display % 100) / 10 + 0x30);   // 일의 자리
        LCD_DisplayChar(4,6, '.');                                 // 소수점
        LCD_DisplayChar(4,7, (DR_display % 10) + 0x30);         // 소수 첫째 자리
        
        graph = 3.6*(TMP/10); // 그래프 채우기 픽셀 계산 
        LCD_SetBrushColor(RGB_GREEN);
        LCD_DrawFillRect(11, 66, graph, 12); //138이 풀게이지
        LCD_SetBrushColor(RGB_YELLOW); // 남은 게이지 노란색 
        LCD_DrawFillRect(11+graph, 66, 138-graph, 12);
        


	// NO SWSTART !!!
}

int main(void)
{
	LCD_Init();	// LCD 구동 함수
	DelayMS(10);	// LCD구동 딜레이
 	DisplayTitle();	//LCD 초기화면구동 함수
      
	_GPIO_Init();
	_ADC_Init();
	TIMER1_Init();
        TIMER14_PWM_Init();

	// NO SWSTART !!!!
 	while(1)
	{   
          switch(KEY_Scan())	
		{
			case SW7_PUSH : 	//SW7
                          if(flag == 0)
                          {
                            flag = 1; // 무음 모드 해제
                          }
                          else if(flag == 1)
                          {
                            flag = 0; // 무음 모드
                          }
                          
                          if(flag == 1)
                          {
                            TIM14->CR1	|= (1<<0);	// CEN: Counter TIM14 enable
                          }
                          else if(flag ==0)
                          {
                            TIM14->CR1	&= ~(1<<0);	// CEN: Counter TIM14 disable
                          }				 				
				
 			break;
                }
	}
}

void _ADC_Init(void)
{   	// ADC3: PA1(pin 41)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;	// (1<<0) ENABLE GPIOA CLK (stm32f4xx.h 참조)
	GPIOA->MODER |= (3<<2*1);		// CONFIG GPIOA PIN1(PA1) TO ANALOG IN MODE
						
	RCC->APB2ENR |= RCC_APB2ENR_ADC3EN;	// (1<<10) ENABLE ADC3 CLK (stm32f4xx.h 참조)

	ADC->CCR &= ~(0X1F<<0);		// MULTI[4:0]: ADC_Mode_Independent
	ADC->CCR |=  (1<<16); 		// 0x00010000 ADCPRE:ADC_Prescaler_Div4 (ADC MAX Clock 36MHz, 84Mhz(APB2)/4 = 21MHz)
        
	ADC3->CR1 &= ~(3<<24);		// RES[1:0]= 0x00 : 12bit Resolution
	ADC3->CR1 &= ~(1<<8);		// SCAN=0 : ADC_ScanCovMode Disable
	ADC3->CR1 |=  (1<<5);		// EOCIE=1: Interrupt enable for EOC

	ADC3->CR2 &= ~(1<<1);		// CONT=0: ADC_Continuous ConvMode Disable
	ADC3->CR2 |=  (3<<28);		// EXTEN[1:0]: ADC_ExternalTrigConvEdge_Enable(Both Edge)
	ADC3->CR2 |= (0x02<<24);	// TIMER1 CC3 event 사용
	ADC3->CR2 &= ~(1<<11);		// ALIGN=0: ADC_DataAlign_Right
	ADC3->CR2 &= ~(1<<10);		// EOCS=0: The EOC bit is set at the end of each sequence of regular conversions

	ADC3->SQR1 &= ~(0xF<<20);	// L[3:0]=0b0000: ADC Regular channel sequece length 
					
 	//Channel selection, The Conversion Sequence of PIN1(ADC3_CH1) is first, Config sequence Range is possible from 0 to 17
	ADC3->SQR3 |= (1<<0);		// SQ1[4:0]=0b0001 : CH1
	ADC3->SMPR2 |= (0x7<<(3*1));	// ADC3_CH1 Sample TIme_480Cycles (3*Channel_1)
 	//Channel selection, The Conversion Sequence of PIN1(ADC3_CH1) is first, Config sequence Range is possible from 0 to 17

	NVIC->ISER[0] |= (1<<18);	// Enable ADC global Interrupt

	ADC3->CR2 |= (1<<0);		// ADON: ADC ON
}

void TIMER1_Init(void)
{
// TIM1_CH3 (PE13) : 200ms 이벤트 발생
// Clock Enable : GPIOE & TIMER1
	RCC->AHB1ENR	|= (1<<5);	// GPIOE Enable
	RCC->APB2ENR 	|= (1<< 0);	// TIMER1 Enable 
    						
// PE13을 출력설정하고 Alternate function(TIM1_CH3)으로 사용 선언 
	GPIOE->MODER 	|= (2<<2*13);	// PE13 Output Alternate function mode					
	GPIOE->OSPEEDR 	|= (3<<2*13);	// PE13 Output speed (100MHz High speed)
	GPIOE->OTYPER	&= ~(1<<13);	// PE13 Output type push-pull (reset state)
	GPIOE->AFR[1]	|= (1 << 20); 	// 0x00000200	(AFR[1].0b0001): Connect TIM1 pins(PE13) to AF1TIM1,2)					

	// Assign 'Interrupt Period' and 'Output Pulse Period'
	TIM1->PSC = 1680-1;	// Prescaler 168MHz/1680 = 0.1MHz (10us)
	TIM1->ARR = 20000;	// Auto reload  : 10us * 20K = 200ms(period)

	// CR1 : Up counting
	TIM1->CR1 &= ~(1<<4);	// DIR=0(Up counter)(reset state)
	TIM1->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled): By one of following events
				//	- Counter Overflow/Underflow, 
				// 	- Setting the UG bit Set,
				//	- Update Generation through the slave mode controller 
	TIM1->CR1 &= ~(1<<2);	// URS=0(Update event source Selection): one of following events
				//	- Counter Overflow/Underflow, 
				// 	- Setting the UG bit Set,
				//	- Update Generation through the slave mode controller 
	TIM1->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
	TIM1->CR1 &= ~(1<<7);	// ARPE=0(ARR is NOT buffered) (reset state)
	TIM1->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
	TIM1->CR1 &= ~(3<<5); 	// CMS(Center-aligned mode Sel)=00 (Edge-aligned mode) (reset state)
				// Center-aligned mode: The counter counts Up and DOWN alternatively

	// Event & Interrup Enable : UI  
	TIM1->EGR |= (1<<0);    // UG: Update generation    

	////////////////////////////////
	
    
	// Define the corresponding pin by 'Output'  
	TIM1->CCER |= (1<< 8);	// CC3E=1: CC3 channel Output Enable
				
	TIM1->CCER &= ~(1<<9);	// CC3P=0: CC3 channel Output Polarity (OCPolarity_High : OC3으로 반전없이 출력)  
        
        TIM1->BDTR |= (1<<15);  // main output enable
        
	// 'Mode' Selection : Output mode, toggle  
	TIM1->CCMR2 &= ~(3<<0); // CC3S(CC3 channel) = '0b00' : Output 
	TIM1->CCMR2 &= ~(1<<3); // OC3P=0: Output Compare 3 preload disable
	TIM1->CCMR2 |= (3<<4);	// OC3M=0b011: Output Compare 3 Mode : toggle
				// OC3REF toggles when CNT = CCR3

 	TIM1->CCR3 = 10000;	// TIM1 CCR3 TIM1_Pulse

	
	
      
	TIM1->CR1 |= (1<<0);	// CEN: Enable the Tim1 Counter  					
}

void TIMER14_PWM_Init(void)
{
// TIM14 CH1 : PF9
// Clock Enable : GPIOF & TIMER14
	RCC->AHB1ENR	|= (1<<5);	// GPIOF CLOCK Enable
	RCC->APB1ENR 	|= (1<<8);	// TIMER14 CLOCK Enable 
    						
// PF9을 출력설정하고 Alternate function(TIM14_CH1)으로 사용 선언 : PWM 출력
	GPIOF->MODER 	|= (2<<18);	// PF9 Output Alternate function mode					
	GPIOF->OSPEEDR 	|= (3<<18);	// PF9 Output speed (100MHz High speed)
	GPIOF->OTYPER	&= ~(1<<9);	// PF9 Output type push-pull (reset state)
	GPIOF->PUPDR	|= (1<<18);	// PF9 Pull-up
 	GPIOF->AFR[1]	|= (9<<4);	// (AFR[1].Connect TIM14 pins(PF9) to AF9(TIM12..14)
    
// TIM14 Channel 1 : PWM 1 mode
	// Assign 'PWM Pulse Period'
	TIM14->PSC	= 84-1;	// Prescaler 84MHz/84 = 0.01MHz (1us)
	TIM14->ARR	= 500-1;	// Auto reload  : 1us * 500 = 500us(period)

	// Setting CR1 : 0x0000 (Up counting)
	TIM14->CR1 &= ~(1<<4);	// DIR=0(Up counter)(reset state)
        
	TIM14->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled)
	TIM14->CR1 &= ~(1<<2);	// URS=0(Update event source Selection)g events
	TIM14->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
	TIM14->CR1 |= (1<<7);	// ARPE=1(ARR is buffered): ARR Preload Enable 
	TIM14->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
	TIM14->CR1 &= ~(3<<5); 	// CMS(Center-aligned mode Sel)=00 : Edge-aligned mode(reset state)
	
	// Define the corresponding pin by 'Output'  
	// CCER(Capture/Compare Enable Register) : Enable "Channel 1" 
	TIM14->CCER	|= (1<<0);	// CC1E=1: OC1(TIM14_CH1) Active(Capture/Compare 1 output enable)
					
	TIM14->CCER	&= ~(1<<1);	// CC1P=0: CC1 Output Polarity (OCPolarity_High : OC1으로 반전없이 출력)

	// Duty Ratio 
	TIM14->CCR1	= 0;		// CCR1 value

	// 'Mode' Selection : Output mode, PWM 1
	// CCMR1(Capture/Compare Mode Register) : Setting the MODE of Ch1 or Ch2
	TIM14->CCMR1 &= ~(3<<0); // CC1S(CC1 channel)= '0b00' : Output 
	TIM14->CCMR1 |= (1<<3); 	// OC1PE=1: Output Compare 1 preload Enable
	TIM14->CCMR1	|= (6<<4);	// OC1M=0b110: Output compare 3 mode: PWM 1 mode
        
	TIM14->CCMR1	|= (1<<7);	// OC1CE=1: Output compare 3 Clear enable
	
	
	TIM14->CR1	&= ~(1<<0);	// CEN: Counter TIM14 disable
  
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
	
	//NAVI.SW(PORT I) 설정
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
	LCD_Clear(RGB_YELLOW);
	LCD_SetFont(&Gulim7);		//폰트 
	LCD_SetBackColor(RGB_WHITE);	//글자배경색
	LCD_SetTextColor(RGB_BLACK);	//글자색
       LCD_DisplayText(0,0,"Air Conditioner Control");
       LCD_SetFont(&Gulim8);		//폰트 
       LCD_SetBackColor(RGB_YELLOW);	//글자배경색
       LCD_DisplayText(1,0,"2020132013 KTH");
      	LCD_DisplayText(2,0,"VOL: 0.00V");
        LCD_DisplayText(3,0,"TMP: 00.0C");
        LCD_DisplayText(4,0,"DR: 00.0%");
	LCD_SetBackColor(RGB_YELLOW);	//글자배경색
        LCD_SetPenColor(RGB_GREEN);       // 박스 테두리 녹색
        LCD_DrawRectangle(10, 65, 139, 13);

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
