//////////////////////////////////////////////////////////////////////
//TP2: Remote-controlled Robot
//제출자: 2020132013 김태환
//제출일: 2024.12.12
//과제 개요:
//? 두개의 원격제어기에서 USART통신으로 제어하는 이동로봇구현
//  - 원격제어기1: PC(MFC 프로그램)
//  - 원격제어기2: Mobilephone(LightBlue 프로그램)
//  - 이동로봇: F407 보드(user control program)
// ? 원격제어기의 역할: 이동로봇에 이동/정지/회전 명령을전송하고, 이동로봇으로부터 로봇상태(이동/정지/회전 상태)를 수신하여화면에표시함. 
// ? 이동로봇의 역할: 원격제어기로부터 이동/정지/회전 명령을수신하고실행(LCD, LED, Buzzer 작동)함. 로봇상태(이동/정지/회전상태)를원격제어기에 송신함.
//////////////////////////////////////////////////////////////////////
#include "stm32f4xx.h"
#include "GLCD.h"
#include "default.h"
#include "Util.h"

#define SW0_PUSH        0xFE00  //PH8
#define SW1_PUSH        0xFD00  //PH9
#define SW2_PUSH        0xFB00  //PH10
#define SW3_PUSH        0xF700  //PH11
#define SW4_PUSH        0xEF00  //PH12
#define SW5_PUSH        0xDF00  //PH13
#define SW6_PUSH        0xBF00  //PH14
#define SW7_PUSH        0x7F00  //PH15

void USART_BRR_Configuration_1(uint32_t USART_BaudRate); //usart1 보레이트 설정
void USART_BRR_Configuration_4(uint32_t USART_BaudRate); //uart4 보레이트 설정
void SerialPutChar(uint8_t c); //usart1 송신
void Serial_PutString(char *s);//usart1 송신
void SerialSendChar(uint8_t c);//uart4 송신
void SerialSendString(char *s);//uart4 송신
uint16_t KEY_Scan(void);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);
void BEEP(void);
//-------------------------설정-------------------------------------
void _GPIO_Init(void);
void USART1_Init(void);
void DisplayTitle(void);
void DMAInit(void);
void UART4_Init(void);
void _ADC_Init(void);
void _EXTI_Init(void);
void TIMER7_Init(void);
//-----------------------변수 선언-------------------------------
uint8_t USART_value[6]; //usart1으로 수신 받은 값 저장
#define BUFFER_SIZE 1 //버퍼 사이즈 설정
int robot_state = 0; //0이면 정지상태 1이면 이동
char pc_hex[3]; // 16진수 문자열 저장 (2자리 + 종료 문자)
char previous_hex[3] = ""; // 이전 16진수 값을 저장
int previous_robot_state = -1; // 이전 로봇 상태를 저장 (-1로 초기화)
unsigned short ADC_Value, Voltage; // ADC값 및 변환 된 전압값
volatile int r_state = 0; //로봇 상태문자를 위한 변수(0: stop, 1: move, 2: right, 3: straight, 4: left)

int main(void)
{
	_GPIO_Init();
	_EXTI_Init();	
	_ADC_Init();
	USART1_Init();
	DMAInit();
	UART4_Init();
	LCD_Init();	// LCD 구동 함수
	DelayMS(1000);	// LCD구동 딜레이
    TIMER7_Init();
	GPIOG->ODR &= 0x00;// LED0~7 Off 
          
	//LCD 초기화면구동 함수
	DisplayTitle();
	
	GPIOB->ODR &= ~0x0200;    
	while(1)
	{
		// USART_value에서 데이터를 읽어와 16진수 문자열로 변환
		snprintf(pc_hex, sizeof(pc_hex), "%02X", USART_value[0]);

		// 상태 변화 감지 로직
		if (strcmp(pc_hex, previous_hex) != 0) // 현재 값이 이전 값과 다를 때
		{
			// 이전 상태 업데이트
			strncpy(previous_hex, pc_hex, sizeof(previous_hex));
			if (strcmp(pc_hex, "80") == 0)
			{
				GPIOG->ODR &= 0x00; // LED0~7 Off
				GPIOG->ODR |= 0x80; // LED7 On
				robot_state = 1; //move 상태
				BEEP(); 
				DelayMS(100);
				BEEP();
				LCD_DisplayText(3, 7, "Move    ");
				LCD_DisplayText(1, 9, pc_hex);
				r_state = 1; //move상태 usart1,uart4 송신 확인용
				
			}
			else if (strcmp(pc_hex, "40") == 0)
			{
				GPIOG->ODR &= 0x00; // LED0~7 Off
				robot_state = 0; //stop상태
				BEEP(); 
				DelayMS(100);
				BEEP();
				DelayMS(100);
				BEEP();
				LCD_DisplayText(3, 7, "Stop    ");
				LCD_DisplayText(1, 9, pc_hex);
				r_state = 0; //stop 상태 usart1,uart4 송신 확인용
			}
			// 로봇 이동 상태일 때
			if (robot_state == 1)
			{
				if (strcmp(pc_hex, "04") == 0)
				{
					GPIOG->ODR &= 0x00; // LED0~7 Off
					GPIOG->ODR |= 0x04; // LED2 On
					BEEP(); 
					LCD_DisplayText(4, 5, "Right   ");
					LCD_DisplayText(1, 9, pc_hex);
					r_state = 2; //right 상태 usart1,uart4 송신 확인용
				}
				else if (strcmp(pc_hex, "02") == 0)
				{
					GPIOG->ODR &= 0x00; // LED0~7 Off
					GPIOG->ODR |= 0x02; // LED1 On
					BEEP(); 
					LCD_DisplayText(4, 5, "Straight");
					LCD_DisplayText(1, 9, pc_hex);					
					r_state = 3; //straight 상태 usart1,uart4 송신 확인용
				}
				else if (strcmp(pc_hex, "01") == 0)
				{
					GPIOG->ODR &= 0x00; // LED0~7 Off
					GPIOG->ODR |= 0x01; // LED0 On
					BEEP(); 
					LCD_DisplayText(4, 5, "Left    ");
					LCD_DisplayText(1, 9, pc_hex);
					r_state = 4;//left 상태 usart1,uart4 송신 확인용
				}
			}
		}
	}
}
void TIM7_IRQHandler(void)  	// 2sec Interrupt
{	    
	TIM7->SR &= ~(1<<0);	// Interrupt flag Clear	
   	if(r_state == 0) //stop상태이면
	{
		Serial_PutString("Stop "); //usart1 송신
		SerialSendString("Stop ");//uart4 송신
	}
	else if(r_state == 1) //move 상태일 때
	{
		Serial_PutString("Move ");//usart1 송신
		SerialSendString("Move ");//uart4 송신
	}
	else if(r_state == 2) //right 상태일때
	{
		Serial_PutString("Right ");//usart1 송신
		SerialSendString("Right ");//uart4 송신
	}
	else if(r_state == 3)//straight 상태일때
	{
		Serial_PutString("Straight ");//usart1 송신
		SerialSendString("Straight ");//uart4 송신
	}
	else if(r_state == 4)//left 상태일때
	{
		Serial_PutString("Left ");//usart1 송신
		SerialSendString("Left ");//uart4 송신
	}
}
void UART4_IRQHandler(void)	
{       
	if ( (UART4->SR & USART_SR_RXNE) ) // USART_SR_RXNE= 1? RX Buffer Full?
    // #define  USART_SR_RXNE ((uint16_t)0x0020)    //  Read Data Register Not Empty(FULL)     
	{
		char ch;
		ch = UART4->DR;	// 수신된 문자 저장
		if(ch == 0x80)
		{
			LCD_DisplayText(2,7,"0x80");
			GPIOG->ODR &= 0x00; // LED0~7 Off
			GPIOG->ODR |= 0x80; // LED7 On
			robot_state = 1;//move상태
			BEEP(); 
			DelayMS(100);
			BEEP();
			LCD_DisplayText(3, 7, "Move    ");			
			r_state = 1;//move 상태 usart1,uart4 송신 확인용
		}
		else if(ch == 0x40)
		{
			LCD_DisplayText(2,7,"0x40");
			GPIOG->ODR &= 0x00; // LED0~7 Off
			robot_state = 0;//stop상태
			BEEP(); 
			DelayMS(100);
			BEEP();
			DelayMS(100);
			BEEP();
			LCD_DisplayText(3, 7, "Stop    ");			
			r_state = 0;//stop 상태 usart1,uart4 송신 확인용
		}
		if(robot_state == 1)
		{
			if(ch == 0x04)
			{
				LCD_DisplayText(2,7,"0x04");
				GPIOG->ODR &= 0x00; // LED0~7 Off
				GPIOG->ODR |= 0x04; // LED2 On
				BEEP(); 
				LCD_DisplayText(4, 5, "Right   ");			
				r_state = 2;//right 상태 usart1,uart4 송신 확인용
			}
			else if(ch == 0x02)
			{
				LCD_DisplayText(2,7,"0x02");
				GPIOG->ODR &= 0x00; // LED0~7 Off
				GPIOG->ODR |= 0x02; // LED1 On
				BEEP(); 
				LCD_DisplayText(4, 5, "Straight");			
				r_state = 3;//straight 상태 usart1,uart4 송신 확인용
			}
			else if(ch == 0x01)
			{
				LCD_DisplayText(2,7,"0x01");
				GPIOG->ODR &= 0x00; // LED0~7 Off
				GPIOG->ODR |= 0x01; // LED0 On
				BEEP(); 
				LCD_DisplayText(4, 5, "Left    ");			
				r_state = 4;//left 상태 usart1,uart4 송신 확인용
			}
		}
	} 
        // DR 을 읽으면 SR.RXNE bit(flag bit)는 clear 된다. 즉 clear 할 필요없음 
}
void ADC_IRQHandler(void)
{
	char voltage_str[10];  // Voltage 값을 문자열로 저장할 버퍼

	ADC3->SR &= ~(1<<1);		// EOC flag clear
	ADC_Value = ADC3->DR;		// Reading ADC result     
	Voltage = ADC_Value * (3.3 * 100) / 255;   // 3.3 : 255 =  Volatge : ADC_Value
	int voltage_int = Voltage / 100;         // 정수 부분
	int voltage_dec = Voltage%100/10;        // 소수 첫째 자리                                                     
	LCD_DisplayChar(5,7, voltage_int + 0x30);
	LCD_DisplayChar(5,8,'.');
	LCD_DisplayChar(5,9,voltage_dec + 0x30);

	// Voltage 값을 문자열로 변환
    snprintf(voltage_str, sizeof(voltage_str), "%d.%d ", voltage_int, voltage_dec);
	Serial_PutString(voltage_str); // PC로 전압값 전송
}
void _ADC_Init(void)
{     	// ADC3: PA1(pin 41)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;	// ENABLE GPIOA CLK (stm32f4xx.h 참조)
	GPIOA->MODER |= (3<<2*1);		// CONFIG GPIOA PIN1(PA1) TO ANALOG IN MODE
						
	RCC->APB2ENR |= RCC_APB2ENR_ADC3EN;	//ENABLE ADC3 CLK (stm32f4xx.h 참조)

	ADC->CCR &= ~0X0000001F;	// MULTI[4:0]: ADC_Mode_Independent
	ADC->CCR |= 0x00010000;		// ADCPRE: ADC_Prescaler_Div4 (ADC MAX Clock 36MHz, 84Mhz(APB2)/4 = 21MHz)
	ADC->CCR &= ~0x0000C000;	// DMA: Disable
	ADC->CCR |= 0x00000F00;		// ADC_TwoSamplingDelay_20Cycles
        
	ADC3->CR1 |= (2<<24);		// 8bit Resolution
	ADC3->CR1 &= ~(1<<8);		// SCAN=0 : ADC_ScanCovMode Disable
	ADC3->CR1 |=  (1<<5);		// EOCIE=1: Interrupt enable for EOC

	ADC3->CR2 &= ~(1<<1);		// CONT=0: ADC_Continuous ConvMode Disable        
	ADC3->CR2 |=  (2<<28);		// EXTEN[1:0]: ADC_ExternalTrigConvEdge_Enable(Falling Edge)
	ADC3->CR2 |= (0x0F<<24);	// EXTSEL[3:0]: ADC_ExternalTrig (EXTI11)        
	ADC3->CR2 &= ~(1<<11);		// ALIGN: ADC_DataAlign_Right   
	ADC3->CR2 &= ~(1<<10);		// EOCS=0: The EOC bit is set at the end of each sequence of regular conversions
	ADC3->SQR1 &= ~(0xF<<20);	// L[3:0]=0b0000: ADC Regular channel sequece length 					
	ADC3->SQR3 |= (1<<0);		// SQ1[4:0]=0b0001 : CH1
 
	ADC3->SMPR2 |= (0x7<<(3*1));	// ADC3_CH1 Sample TIme_480Cycles (3*Channel_1)

	NVIC->ISER[0] |= (1<<18);		// Enable ADC global Interrupt

	ADC3->CR2 |= (1<<0);		// ADON: ADC ON
}
void _GPIO_Init(void)
{
	// LED (GPIO G) 설정
    RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
 	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
    
	// SW (GPIO H) 설정 
	RCC->AHB1ENR	|=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	// Buzzer (GPIO F) 설정 
	RCC->AHB1ENR	|=  0x00000020;	// RCC_AHB1ENR : GPIOF(bit#5) Enable							
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
	GPIOF->OTYPER 	&= ~0x0200;	// GPIOF 9 : Push-pull  	
	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 
}	
void _EXTI_Init(void)    //EXTI11(PH11,SW3)
{
	RCC->AHB1ENR    |= (1<<7); 	// 0x80, RCC_AHB1ENR GPIOH Enable
	RCC->APB2ENR 	|= (1<<14);	// 0x4000, Enable System Configuration Controller Clock
	
	SYSCFG->EXTICR[2] |= (7<<12);	// 0x7000, EXTI11에 대한 소스 입력은 GPIOH로 설정 (EXTICR3) (reset value: 0x0000)	
	
	EXTI->FTSR |= (1<<11);		// 0x000800, Falling Trigger Enable  (EXTI11:PH11)
}
void USART1_Init(void)
{
	RCC->APB2ENR |=	0x0010;	// RCC_APB2ENR USART1 Enable
  
	//TX(PA9)
	RCC->AHB1ENR |= 0x01;	// RCC_AHB1ENR GPIOA Enable
	GPIOA->MODER |= 0x00080000;// GPIOA PIN9 Output Alternate function mode					
	GPIOA->OSPEEDR |= 0x000C0000;// GPIOA PIN9 Output speed (100MHz Very High speed)
	GPIOA->OTYPER |= 0x00000000;// GPIOA PIN9 Output type push-pull (reset state)
	GPIOA->PUPDR |= 0x00040000;// GPIOA PIN9 Pull-up
	GPIOA->AFR[1] |= 0x70;	//Connect GPIOA pin9 to AF7(USART1)
    
	//RX(PA10)
	RCC->AHB1ENR |= 0x01;	// RCC_AHB1ENR GPIOA Enable    				   
	GPIOA->MODER |= 0x200000;// GPIOA PIN10 Output Alternate function mode
	GPIOA->OSPEEDR |= 0x00300000;// GPIOA PIN10 Output speed (100MHz Very High speed
	GPIOA->AFR[1]	|= 0x700;//Connect GPIOA pin10 to AF7(USART1)
    
	USART_BRR_Configuration_1(9600);	// USART Baudrate Configuration
    
	USART1->CR1 |= 0x0000;	// USART_WordLength 8 Data bit
	USART1->CR1 |= 0x0000;	// USART_Parity_No
	USART1->CR1 |= 0x0004;	// USART_Mode_RX Enable
	USART1->CR1 |= 0x0008;	// USART_Mode_Tx Enable
	USART1->CR2 |= 0x0000;	// USART_StopBits_1
	USART1->CR3 |= 0x0000;	// USART_HardwareFlowControl_None;
	USART1->CR3 |= (1<<6);	// DMA enable receiver
    
	USART1->CR1 |=	0x2000;	// USART1 Enable   
}
void UART4_Init(void)
{
	// UART4 : TX(PC10)
	RCC->AHB1ENR	|= (1<<2);	// RCC_AHB1ENR GPIOC Enable
	GPIOC->MODER	|= (2<<2*10);	// GPIOC PIN10 Output Alternate function mode					
	GPIOC->OSPEEDR	|= (3<<2*10);	// GPIOC PIN10 Output speed (100MHz Very High speed)
	GPIOC->AFR[1]	|= (8<<4*(10-8));// Connect GPIOC pin10 to AF8(UART4)
    
	// UART4 : RX(PC11)
	GPIOC->MODER 	|= (2<<2*11);	// GPIOC PIN11 Output Alternate function mode
	GPIOC->OSPEEDR	|= (3<<2*11);	// GPIOC PIN11 Output speed (100MHz Very High speed
	GPIOC->AFR[1]	|= (8<<4*(11-8));// Connect GPIOC pin11 to AF8(UART4)

	// BT RESET (PC13) : GPIO
	GPIOC->MODER 	|= (1<<2*13);	// GPIOC PIN13 Output mode
	GPIOC->OSPEEDR  |= (3<<2*13);
	GPIOC->ODR	|= (1<<13);	// BT Reset
 
	RCC->APB1ENR	|= (1<<19);	// RCC_APB1ENR UART4 Enable
    
	USART_BRR_Configuration_4(9600); // USART Baud rate Configuration
    
	UART4->CR1	&= ~(1<<12);	// UART_WordLength 8 Data bit
	UART4->CR1	&= ~(1<<10);	// NO USART_Parity

	UART4->CR1	|= (1<<2);	// 0x0004, USART_Mode_RX Enable
	UART4->CR1	|= (1<<3);	// 0x0008, USART_Mode_Tx Enable
	UART4->CR2	&= ~(3<<12);	// 0b00, USART_StopBits_1
	UART4->CR3	= 0x0000;	// No HardwareFlowControl, No DMA
    
	UART4->CR1 	|= (1<<5);	// 0x0020, RXNE interrupt Enable
	NVIC->ISER[1]	|= (1<<(52-32));// Enable Interrupt USART4 (NVIC 52번)
	UART4->CR1 	|= (1<<13);	//  0x2000, UART4 Enable
}
void DMAInit(void)
{
	// DMA2 Stream2 channe4 configuration *************************************
	RCC->AHB1ENR |= (1<<22);			//DMA2 clock enable
	DMA2_Stream2->CR   |= (4<<25);	//DMA2 Stream2 channel 4 selected  100 
	DMA2_Stream2->PAR |= (uint32_t)&USART1->DR;    //Peripheral address 
	DMA2_Stream2->M0AR |= (uint32_t)USART_value;  //Memory address 
	DMA2_Stream2->NDTR = 1;		//DMA_BufferSize = 1
	  
	DMA2_Stream2->CR &= ~(3<<6);	//Data transfer direction : Peripheral-to-memory
	DMA2_Stream2->CR &= ~(1<<9);	//Peripheral increment mode - Peripheral address pointer is fixed
	DMA2_Stream2->CR &= ~(1<<10);	//Memory increment mode - Memory address pointer is fixed
	  
	DMA2_Stream2->CR &= ~(3<<11);	//Peripheral data size -byte(8bit) 
	DMA2_Stream2->CR &= ~(3<<13);	//Memory data size - byte(8bit) peripheral data size와 동일
	DMA2_Stream2->CR |= (1<<8);	//Circular mode enabled   
	DMA2_Stream2->CR |= (2<<16);	//Priority level - High

	DMA2_Stream2->FCR &= ~(1<<2);	//DMA_FIFO_direct mode enabled

	DMA2_Stream2->CR &= ~(3<<23);	//Memory burst transfer configuration - single transfer
	DMA2_Stream2->CR &= ~(3<<21);	//Peripheral burst transfer configuration - single transfer  
	DMA2_Stream2->CR |= (1<<0);	//DMA2_Stream2 enabled

}
void TIMER7_Init(void)
{
// Enable Timer CLK 
	RCC->APB1ENR |= 0x20;	// RCC_APB1ENR TIMER7 Enable

// Setting CR1 : 0x0000 
	TIM7->CR1 &= ~(1<<4);	// DIR=0(Up counter)(reset state)    
	TIM7->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled): By one of following events

	TIM7->CR1 &= ~(1<<2);	// URS=0(Update Request Source  Selection): By one of following events
	TIM7->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
	TIM7->CR1 &= ~(1<<7);	// ARPE=0(ARR is NOT buffered) (reset state)
	TIM7->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
	TIM7->CR1 &= ~(3<<5); 	// CMS(Center-aligned mode Sel)=00 (Edge-aligned mode) (reset state)

// Deciding the Period
	TIM7->PSC = 8400-1;	// Prescaler 84,000,000Hz/8400 = 10,000 Hz (0.1ms)  (1~65536)
	TIM7->ARR = 20000-1;		// Auto reload  0.1ms * 20000 = 2000ms --> 2sec

// Clear the Counter
	TIM7->EGR |= (1<<0);	// UG(Update generation)=1 

// Setting an UI(UEV) Interrupt 
	NVIC->ISER[1] |= (1<<55-32); 	// Enable TIMER7 global Interrupt
 	TIM7->DIER |= (1<<0);	// Enable the TIM7 Update interrupt
	TIM7->CR1 |= (1<<0);	// Enable the TIM7 Counter (clock enable)
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

void DisplayTitle(void)
{	LCD_Clear(RGB_GRAY);
	LCD_SetFont(&Gulim8);
	LCD_SetBackColor(RGB_YELLOW);	//배경색
	LCD_SetTextColor(RGB_BLACK);	//글자색
	LCD_DisplayText(0,0,"RC Robot(KTH)");
	LCD_SetBackColor(RGB_GRAY);	//글자배경색
	LCD_DisplayText(1,0,"Rx(PC):");
	LCD_DisplayText(2,0,"Rx(MP):");
	LCD_DisplayText(3,0,"Motion:");
	LCD_DisplayText(4,0,"Turn:");
	LCD_DisplayText(5,0,"ADC(V):");
	LCD_SetTextColor(RGB_BLUE);	//글자색
	LCD_DisplayText(1,7,"0x40");
	LCD_DisplayText(2,7,"0x40");
	LCD_DisplayText(3,7,"Stop");
	LCD_DisplayText(4,5,"Straight");
	LCD_DisplayText(5,7,"0.0");
}

void SerialPutChar(uint8_t c) // 1문자 보내기 함수
{
	while((USART1->SR & 0x0080) == RESET); 	// 송신 가능한 상태까지 대기

	USART1->DR = (c & (uint16_t)0x01FF);	// 전송
}


void Serial_PutString(char *s) // 여러문자 보내기 함수
{
	while (*s != '\0') // 종결문자가 나오기 전까지 구동. 종결문자가 나온후에도 구동시 메모리 오류 발생가능성 있음.
	{
		SerialPutChar(*s); // 포인터가 가르키는 곳의 데이터를 송신
		s++; // 포인터 수치 증가
	}
}

void SerialSendChar(uint8_t Ch) // 1문자 보내기 함수
{
	// USART_SR_TXE(1<<7)=0?, TX Buffer NOT Empty? 
	// TX buffer Empty되지 않으면 계속 대기(송신 가능한 상태까지 대기)
    	while((UART4->SR & USART_SR_TXE) == RESET); 
	UART4->DR = (Ch & 0x01FF);	// 전송 (최대 9bit 이므로 0x01FF과 masking)
}

void SerialSendString(char *str) // 여러문자 보내기 함수
{
	while (*str != '\0') // 종결문자가 나오기 전까지 구동, 종결문자가 나온후에도 구동시 메모리 오류 발생가능성 있음.
	{
		SerialSendChar(*str);	// 포인터가 가르키는 곳의 데이터를 송신
		str++; 			// 포인터 수치 증가
	}
}

void USART_BRR_Configuration_1(uint32_t USART_BaudRate)
{ 
	uint32_t tmpreg = 0x00;
	uint32_t apbclock = 84000000;	//PCLK2_Frequency
	uint32_t integerdivider = 0x00;
	uint32_t fractionaldivider = 0x00;

	/* Determine the integer part */
	if ((USART1->CR1 & 0x8000) != 0)
	{
		/* Integer part computing in case Oversampling mode is 8 Samples */
		integerdivider = ((25 * apbclock) / (2 * (USART_BaudRate)));    
	}
	else /* if ((USARTx->CR1 & USART_CR1_OVER8) == 0) */
	{
		/* Integer part computing in case Oversampling mode is 16 Samples */
		integerdivider = ((25 * apbclock) / (4 * (USART_BaudRate)));    
	}
	tmpreg = (integerdivider / 100) << 4;
  
	/* Determine the fractional part */
	fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

	/* Implement the fractional part in the register */
	if ((USART1->CR1 & USART_CR1_OVER8) != 0)
	{
		tmpreg |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07);
	}
	else /* if ((USARTx->CR1 & USART_CR1_OVER8) == 0) */
	{
		tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
	}
	/* Write to USART BRR register */
	USART1->BRR = (uint16_t)tmpreg;
    
}

// Baud rate 설정
void USART_BRR_Configuration_4(uint32_t USART_BaudRate)
{ 
	uint32_t tmpreg = 0x00;
	uint32_t APB1clock = 42000000;	//PCLK2_Frequency
	uint32_t integerdivider = 0x00;
	uint32_t fractionaldivider = 0x00;

	// Find the integer part 
	if ((UART4->CR1 & USART_CR1_OVER8) != 0) // USART_CR1_OVER8=(1<<15)
        //  #define  USART_CR1_OVER8 ((uint16_t)0x8000) // USART Oversampling by 8 enable   
	{       // UART4->CR1.OVER8 = 1 (8 oversampling)
		// Computing 'Integer part' when the oversampling mode is 8 Samples 
		integerdivider = ((25 * APB1clock) / (2 * USART_BaudRate));  // 공식에 100을 곱한 곳임(소수점 두번째자리까지 유지하기 위함)  
	}
	else  // USART1->CR1.OVER8 = 0 (16 oversampling)
	{	// Computing 'Integer part' when the oversampling mode is 16 Samples 
		integerdivider = ((25 * APB1clock) / (4 * USART_BaudRate));  // 공식에 100을 곱한 곳임(소수점 두번째자리까지 유지하기 위함)    
	}
	tmpreg = (integerdivider / 100) << 4;
  
	// Find the fractional part 
	fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

	// Implement the fractional part in the register 
	if ((UART4->CR1 & USART_CR1_OVER8) != 0)	
	{	// 8 oversampling
		tmpreg |= (((fractionaldivider * 8) + 50) / 100) & (0x07);
	}
	else	// 16 oversampling
	{
		tmpreg |= (((fractionaldivider * 16) + 50) / 100) & (0x0F);
	}

	// Write to USART BRR register
	UART4->BRR = (uint16_t)tmpreg;
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
void BEEP(void)			// Beep for 20 ms 
{ 	
	GPIOF->ODR |= (1<<9);	// PF9 'H' Buzzer on
	DelayMS(20);		// Delay 20 ms
	GPIOF->ODR &= ~(1<<9);	// PF9 'L' Buzzer off
}