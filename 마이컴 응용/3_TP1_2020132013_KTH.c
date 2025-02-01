//////////////////////////////////////////////////////////////////////
//TP1:Smart Watch 
//제출자: 2020132013 김태환
//제출일: 2024.12.10
//과제 개요:
// 3개의 기능(화면)을 갖는 smart watch를 제작함. 
// 각 기능(화면)은 스위치(SW7)를 입력하면 전환이 되도록 힘.
// 초기(1번) 화면은 ‘Alarm’, 2번 화면은 ‘Ball game‘, 3번 화면은‘Thermostat’ 임. 3번 화면에서 SW7을 입력하면 1번 화면(’Alarm’)으로 전환함.

// (세부 규칙)
// 각 화면에는 동일한 시계(시간)가 오른쪽 상단에 항상 표시되도록함.
// Ball game, Thermostat 화면에서는 알람설정 스위치를 입력해도무시되도록설정해야 함.
// Ball game, Thermostat 화면에서도 알람시각이 되면 알람은 울려야함.
// Thermostat 화면에서 설정한 PWM신호는 다른 화면에서도 계속발생하도록함.
//////////////////////////////////////////////////////////////////////
#include "stm32f4xx.h"
#include "GLCD.h"
#include "FRAM.h"
#include "ACC.h"

#define SW0_PUSH        0xFE00  //PH8
#define SW1_PUSH        0xFD00  //PH9
#define SW2_PUSH        0xFB00  //PH10
#define SW3_PUSH        0xF700  //PH11
#define SW4_PUSH        0xEF00  //PH12
#define SW5_PUSH        0xDF00  //PH13
#define SW6_PUSH        0xBF00  //PH14
#define SW7_PUSH        0x7F00  //PH15

void USART_BRR_Configuration(uint32_t USART_BaudRate);
void SerialSendChar(uint8_t c);
void SerialSendString(char *s);
uint16_t KEY_Scan(void);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);
void BEEP(void);
uint8_t str[20];
void SPI1_Process(int16 *pBuf);  // ACC.c (ACC.h) 
void ACC_Init(void); // ACC.c (ACC.h)
void SPI1_Init(void);

void Display_Process(int16 *pBuf);

//----------------TIM2,TIM3,TIM6,TIM13 USART1, ADC2, GPIO, EXTI 설정-------------
void _GPIO_Init(void);
void USART1_Init(void);
void TIMER2_Init(void);
void TIMER3_PWM_Init(void);
void TIMER6_Init(void);
void TIMER13_Init(void);
void _ADC_Init(void);   //ADC2
void _EXTI_Init(void);

//-------------함수 선언-------------------
void Dispay_ALARM(void); //1번 알람 화면
void Dispay_Game(void);  //2번 Ball Game 화면 
void Dispay_Thermostat(void); //3번 Tehrmostat 화면
char ConvertToHexChar(uint8_t); //16진수로 변환하는 함수
void Display_ball_position(void);
//-----------Fram 변수 설정------------------
uint8_t initial_alarm;  //초기 알람시각 저장용
uint8_t set_alarm;       // 변경 알람시각 저장용
//------------변수 설정--------------------
int page = 1; //화면 변경을 위한 변수(1:알람, 2:Ball Game, 3:Thermostat)
uint8_t sec = 10; //초기 현재시각 초: A로 설정
uint8_t min = 15; //초기 현재시각 분: F로 설정
UINT8 bControl; //  SPI통신을 이용한 가속도센서 측정값 업데이트를 위한 값
int ball_x = 58; //ball의 x 초기 위치
int ball_y = 66; //ball의 y 초기 위치
#define SPEED     10   // 기울기에 따른 속도 가중치
uint16_t ADC_Value, Voltage; //ADC값, 전압 값
#define RGB_SKY_BLUE 	GET_RGB(135, 206, 235) //하늘색 정의
uint16_t graph; //그래프 채우기(픽셀 계산)
int H_level, C_level; //히터 및 쿨러 레벨

int main(void)
{
	int16 buffer[3];
	LCD_Init();   // LCD 모듈 초기화
	DelayMS(10);	    
	_GPIO_Init();   // GPIO 초기화
	_EXTI_Init();	// EXTI 초기화
	
	Fram_Init();              
	Fram_Status_Config();
	initial_alarm = Fram_Read(1200); //Fram('시':1200번지)에서 Read
	if (initial_alarm > 15) initial_alarm = 0; //Fram에 유효한 값이 없을 때 0
    
	USART1_Init();
	GPIOG->ODR &= 0x00;	// LED0~7 Off 
	SPI1_Init();    // SPI1 초기화
	ACC_Init();		// 가속도센서 초기화	
	TIMER2_Init();  // TIM2 초기화
	TIMER3_PWM_Init(); //TIM3_PWM 초기화
	TIMER6_Init();  // TIM6 초기화
	TIMER13_Init(); // TIM13 초기화
	Dispay_ALARM(); //초기화면
	_ADC_Init();	// ADC2 초기화
	while(1)
	{
		if(page == 2)
		{
			if(bControl)
			{
				bControl = FALSE;     
				SPI1_Process(&buffer[0]);	// SPI통신을 이용하여 가속도센서 측정
				Display_Process(&buffer[0]);	// 측정값을 LCD에 표시				
			}
		}		
	}
}

void Dispay_ALARM(void)
{	
	LCD_Clear(RGB_WHITE);
	LCD_SetFont(&Gulim8);
	LCD_SetBackColor(RGB_WHITE);	
	LCD_SetTextColor(RGB_BLACK);
	LCD_DisplayText(0,0,"1.ALARM(KTH)");
	LCD_DisplayText(1,0,"Alarm");
	LCD_DisplayText(1,7,":0");
	LCD_SetTextColor(RGB_RED);
	LCD_DisplayChar(1,6,ConvertToHexChar(initial_alarm));
	LCD_SetTextColor(RGB_BLUE);
	LCD_DisplayChar(0,16,ConvertToHexChar(min));
	LCD_DisplayChar(0,17,':');
	LCD_DisplayChar(0,18,ConvertToHexChar(sec));
}

void Dispay_Game(void)
{	
	LCD_Clear(RGB_WHITE);
	LCD_SetFont(&Gulim8);
	LCD_SetBackColor(RGB_WHITE);	
	LCD_SetTextColor(RGB_BLACK);
	LCD_DisplayText(0,0,"2.Ball game");
	LCD_SetTextColor(RGB_BLUE);
	LCD_DisplayChar(0,16,ConvertToHexChar(min));
	LCD_DisplayChar(0,17,':');
	LCD_DisplayChar(0,18,ConvertToHexChar(sec));
	LCD_SetPenColor(RGB_BLACK);
    LCD_DrawRectangle(10,20,100,100);
	LCD_SetTextColor(RGB_BLACK);
	LCD_SetFont(&Gulim7);
	LCD_DisplayText(2,19,"Ax:");
	LCD_DisplayText(3,19,"Ay:");
	LCD_SetPenColor(RGB_RED);
    LCD_DrawRectangle(ball_x,ball_y,6,6);
}

void Dispay_Thermostat(void)
{	
	LCD_Clear(RGB_WHITE);
	LCD_SetFont(&Gulim8);
	LCD_SetBackColor(RGB_WHITE);	
	LCD_SetTextColor(RGB_BLACK);
	LCD_DisplayText(0,0,"3.Thermostat");
	LCD_DisplayText(1,0,"T:");
	LCD_DisplayText(2,0,"H:");
	LCD_DisplayText(2,4,"C:");
	LCD_SetTextColor(RGB_BLUE);
	LCD_DisplayChar(0,16,ConvertToHexChar(min));
	LCD_DisplayChar(0,17,':');
	LCD_DisplayChar(0,18,ConvertToHexChar(sec));
	LCD_SetPenColor(RGB_GREEN);
    LCD_DrawRectangle(40,12,91,13);
}

char ConvertToHexChar(uint8_t value) //16진수로 변환하는 함수
{
    if (value <= 9) {
        return value + 0x30;  // '0' ~ '9'
    } else {
        return value + 0x37;  // 'A' ~ 'F'
    }
}

void USART1_IRQHandler(void)	
{        
	// RX Buffer Full interrupt
	if(page == 1)
	{
		if ( (USART1->SR & USART_SR_RXNE) )		// USART_SR_RXNE=(1<<5) 
		{
			char ch;
			ch = (uint16_t)(USART1->DR & (uint16_t)0x01FF);
				
			if(ch >= 0x00 && ch < 0x10) 
			{
				BEEP();
				set_alarm = ConvertToHexChar(ch);
				LCD_SetTextColor(RGB_RED);
				LCD_DisplayChar(1,6,set_alarm);
				Fram_Write(1200,ch);			
			}
			else // 수신된 숫자가 0~F 이외의 숫자인 경우
			{
				BEEP();
				DelayMS(200);
				BEEP();
				DelayMS(200);			
			}
		} 
	}	
}

void _ADC_Init(void)
{   	// ADC2: PA1(pin 41)
	RCC->AHB1ENR |= (1<<0);	// (1<<0) ENABLE GPIOA CLK (stm32f4xx.h 참조)
	GPIOA->MODER |= (3<<2*1);		// CONFIG GPIOA PIN1(PA1) TO ANALOG IN MODE
						
	RCC->APB2ENR |= (1<<9);	// (1<<9) ENABLE ADC2 CLK (stm32f4xx.h 참조)

	ADC->CCR &= ~(0X1F<<0);		// MULTI[4:0]: ADC_Mode_Independent
	ADC->CCR |=  (1<<16); 		// 0x00010000 ADCPRE:ADC_Prescaler_Div4 (ADC MAX Clock 36MHz, 84Mhz(APB2)/4 = 21MHz)
        
	ADC2->CR1 &= ~(3<<24);		// RES[1:0]= 0x00 : 12bit Resolution
	ADC2->CR1 &= ~(1<<8);		// SCAN=0 : ADC_ScanCovMode Disable
	ADC2->CR1 |=  (1<<5);		// EOCIE=1: Interrupt enable for EOC

	ADC2->CR2 &= ~(1<<1);		// CONT=0: ADC_Continuous ConvMode Disable
	ADC2->CR2 |=  (3<<28);		// EXTEN[1:0]: ADC_ExternalTrigConvEdge_Enable(Both Edge)
	ADC2->CR2 |= (5<<24);		// TIMER2 CC4 event 사용
	ADC2->CR2 &= ~(1<<11);		// ALIGN=0: ADC_DataAlign_Right
	ADC2->CR2 &= ~(1<<10);		// EOCS=0: The EOC bit is set at the end of each sequence of regular conversions

	ADC2->SQR1 &= ~(0xF<<20);	// L[3:0]=0b0000: ADC Regular channel sequece length 					
 	//Channel selection, The Conversion Sequence of PIN1(ADC2_CH1) is first, Config sequence Range is possible from 0 to 17
	ADC2->SQR3 |= (1<<0);		// SQ1[4:0]=0b0001 : CH1
	ADC2->SMPR2 |= (0x7<<(3*1));	// ADC2_CH1 Sample TIme_480Cycles (3*Channel_1)
 	//Channel selection, The Conversion Sequence of PIN1(ADC2_CH1) is first, Config sequence Range is possible from 0 to 17

	NVIC->ISER[0] |= (1<<18);	// Enable ADC global Interrupt

	ADC2->CR2 |= (1<<0);		// ADON: ADC ON
}

void ADC_IRQHandler(void)
{
	ADC2->SR &= ~(1<<1);		// EOC flag clear
	
	ADC_Value = ADC2->DR;		// Reading ADC result    
	Voltage = ADC_Value * (3.3 * 100) / 4095;   // 3.3 : 4095 =  Volatge : ADC_Value 			
	// 전압 → 온도로 변환 (-10°C ~ 30°C)
	int temperature = -10 + (40 * Voltage / 330);
	//Thermostat 구동을 위한 PWM발생
	if (temperature <= 0) 
	{
		GPIOG->ODR |= 0x0020;	// LED5 on
		GPIOG->ODR &= ~0x0040;	// LED6 Off 
		TIM3->CCR3	= 1000;		// CCR3 value --> Duty Ratio 90%
	}
	else if(temperature > 0 && temperature <= 10)
	{
		GPIOG->ODR |= 0x0020;	// LED5 on
		GPIOG->ODR &= ~0x0040;	// LED6 Off
		TIM3->CCR3	= 9000;		// CCR3 value --> Duty Ratio 10%
	} 
	else if(temperature > 10 && temperature <= 20) 
	{
		GPIOG->ODR &= ~0x0020;	// LED5 off
		GPIOG->ODR &= ~0x0040;	// LED6 off
		TIM3->CCR3	= 9000;		// CCR3 value --> Duty Ratio 10%
	}
	else if(temperature > 20 && temperature <= 30) 
	{
		GPIOG->ODR &= ~0x0020;	// LED5 off
		GPIOG->ODR |= 0x0040;	// LED6 On
		TIM3->CCR3	= 1000;		// CCR3 value --> Duty Ratio 90%
	}

	if(page == 3)
	{		
		if (temperature <= 0) 
        {
			LCD_SetTextColor(RGB_GREEN);
            LCD_DisplayChar(1, 2, '-'); // 음수 부호 표시  
			H_level = 2;
			C_level = 0;          
			LCD_SetTextColor(RGB_RED);
			LCD_DisplayChar(2, 2, H_level + 0x30); // Heater 레벨 표시
			LCD_SetTextColor(RGB_BLUE);
			LCD_DisplayChar(2, 6, C_level + 0x30); // Cooler 레벨 표시
			LCD_SetBrushColor(RGB_BLUE);								
        } 
        else 
        {
			LCD_SetTextColor(RGB_GREEN);
            LCD_DisplayChar(1, 2, '+'); // 양수 부호 표시
			if(temperature > 0 && temperature <= 10) 
			{
				LCD_SetBrushColor(RGB_SKY_BLUE);
				H_level = 1;
				C_level = 0;
				LCD_SetTextColor(RGB_RED);
				LCD_DisplayChar(2, 2, H_level + 0x30); // Heater 레벨 표시
				LCD_SetTextColor(RGB_BLUE);
				LCD_DisplayChar(2, 6, C_level + 0x30); // Cooler 레벨 표시								 				
			}
			else if(temperature > 10 && temperature <= 20)
			{
				LCD_SetBrushColor(RGB_GREEN);
				H_level = 0;
				C_level = 0;
				LCD_SetTextColor(RGB_RED);
				LCD_DisplayChar(2, 2, H_level + 0x30); // Heater 레벨 표시
				LCD_SetTextColor(RGB_BLUE);
				LCD_DisplayChar(2, 6, C_level + 0x30); // Cooler 레벨 표시								
			} 
			else if(temperature > 20 && temperature <= 30)
			{
				LCD_SetBrushColor(RGB_RED);
				H_level = 0;
				C_level = 1;
				LCD_SetTextColor(RGB_RED);
				LCD_DisplayChar(2, 2, H_level + 0x30); // Heater 레벨 표시
				LCD_SetTextColor(RGB_BLUE);
				LCD_DisplayChar(2, 6, C_level + 0x30); // Cooler 레벨 표시								
			} 
        }        
		LCD_SetTextColor(RGB_GREEN);
        // 온도값 출력 
        LCD_DisplayChar(1, 3, abs(temperature) / 10 + 0x30); // 십의 자리
        LCD_DisplayChar(1, 4, abs(temperature) % 10 + 0x30); // 일의 자리

		//막대 그래프
		graph = 30 + (2 * temperature);
        LCD_DrawFillRect(41, 13, graph, 12); //90이 풀게이지
        LCD_SetBrushColor(RGB_WHITE); // 남은 게이지 흰색 
        LCD_DrawFillRect(41+graph, 13, 90-graph, 12);
	}

}
void TIMER3_PWM_Init(void)
{
	// TIM3 CH3 : PB0
// Clock Enable : GPIOB & TIMER3
	RCC->AHB1ENR	|= (1<<1);	// GPIOB CLOCK Enable
	RCC->APB1ENR 	|= (1<<1);	// TIMER3 CLOCK Enable 
    						
// PB0을 출력설정하고 Alternate function(TIM3_CH3)으로 사용 선언 : PWM 출력
	GPIOB->MODER 	|= (2<<0);	// PB0 Output Alternate function mode					
	GPIOB->OSPEEDR 	|= (3<<0);	// PB0 Output speed (100MHz High speed)
	GPIOB->OTYPER	&= ~(1<<0);	// PB0 Output type push-pull (reset state)
	GPIOB->PUPDR	|= (1<<0);	// PB0 Pull-up
 	GPIOB->AFR[0]	|= (2<<0);	// (AFR[0].Connect TIM3 pins(PB0) to AF2
    
// TIM3 Channel 3 : PWM 2 mode
	// Assign 'PWM Pulse Period'
	TIM3->PSC	= 16800-1;	// Prescaler 84MHz/16800 = 0.2ms
	TIM3->ARR	= 10000-1;	// Auto reload  : 0.2ms * 10000 =  2sec

	// Setting CR1 : 0x0000 (Up counting)
	TIM3->CR1 &= ~(1<<4);	// DIR=0(Up counter)(reset state)
        
	TIM3->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled)
	TIM3->CR1 &= ~(1<<2);	// URS=0(Update event source Selection)g events
	TIM3->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
	TIM3->CR1 |= (1<<7);	// ARPE=1(ARR is buffered): ARR Preload Enable 
	TIM3->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
	TIM3->CR1 &= ~(3<<5); 	// CMS(Center-aligned mode Sel)=00 : Edge-aligned mode(reset state)
	
	// Define the corresponding pin by 'Output'  
	// CCER(Capture/Compare Enable Register) : Enable "Channel 3" 
	TIM3->CCER	|= (1<<8);	// CC3E=1: OC3(TIM3_CH3) Active(Capture/Compare 3 output enable)
					
	TIM3->CCER	|= (1<<9);	// CC3P=1: CC3 Output Polarity OC3으로 반전하여 출력

	// Duty Ratio 
	TIM3->CCR3	= 0;		// CCR3 value

	// 'Mode' Selection : Output mode, PWM 2	
	TIM3->CCMR2 &= ~(3<<0); // CC3S(CC3 channe3)= '0b00' : Output 
	TIM3->CCMR2 |= (1<<3); 	// OC3PE=1: Output Compare 3 preload Enable
	TIM3->CCMR2	|= (7<<4);	// OC3M=0b110: Output compare 3 mode: PWM 2 mode        
	TIM3->CCMR2	|= (1<<7);	// OC3CE=1: Output compare 3 Clear enable
	
	TIM3->CR1	|= (1<<0);	// CEN: Counter TIM3 enable
}

void TIMER2_Init(void)
{
// TIM2_CH4 (PA3) : 450ms 이벤트 발생
// Clock Enable : GPIOA & TIMER2
	RCC->AHB1ENR	|= (1<<0);	// GPIOA Enable
	RCC->APB1ENR 	|= (1<< 0);	// TIMER2 Enable 
    						
// PA3을 출력설정하고 Alternate function(TIM2_CH4)으로 사용 선언 
	GPIOA->MODER 	|= (2<<2*3);	// PA3 Output Alternate function mode					
	GPIOA->OSPEEDR 	|= (3<<2*3);	// PA3 Output speed (100MHz High speed)
	GPIOA->OTYPER	&= ~(1<<3);	// PA3 Output type push-pull (reset state)
	GPIOA->AFR[0]	|= (1 << 12); 	//	AF1					

	// Assign 'Interrupt Period' and 'Output Pulse Period'
	TIM2->PSC = 8400-1;	// Prescaler 84MHz/8400 --> 0.1ms
	TIM2->ARR = 4500-1;	// Auto reload  : 0.1ms * 4500 = 450ms(period)

	// CR1 : Up counting
	TIM2->CR1 &= ~(1<<4);	// DIR=0(Up counter)(reset state)
	TIM2->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled): By one of following events
	TIM2->CR1 &= ~(1<<2);	// URS=0(Update event source Selection): one of following events
	TIM2->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
	TIM2->CR1 &= ~(1<<7);	// ARPE=0(ARR is NOT buffered) (reset state)
	TIM2->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
	TIM2->CR1 &= ~(3<<5); 	// CMS(Center-aligned mode Sel)=00 (Edge-aligned mode) (reset state)
	// Event & Interrup Enable : UI  
	TIM2->EGR |= (1<<0);    // UG: Update generation    
    
	// Define the corresponding pin by 'Output'  
	TIM2->CCER |= (1<< 12);	// CC4E=1: CC4 channel Output Enable
				// OC4(TIM2_CH4) Active: 해당핀을 통해 신호출력
	TIM2->CCER &= ~(1<<13);	// CC4P=0: CC4 channel Output Polarity (OCPolarity_High : OC4으로 반전없이 출력)  

	// 'Mode' Selection : Output mode, toggle  
	TIM2->CCMR2 &= ~(3<<8); // CC4S(CC3 channel) = '0b00' : Output 
	TIM2->CCMR2 &= ~(1<<11); // OC4P=0: Output Compare 4 preload disable
	TIM2->CCMR2 |= (3<<12);	// OC4M=0b011: Output Compare 4 Mode : toggle				

 	TIM2->CCR4 = 2000;	// TIM2 CCR4 TIM2_Pulse	
      
	TIM2->CR1 |= (1<<0);	// CEN: Enable the TIM2 Counter  					
}

void TIMER13_Init(void)
{
// PF8: TIM13_CH1
// PF8을 출력설정하고 Alternate function(TIM13_CH1)으로 사용 선언
	RCC->AHB1ENR	|= (1<<5);	// RCC_AHB1ENR GPIOF Enable : AHB1ENR

	GPIOD->MODER    |= (2<<16);	//  Output Alternate function mode 					
	GPIOD->OSPEEDR 	|= (3<<16);	//  Output speed (100MHz High speed)
	GPIOD->OTYPER	&= ~(1<<8);	//  Output type push-pull (reset state)
	GPIOD->PUPDR    |= (1<<16); //   Pull-up  					
	GPIOD->AFR[1]	|= (9<<0);  // AF9 
 
// Time base 설정
	RCC->APB1ENR |= (1<<7);	//  RCC_APB1ENR TIMER13 Enable

	// Setting CR1 : 0x0000 
	TIM13->CR1 &= ~(1<<4);	// DIR=0(Up counter)(reset state)
	TIM13->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled): By one of following events
	TIM13->CR1 &= ~(1<<2);	// URS=0(Update event source Selection): one of following events
	TIM13->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
	TIM13->CR1 |= (1<<7);	// ARPE=1(ARR is buffered): ARR Preload Enalbe 
	TIM13->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
	TIM13->CR1 &= ~(3<<5); 	// CMS(Center-aligned mode Sel)=00 : Edge-aligned mode(reset state)
	// Setting the Period
	TIM13->PSC = 8400-1; //84Mhz/8400 = 0.1ms
	TIM13->ARR = 1500-1;	 // 0.1ms * 1500 = 150ms
	// Update(Clear) the Counter
	TIM13->EGR |= (1<<0);    // UG: Update generation    
// Output Compare 설정	
	TIM13->CCMR1 &= ~(3<<0); // CC1S(CC1 channel) = '0b00' : Output 
	TIM13->CCMR1 &= ~(1<<2); // OC1FE=0: Output Compare 1 Fast disable 
	TIM13->CCMR1 &= ~(1<<3); // OC1PE=0: Output Compare 1 preload disable
	TIM13->CCMR1 |= (3<<4);	// OC1M=0b011 (Output Compare 1 Mode : toggle)
	// CCER(Capture/Compare Enable Register) : Enable "Channel 1" 
	TIM13->CCER |= (1<<0);	// CC1E=1: CC1 channel Output Enable
      		
	TIM13->CCER &= ~(1<<1);	// CC1P=0: CC1 channel Output Polarity (OCPolarity_High : OC1으로 반전없이 출력)  	
	
	TIM13->CCR1 = 750;	// TIM13 CCR1 TIM13_Pulse          	
	TIM13->DIER |= (1<<1);	// CC1IE: Enable the TIM13 CC1 interrupt

	NVIC->ISER[1] 	|= (1<<44-32);	// Enable Timer13 global Interrupt on NVIC

	TIM13->CR1 |= (1<<0);	// CEN: Enable the TIM13 Counter  					
}

void TIM8_UP_TIM13_IRQHandler(void) {
    if ((TIM13->SR & (1 << 1)) != 0) // CC1 인터럽트 플래그 체크
	{ 
        TIM13->SR &= ~(1 << 1); // CC1 인터럽트 플래그 클리어
		bControl = TRUE;	// 150ms마다 센서 측정
    }
}

void TIMER6_Init(void)
{
// Enable Timer CLK 
	RCC->APB1ENR |= 0x10;	// RCC_APB1ENR TIMER6 Enable

// Setting CR1 : 0x0000 
	TIM6->CR1 &= ~(1<<4);	// DIR=0(Up counter)(reset state)    
	TIM6->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled): By one of following events

	TIM6->CR1 &= ~(1<<2);	// URS=0(Update Request Source  Selection): By one of following events
	TIM6->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
	TIM6->CR1 &= ~(1<<7);	// ARPE=0(ARR is NOT buffered) (reset state)
	TIM6->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
	TIM6->CR1 &= ~(3<<5); 	// CMS(Center-aligned mode Sel)=00 (Edge-aligned mode) (reset state)

// Deciding the Period
	TIM6->PSC = 8400-1;	// Prescaler 84,000,000Hz/8400 = 10,000 Hz (0.1ms)  (1~65536)
	TIM6->ARR = 10000-1;		// Auto reload  0.1ms * 10000 = 1000ms --> 1sec

// Clear the Counter
	TIM6->EGR |= (1<<0);	// UG(Update generation)=1 

// Setting an UI(UEV) Interrupt 
	NVIC->ISER[1] |= (1<<54-32); 	// Enable Timer6 global Interrupt
 	TIM6->DIER |= (1<<0);	// Enable the TIM6 Update interrupt
	TIM6->CR1 |= (1<<0);	// Enable the TIM6 Counter (clock enable)   
}

void TIM6_DAC_IRQHandler(void)  //인터럽트 주기 1sec
{
	TIM6->SR &= ~(1<<0);	// Interrupt flag Clear	

	initial_alarm = Fram_Read(1200); //Fram('시':1200번지)에서 Read
	LCD_SetTextColor(RGB_BLUE);
	LCD_SetFont(&Gulim8);
	LCD_DisplayChar(0,16,ConvertToHexChar(min));
	LCD_DisplayChar(0,17,':');
	LCD_DisplayChar(0,18,ConvertToHexChar(sec));	
	if((ConvertToHexChar(min) == ConvertToHexChar(initial_alarm)) && (ConvertToHexChar(sec) == '0'))
	{
		BEEP();
		DelayMS(200);
		BEEP();
		DelayMS(200);
		BEEP();
		DelayMS(200);
	}		
	sec++;
	if(sec>15) //시간 영역 0:0 ~ F:F로 설정
	{
		min++;
		sec = 0;
		if(min>15) min = 0;
	}
}

void EXTI15_10_IRQHandler(void)
{ 
	if(EXTI->PR & 0x8000) //EXTI15 인터럽트 발생
	{              
		EXTI->PR |= 0x8000;		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)                  
		page++;
		BEEP();
		if(page > 3) page = 1;
		if(page == 1) Dispay_ALARM(); //1번페이지
		else if(page == 2) Dispay_Game(); //2번페이지
		else if(page == 3) Dispay_Thermostat(); //3번페이지
	}
}

void _GPIO_Init(void)
{
	// LED (GPIO G) 
	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
 	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
    
	// SW (GPIO H) 
	RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	// Buzzer (GPIO F) 
	RCC->AHB1ENR	|=  0x00000020; // RCC_AHB1ENR : GPIOF(bit#5) Enable							
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
	GPIOF->OTYPER 	&= ~0x0200;	// GPIOF 9 : Push-pull  	
 	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 
}
/* EXTI 초기 설정 */
void _EXTI_Init(void)
{
	RCC->AHB1ENR 	|= 0x0080;	// RCC_AHB1ENR GPIOH Enable	
	RCC->APB2ENR 	|= 0x4000;	// Enable System Configuration Controller Clock
	
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH PIN8~PIN15 Input mode (reset state)				 
	
    SYSCFG->EXTICR[3] &= ~0x7777;        		
	SYSCFG->EXTICR[3] |= 0x7000;	// EXTI15: PH15
	
    EXTI->RTSR |= 0x8000;		// 15 Rising Trigger Enable

	EXTI->IMR  |= 0x8000;		// EXTI15 인터럽트 mask (Interrupt Enable) 설정
			
    NVIC->ISER[1] |= (1 << 40-32); //EXTI15        
}
// Master mode, Full-duplex, 8bit frame(MSB first), 
// fPCLK/32 Baud rate, Software slave management EN
void SPI1_Init(void)
{
	/*!< Clock Enable  *********************************************************/
	RCC->APB2ENR 	|= (1<<12);	// 0x1000, SPI1 Clock EN
	RCC->AHB1ENR 	|= (1<<0);	// 0x0001, GPIOA Clock EN		
  
	/*!< SPI1 pins configuration ************************************************/
	
	/*!< SPI1 NSS pin(PA8) configuration : GPIO 핀  */
	GPIOA->MODER 	|= (1<<(2*8));	// 0x00010000, PA8 Output mode
	GPIOA->OTYPER 	&= ~(1<<8); 	// 0x0100, push-pull(reset state)
	GPIOA->OSPEEDR 	|= (3<<(2*8));	// 0x00030000, PA8 Output speed (100MHZ) 
	GPIOA->PUPDR 	&= ~(3<<(2*8));	// 0x00030000, NO Pullup Pulldown(reset state)
    
	/*!< SPI1 SCK pin(PA5) configuration : SPI1_SCK */
	GPIOA->MODER 	|= (2<<(2*5)); 	// 0x00000800, PA5 Alternate function mode
	GPIOA->OTYPER 	&= ~(1<<5); 	// 0020, PA5 Output type push-pull (reset state)
	GPIOA->OSPEEDR 	|= (3<<(2*5));	// 0x00000C00, PA5 Output speed (100MHz)
	GPIOA->PUPDR 	|= (2<<(2*5)); 	// 0x00000800, PA5 Pull-down
	GPIOA->AFR[0] 	|= (5<<(4*5));	// 0x00500000, Connect PA5 to AF5(SPI1)
    
	/*!< SPI1 MOSI pin(PA7) configuration : SPI1_MOSI */    
	GPIOA->MODER 	|= (2<<(2*7));	// 0x00008000, PA7 Alternate function mode
	GPIOA->OTYPER	&= ~(1<<7);	// 0x0080, PA7 Output type push-pull (reset state)
	GPIOA->OSPEEDR 	|= (3<<(2*7));	// 0x0000C000, PA7 Output speed (100MHz)
	GPIOA->PUPDR 	|= (2<<(2*7)); 	// 0x00008000, PA7 Pull-down
	GPIOA->AFR[0] 	|= (5<<(4*7));	// 0x50000000, Connect PA7 to AF5(SPI1)
    
	/*!< SPI1 MISO pin(PA6) configuration : SPI1_MISO */
	GPIOA->MODER 	|= (2<<(2*6));	// 0x00002000, PA6 Alternate function mode
	GPIOA->OTYPER 	&= ~(1<<6);	// 0x0040, PA6 Output type push-pull (reset state)
	GPIOA->OSPEEDR 	|= (3<<(2*6));	// 0x00003000, PA6 Output speed (100MHz)
	GPIOA->PUPDR 	|= (2<<(2*6));	// 0x00002000, PA6 Pull-down
	GPIOA->AFR[0] 	|= (5<<(4*6));	// 0x05000000, Connect PA6 to AF5(SPI1)

	// Init SPI1 Registers 
	SPI1->CR1 |= (1<<2);	// MSTR(Master selection)=1, Master mode
	SPI1->CR1 &= ~(1<<15);	// SPI_Direction_2 Lines_FullDuplex
	SPI1->CR1 &= ~(1<<11);	// SPI_DataSize_8bit
	SPI1->CR1 |= (1<<9);  	// SSM(Software slave management)=1, 
				// NSS 핀 상태가 코딩에 의해 결정
	SPI1->CR1 |= (1<<8);	// SSI(Internal_slave_select)=1,
				// 현재 MCU가 Master이므로 NSS 상태는 'High' 
	SPI1->CR1 &= ~(1<<7);	// LSBFirst=0, MSB transmitted first    
	SPI1->CR1 |= (4<<3);	// BR(BaudRate)=0b100, fPCLK/32 (84MHz/32 = 2.625MHz)
	SPI1->CR1 |= (1<<1);	// CPOL(Clock polarity)=1, CK is 'High' when idle
	SPI1->CR1 |= (1<<0);	// CPHA(Clock phase)=1, 두 번째 edge 에서 데이터가 샘플링
 
	SPI1->CR1 |= (1<<6);	// SPE=1, SPI1 Enable 
}

void Display_Process(int16 *pBuf)
{	
	UINT16 G_VALUE_X, G_VALUE_Y;
	int speed_x, speed_y;

	// X 축 가속도 표시		
	if (pBuf[0] < 0)  //음수
	{
		G_VALUE_X = abs(pBuf[0]);
		LCD_SetTextColor(RGB_RED);
		LCD_SetFont(&Gulim7);
		LCD_DisplayChar(2,22,'-'); // g 부호 표시
	}
	else				// 양수
	{
		G_VALUE_X = pBuf[0];
		LCD_SetTextColor(RGB_RED);
		LCD_SetFont(&Gulim7);
		LCD_DisplayChar(2,22,'+'); // g 부호 표시
	}	
	G_VALUE_X = 100 * G_VALUE_X / 0x4009; // 가속도 --> g 변환
	// 중력가속도 범위 제한
	if (G_VALUE_X > 100) G_VALUE_X = 100;    // -1.0g ~ +1.0g	
	LCD_SetTextColor(RGB_RED);
	LCD_SetFont(&Gulim7);
	LCD_DisplayChar(2,23, G_VALUE_X/100 +0x30);
	LCD_DisplayChar(2,24,'.');
	LCD_DisplayChar(2,25, G_VALUE_X%100/10 +0x30);
	LCD_DisplayChar(2,26, G_VALUE_X	%10 +0x30);
	

	// Y 축 가속도 표시	
	if (pBuf[1] < 0)  //음수
	{
		G_VALUE_Y = abs(pBuf[1]);
		LCD_SetTextColor(RGB_RED);
		LCD_SetFont(&Gulim7);		
		LCD_DisplayChar(3,22,'-'); // g 부호 표시
	}
	else				// 양수
	{
		G_VALUE_Y = pBuf[1];
		LCD_SetTextColor(RGB_RED);
		LCD_SetFont(&Gulim7);
		LCD_DisplayChar(3,22,'+'); // g 부호 표시
	}
	G_VALUE_Y = 100 * G_VALUE_Y / 0x4009; 
	// 중력가속도 범위 제한
	if (G_VALUE_Y > 100) G_VALUE_Y = 100;    // -1.0g ~ +1.0g	
	LCD_SetTextColor(RGB_RED);
	LCD_SetFont(&Gulim7);
	LCD_DisplayChar(3,23, G_VALUE_Y/100 +0x30);
	LCD_DisplayChar(3,24,'.');
	LCD_DisplayChar(3,25, G_VALUE_Y%100/10 +0x30);
	LCD_DisplayChar(3,26, G_VALUE_Y%10 +0x30);	

	// X축 기울기 값에 따른 속도 계산 (X축 가속도에 비례)
    speed_x = (G_VALUE_X / 100.0) * SPEED;  // X축 속도 계산

    // Y축 기울기 값에 따른 속도 계산 (Y축 가속도에 비례)    
    speed_y = (G_VALUE_Y / 100.0) * SPEED;  // Y축 속도 계산
    
    // 공의 위치 업데이트
	int prev_ball_x = ball_x;
    int prev_ball_y = ball_y;

    if (pBuf[0] > 0 && speed_x != 0) ball_x -= speed_x;  // 오른쪽 이동
    else if (pBuf[0] < 0 && speed_x != 0) ball_x += speed_x;  // 왼쪽 이동

    if (pBuf[1] > 0 && speed_y != 0) ball_y -= speed_y;  // 아래로 이동
    else if (pBuf[1] < 0 && speed_y != 0) ball_y += speed_y;  // 위로 이동

	// 화면 경계 제한
    if (ball_x < 11) ball_x = 11;
    if (ball_x > 103) ball_x = 103;
    if (ball_y < 21) ball_y = 21;
    if (ball_y > 113) ball_y = 113;

	LCD_SetPenColor(RGB_WHITE);
    LCD_DrawRectangle(prev_ball_x,prev_ball_y,6,6);
	LCD_SetPenColor(RGB_RED);
    LCD_DrawRectangle(ball_x,ball_y,6,6);

}

void USART1_Init(void)
{
	// USART1 : TX(PA9)
	RCC->AHB1ENR	|= (1<<0);	// RCC_AHB1ENR GPIOA Enable
	GPIOA->MODER	|= (2<<2*9);	// GPIOB PIN9 Output Alternate function mode					
	GPIOA->OSPEEDR	|= (3<<2*9);	// GPIOB PIN9 Output speed (100MHz Very High speed)
	GPIOA->AFR[1]	|= (7<<4);	// Connect GPIOA pin9 to AF7(USART1)
    
	// USART1 : RX(PA10)
	GPIOA->MODER 	|= (2<<2*10);	// GPIOA PIN10 Output Alternate function mode
	GPIOA->OSPEEDR	|= (3<<2*10);	// GPIOA PIN10 Output speed (100MHz Very High speed
	GPIOA->AFR[1]	|= (7<<8);	// Connect GPIOA pin10 to AF7(USART1)

	RCC->APB2ENR	|= (1<<4);	// RCC_APB2ENR USART1 Enable
    
	USART_BRR_Configuration(9600); // USART Baud rate Configuration --> 9600으로 설정    	

	USART1->CR1	|= (1<<12);	// USART_WordLength 9 Data bit
	USART1->CR1	|= (1<<10);	// USART_Parity
    USART1->CR1	&= ~(1<<9);	// Even_Parity

	USART1->CR1	|= (1<<2);	// 0x0004, USART_Mode_RX Enable
	USART1->CR1	|= (1<<3);	// 0x0008, USART_Mode_Tx Enable
	USART1->CR2	&= ~(3<<12);	// 0b00, USART_StopBits_1
	USART1->CR3	= 0x0000;	// No HardwareFlowControl, No DMA
    
	USART1->CR1	|= (1<<5);	// 0x0020, RXNE interrupt Enable
	USART1->CR1	&= ~(1<<7); // 0x0080, TXE interrupt Disable 

	NVIC->ISER[1]	|= (1<<(37-32));// Enable Interrupt USART1 (NVIC 37)
	USART1->CR1 	|= (1<<13);	//  0x2000, USART1 Enable
}

void SerialSendChar(uint8_t Ch) // 
{
	while((USART1->SR & USART_SR_TXE) == RESET); // USART_SR_TXE=(1<<7), 

	USART1->DR = (Ch & 0x01FF);	
}

void SerialSendString(char *str) 
{
	while (*str != '\0') 
	{
		SerialSendChar(*str);	
		str++; 			
	}
}

// Baud rate  
void USART_BRR_Configuration(uint32_t USART_BaudRate)
{ 
	uint32_t tmpreg = 0x00;
	uint32_t APB2clock = 84000000;	//PCLK2_Frequency
	uint32_t integerdivider = 0x00;
	uint32_t fractionaldivider = 0x00;

	// Determine the integer part 
	if ((USART1->CR1 & USART_CR1_OVER8) != 0) // USART_CR1_OVER8=(1<<15)
	{                                         // USART1->CR1.OVER8 = 1 (8 oversampling)
		// Computing 'Integer part' when the oversampling mode is 8 Samples 
		integerdivider = ((25 * APB2clock) / (2 * USART_BaudRate));    
	}
	else  // USART1->CR1.OVER8 = 0 (16 oversampling)
	{	// Computing 'Integer part' when the oversampling mode is 16 Samples 
		integerdivider = ((25 * APB2clock) / (4 * USART_BaudRate));    
	}
	tmpreg = (integerdivider / 100) << 4;
  
	// Determine the fractional part 
	fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

	// Implement the fractional part in the register 
	if ((USART1->CR1 & USART_CR1_OVER8) != 0)	// 8 oversampling
	{
		tmpreg |= (((fractionaldivider * 8) + 50) / 100) & (0x07);
	}
	else 			// 16 oversampling
	{
		tmpreg |= (((fractionaldivider * 16) + 50) / 100) & (0x0F);
	}

	// Write to USART BRR register
	USART1->BRR = (uint16_t)tmpreg;
}

void DelayMS(unsigned short wMS)
{	register unsigned short i;
	for (i=0; i<wMS; i++)
		DelayUS(1000);  // 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{	volatile int Dly = (int)wUS*17;
	for(; Dly; Dly--);
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
		{		DelayMS(10);
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

void BEEP(void)			// Beep for 20 ms 
{ 	GPIOF->ODR |= (1<<9);	// PF9 'H' Buzzer on
	DelayMS(20);		// Delay 20 ms
	GPIOF->ODR &= ~(1<<9);	// PF9 'L' Buzzer off
}