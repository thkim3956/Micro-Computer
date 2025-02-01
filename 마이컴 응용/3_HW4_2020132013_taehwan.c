//************************************//
// 과제번호: 마이컴응용 2024 HW14
// 과제명: 오목게임(USART이용)
// 과제개요: -10X10 바둑판에서 적돌과 청돌을 이용하여 오목게임을 제작
//-바둑돌 착돌좌표위치는 조이스틱(또는 USART)으로 선택하고 승패결과를
//기록하고 FRAM에 승패결과를 저장함
// 사용한 하드웨어(기능): GPIO, Joy-stick, EXTI, GLCD, LED, Buzzer,Fram, USART1
// 제출일: 2024. 11. 27.
// 제출자 클래스: 목요일반
// 학번: 2020132013
// 이름: 김태환

#include "stm32f4xx.h"
#include "GLCD.h"
#include "FRAM.h"

#define SW0_PUSH        0xFE00  //PH8
#define SW1_PUSH        0xFD00  //PH9
#define SW2_PUSH        0xFB00  //PH10
#define SW3_PUSH        0xF700  //PH11
#define SW4_PUSH        0xEF00  //PH12
#define SW5_PUSH        0xDF00  //PH13
#define SW6_PUSH        0xBF00  //PH14
#define SW7_PUSH        0x7F00  //PH15

#define NAVI_PUSH	0x03C0	//PI5 0000 0011 1100 0000 
#define NAVI_UP		0x03A0	//PI6 0000 0011 1010 0000 
#define NAVI_DOWN	0x0360	//PI7 0000 0011 0110 0000 
#define NAVI_RIGHT	0x02E0	//PI8 0000 0010 1110 0000 
#define NAVI_LEFT	0x01E0	//PI9 0000 0001 1110 0000 

void _GPIO_Init(void);
void _EXTI_Init(void);

void DisplayInitScreen(void);
uint16_t KEY_Scan(void);
uint16_t JOY_Scan(void);

void BEEP(void);

void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

void USART1_Init(void);
void USART_BRR_Configuration(uint32_t USART_BaudRate);

void SerialSendChar(uint8_t c);
void SerialSendString(char *s);

uint8_t	SW0_Flag, SW1_Flag;

//--------------------Fram 변수 설정-----------------------------
int red_score; // 적돌 점수
int blue_score; // 청돌 점수
//--------------------변수 설정-----------------------------
int red_x = 5; //적돌 x좌표
int red_y = 5; //적돌 y좌표
int blue_x = 5; //청돌 x좌표
int blue_y = 5; //청돌 y좌표

int red_flag = 0;   //적돌 차례
int blue_flag = 0;  //청돌 차례   1이면 자기 순서


int red_position_x = 73; //적돌 블록 x위치
int red_position_y = 73; //적돌 블록 y위치
int blue_position_x = 73; //청돌 블록 x위치
int blue_position_y = 73; //청돌 블록 y위치
int board[10][10] = {0};  // 0: 빈 칸, 1: 적돌, 2: 청돌
int selected_stone = 0; // 현재 선택된 돌 상태 (0: 선택 안됨, 1: 적돌 선택, 2: 청돌 선택)

int send_ch= 0x30;
int No_send_data;
uint8_t str[20];

int num_uart = 0; //받은 문자의 순서

//----------------------함수 선언----------------------------
void Red_time(void);   //적돌 차례일 때 조이스틱 권한
void Blue_time(void);  //청돌 차례일 때 조이스틱 권한
void display_red(void); //적돌 좌표 및 돌 위치 디스플레이
void display_blue(void); //적돌 좌표 및 돌 위치 디스플레이
void PlaceStone(int x, int y, int color); //착돌 제한 조건 확인 및 착돌 후 승리 판별
int CheckWin(int x, int y, int color); //오목 승리 알고리즘
void ResetGame(void); //게임 리셋


int main(void)
{
	LCD_Init();	// LCD 모듈 초기화
	DelayMS(10);
	_GPIO_Init();	// GPIO 초기화
	_EXTI_Init();	// EXTI 초기화        
	Fram_Init();              
	Fram_Status_Config();
	red_score = Fram_Read(300); //fram 300번지에 적돌 스코어 저장
        if(red_score > 9) red_score = 0;
	blue_score = Fram_Read(301); //fram 301번지에 청돌 스코어 저장
        if(blue_score > 9) blue_score = 0;        
	DisplayInitScreen();	// LCD 초기화면
        USART1_Init();

	GPIOG->ODR &= ~0x00FF;	// 초기값: LED0~7 Off
	


    while(1)
    {
        if (red_flag == 1)  // 적돌이 선택된 경우
        {
            Red_time();  // 적돌 조이스틱 조작 가능
        }
        else if (blue_flag == 1)  // 청돌이 선택된 경우
        {
            Blue_time();  // 청돌 조이스틱 조작 가능
        }
    }
}

void USART1_IRQHandler(void)	
{       	
	// RX Buffer Full interrupt
	if ( (USART1->SR & USART_SR_RXNE) )		// USART_SR_RXNE=(1<<5) 
	{
		char ch;
		ch = (uint16_t)(USART1->DR & (uint16_t)0x01FF);	// 수신된 문자 저장
		// DR 을 읽으면 SR.RXNE bit(flag bit)는 clear 된다. 즉 clear 할 필요없음 
                if(num_uart < 3) num_uart++;
                if(num_uart == 1) //받은 돌 선택
                {
                  if(ch == 'R') 
                  {
                    red_flag = 1;  // 적돌 차례 시작
                    blue_flag = 0;  // 청돌 차례 종료
                    LCD_SetTextColor(RGB_RED);  // 글자색 : RED
                    LCD_DisplayText(9, 0, "*");  // 적돌 선택 표시
                    LCD_DisplayText(9, 17, " ");  // 청돌 선택 해제
                    GPIOG->ODR |= 0x0001;  // 적돌 LED On
                    GPIOG->ODR &= ~0x0080;  // 청돌 LED Off                                   
                  }
                  if(ch =='B')
                  {

                    blue_flag = 1;  // 청돌 차례 시작
                    red_flag = 0;  // 적돌 차례 종료
                    LCD_SetTextColor(RGB_BLUE);  // 글자색 : BLUE
                    LCD_DisplayText(9, 0, " ");  // 적돌 선택 해제
                    LCD_DisplayText(9, 17, "*");  // 청돌 선택 표시
                    GPIOG->ODR |= 0x0080;  // 청돌 LED On
                    GPIOG->ODR &= ~0x0001;  // 적돌 LED Off   
                  }
                }
                if(num_uart == 2) //받은 x좌표 처리
                {
                  if ((red_flag == 1 && selected_stone == 0) || (red_flag == 1 && selected_stone == 1))  // 적돌 차례일 때
                  {
                    red_x = ch - '0'; //red좌표 문자를 숫자로 변환
                    red_position_x = red_x * 10 + 23; //red 착돌 할 디스플레이 위치                    
                  }
                  else if ((blue_flag == 1 && selected_stone == 0) || (blue_flag == 1 && selected_stone == 2))  // 청돌 차례일 때
                  {
                    blue_x = ch - '0'; //blue좌표 문자를 숫자로 변환
                    blue_position_x = blue_x * 10 + 23; //blue 착돌 할 디스플레이 위치
                  }
                }
                
                if(num_uart == 3) //받은 y좌표 처리
                {
                  num_uart = 0;
                  if ((red_flag == 1 && selected_stone == 0) || (red_flag == 1 && selected_stone == 1))  // 적돌 차례일 때
                  {
                    red_y = ch - '0'; //red좌표 문자를 숫자로 변환
                    red_position_y = red_y * 10 + 23;  //red 착돌 할 디스플레이 위치
                    selected_stone = 2;  // 돌 선택 초기화 (다음 돌 선택 가능)
                    red_flag = 0;
                    display_red();
                    PlaceStone(red_x, red_y, 1);  // 적돌 배치
                  }
                  else if ((blue_flag == 1 && selected_stone == 0) || (blue_flag == 1 && selected_stone == 2))  // 청돌 차례일 때
                  {
                    blue_y = ch - '0'; //blue좌표 문자를 숫자로 변환
                    blue_position_y = blue_y * 10 + 23; //blue 착돌 할 디스플레이 위치
                    selected_stone = 1;  // 돌 선택 초기화 (다음 돌 선택 가능)
                    blue_flag = 0;
                    display_blue();
                    PlaceStone(blue_x, blue_y, 2);  // 청돌 배치
                  }
                  else //자기 차례가 아닌 경우
                  {
                    BEEP();
                    DelayMS(100);
                    BEEP();
                    DelayMS(100);
                    BEEP();
                    SerialSendString("Not running");
                  }
                  
                }
                if(num_uart > 3) num_uart = 0;
                                              
	} 
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
    
	USART_BRR_Configuration(38400); // USART Baud rate Configuration
    	
        USART1->CR1	|= (1<<12);	// USART_WordLength 9 Data bit
        USART1->CR1	|= (1<<10);	// USART_Parity
        USART1->CR1	|= (1<<9);	// Odd_Parity
        
	USART1->CR1	|= (1<<2);	// 0x0004, USART_Mode_RX Enable
	USART1->CR1	|= (1<<3);	// 0x0008, USART_Mode_Tx Enable
	USART1->CR2	&= ~(3<<12);	// 0b00, USART_StopBits_1
	USART1->CR3	= 0x0000;	// No HardwareFlowControl, No DMA
    
	USART1->CR1	|= (1<<5);	// 0x0020, RXNE interrupt Enable
	USART1->CR1	&= ~(1<<7); // 0x0080, TXE interrupt Disable 

	NVIC->ISER[1]	|= (1<<(37-32));// Enable Interrupt USART1 (NVIC 37번)
	USART1->CR1 	|= (1<<13);	//  0x2000, USART1 Enable
}

void SerialSendChar(uint8_t Ch) // 1문자 보내기 함수
{
	while((USART1->SR & USART_SR_TXE) == RESET); // USART_SR_TXE=(1<<7), 송신 가능한 상태까지 대기

	USART1->DR = (Ch & 0x01FF);	// 전송 (최대 9bit 이므로 0x01FF과 masking)
}

void SerialSendString(char *str) // 여러문자 보내기 함수
{
	while (*str != '\0') // 종결문자가 나오기 전까지 구동, 종결문자가 나온후에도 구동시 메모리 오류 발생가능성 있음.
	{
		SerialSendChar(*str);	// 포인터가 가르키는 곳의 데이터를 송신
		str++; 			// 포인터 수치 증가
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

/* GLCD 초기화면 설정 함수 */
void DisplayInitScreen(void)
{
	LCD_Clear(RGB_YELLOW);		// 화면 클리어
	LCD_SetFont(&Gulim8);		// 폰트 : 굴림 8
	LCD_SetBackColor(RGB_YELLOW);	// 글자배경색 : Yellow
	LCD_SetTextColor(RGB_BLACK);	// 글자색 : Black
	LCD_DisplayText(0,0,"Mecha-OMOK(KTH)");  	// Title
        LCD_DisplayText(1,2,"0");
        LCD_DisplayText(1,9,"5");
        LCD_DisplayText(1,14,"9");
        LCD_DisplayText(5,2,"5");
        LCD_DisplayText(8,2,"9");
        LCD_DisplayText(9,8,"VS");
        LCD_SetPenColor(RGB_BLACK);
        LCD_DrawRectangle(25,25,90,90);
        LCD_DrawHorLine(25,35,90);
        LCD_DrawHorLine(25,45,90);
        LCD_DrawHorLine(25,55,90);
        LCD_DrawHorLine(25,65,90);
        LCD_DrawHorLine(25,75,90);
        LCD_DrawHorLine(25,85,90);
        LCD_DrawHorLine(25,95,90);
        LCD_DrawHorLine(25,105,90);
        LCD_DrawVerLine(35,25,90);
        LCD_DrawVerLine(45,25,90);
        LCD_DrawVerLine(55,25,90);
        LCD_DrawVerLine(65,25,90);
        LCD_DrawVerLine(75,25,90);
        LCD_DrawVerLine(85,25,90);
        LCD_DrawVerLine(95,25,90);
        LCD_DrawVerLine(105,25,90); 
        
        LCD_SetBrushColor(RGB_BLACK);
        LCD_DrawFillRect(73,73,5,5);
        
        LCD_SetTextColor(RGB_RED);	// 글자색 : RED
        LCD_DisplayText(9,1,"("); 
        LCD_DisplayText(9,3,","); 
        LCD_DisplayText(9,5,")");        
        LCD_DisplayChar(9,2,red_x+0x30);  // 적돌 x좌표 display
        LCD_DisplayChar(9,4,red_y+0x30);  // 적돌 y좌표 disply
        LCD_DisplayChar(9,7,red_score+0x30);  // 적돌 점수 disply
        
        LCD_SetTextColor(RGB_BLUE);	// 글자색 : BLUE
        LCD_DisplayText(9,12,"(");
        LCD_DisplayText(9,14,",");
        LCD_DisplayText(9,16,")");
        LCD_DisplayChar(9,13,blue_x+0x30);  // 청돌 x좌표 disply
        LCD_DisplayChar(9,15,blue_y+0x30);  // 청돌 y좌표 disply
	LCD_DisplayChar(9,10,blue_score+0x30);  // 청돌 점수 disply
}
/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer)) 초기 설정	*/
void _GPIO_Init(void)
{
	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	// LED (GPIO G) 설정 : Output mode
	GPIOG->MODER 	&= ~0x0000FFFF;	// GPIOG 0~7 : Clear(0b00)						
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						

	GPIOG->OTYPER	&= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	

	GPIOG->OSPEEDR 	&= ~0x0000FFFF;	// GPIOG 0~7 : Clear(0b00)
	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
	// PUPDR : Default (floating) 
   
	// SW (GPIO H) 설정 : Input mode 
	RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	// Buzzer (GPIO F) 설정 : Output mode
	RCC->AHB1ENR	|=  0x00000020;	// RCC_AHB1ENR : GPIOF(bit#5) Enable							
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
	GPIOF->OTYPER 	&= ~0x0200;	// GPIOF 9 : Push-pull  	
	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 

	//Joy Stick SW(PORT I)     
	RCC->AHB1ENR    |= 0x00000100;	// RCC_AHB1ENR GPIOI Enable
	GPIOI->MODER 	&=~0x000FFC00;	// GPIOI 5~9 : Input mode (reset state)
	GPIOI->PUPDR    &=~0x000FFC00;	// GPIOI 5~9 : Floating input (No Pull-up, pull-down) (reset state)

}	

/* EXTI (EXTI8(GPIOH.8, SW0), EXTI9(GPIOH.9, SW1)) 초기 설정  */
void _EXTI_Init(void)
{
	RCC->AHB1ENR 	|= 0x0080;	// RCC_AHB1ENR GPIOH Enable
	RCC->AHB1ENR    |= 0x0100;
	RCC->APB2ENR 	|= 0x4000;	// Enable System Configuration Controller Clock
	
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH PIN8~PIN15 Input mode (reset state)				 
	GPIOI->MODER 	&=~0x000FFC00; // GPIOI 5~9 : Input mode (reset state)
        SYSCFG->EXTICR[2] &= ~0x7077; // 원래 0으로 초기화하고 하는 것이 더 정확한 코드임
        
        SYSCFG->EXTICR[1] |= 0x0080;    // EXTI5 GPIOI로 설정
	SYSCFG->EXTICR[2] |= 0x0007;	// EXTI8에 대한 소스 입력은 GPIOH로 설정
	SYSCFG->EXTICR[3] |= 0x7000;	// EXTI8 <- PH8 EXTI15: PH15  EXTI14:PI9 
					// EXTICR3(EXTICR[2])를 이용 
					// reset value: 0x0000	
	
//	EXTI->FTSR |= 0x0100;		// EXTI8: Falling Trigger Enable 
//	EXTI->RTSR |= 0x0200;		// EXTI9: Rising Trigger  Enable
        EXTI->RTSR |= 0xfb20;		//5, 8~9 11~15 Rising Trigger Enable

	EXTI->IMR  |= 0xfb20;		// EXTI 5,8,9 인터럽트 mask (Interrupt Enable) 설정
		
	NVIC->ISER[0] |= ( 1 << 23  );	// Enable 'Global Interrupt EXTI8,9'  
					// Vector table Position 참조
        NVIC->ISER[1] |= (1 << 40-32);
        
}

/* EXTI5~9 인터럽트 핸들러(ISR: Interrupt Service Routine) */
void EXTI9_5_IRQHandler(void)   //  5~9번까지 if문을 통해 만들어냄 
{
	if(EXTI->PR & 0x0100)			// EXTI8 Interrupt Pending(발생) 여부?    PR-> Pending Register(기록지)를 통해 확인
	{                                                  // 0x0000 or 0x0100 둘중에 하나 발생  8번이 발생하면 실행문으로 감
     
          EXTI->PR |= 0x0100;		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
                                        // 지우기 위해 1을 써줌 안지우면 계속 인터럽트가 발생함

              red_flag = 1;  // 적돌 차례 시작
              blue_flag = 0;  // 청돌 차례 종료
              LCD_SetTextColor(RGB_RED);  // 글자색 : RED
              LCD_DisplayText(9, 0, "*");  // 적돌 선택 표시
              LCD_DisplayText(9, 17, " ");  // 청돌 선택 해제
              GPIOG->ODR |= 0x0001;  // 적돌 LED On
              GPIOG->ODR &= ~0x0080;  // 청돌 LED Off
          
          
	}
        else if(EXTI->PR&0x0020)        //EXTI5 Joystic Push발생
        {
          EXTI->PR |= 0x0020;		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
                                        // 지우기 위해 1을 써줌 안지우면 계속 인터럽트가 발생함
          if ((red_flag == 1 && selected_stone == 0) || (red_flag == 1 && selected_stone == 1))  // 적돌 차례일 때
          {              
              selected_stone = 2;  // 돌 선택 초기화 (다음 돌 선택 가능)
              red_flag = 0;
              PlaceStone(red_x, red_y, 1);  // 적돌 배치              
          }
          else if ((blue_flag == 1 && selected_stone == 0) || (blue_flag == 1 && selected_stone == 2))  // 청돌 차례일 때
          {              
              selected_stone = 1;  // 돌 선택 초기화 (다음 돌 선택 가능)
              blue_flag = 0;
              PlaceStone(blue_x, blue_y, 2);  // 청돌 배치
          }
          else              //자기 차례아닐 때 경고음 발생
          {
              BEEP();
              DelayMS(100);
              BEEP();
              DelayMS(100);
              BEEP();
              SerialSendString("Not running");
          }
          
          
        }
	
	
}

void EXTI15_10_IRQHandler(void)
{ 
            if(EXTI->PR & 0x8000)
            {              // 0x0000 or 0x0100 둘중에 하나 발생  8번이 발생하면 실행문으로 감
              EXTI->PR |= 0x8000;		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
                                  // 지우기 위해 1을 써줌 안지우면 계속 인터럽트가 발생함          
                                
                  blue_flag = 1;  // 청돌 차례 시작
                  red_flag = 0;  // 적돌 차례 종료
                  LCD_SetTextColor(RGB_BLUE);  // 글자색 : BLUE
                  LCD_DisplayText(9, 0, " ");  // 적돌 선택 해제
                  LCD_DisplayText(9, 17, "*");  // 청돌 선택 표시
                  GPIOG->ODR |= 0x0080;  // 청돌 LED On
                  GPIOG->ODR &= ~0x0001;  // 적돌 LED Off
              
            }
            


}

/* Switch가 입력되었는지 여부와 어떤 switch가 입력되었는지의 정보를 return하는 함수  */ 
uint8_t key_flag = 0;
uint16_t KEY_Scan(void)	// input key SW0 - SW7 
{ 
	uint16_t key;
	key = GPIOH->IDR & 0xFF00;	// any key pressed ?
	if(key == 0xFF00)		// if no key, check key off
	{	if(key_flag == 0)
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
uint8_t joy_flag = 0;
uint16_t JOY_Scan(void)	// input joy stick NAVI_* 
{ 
	uint16_t key;
	key = GPIOI->IDR & 0x03E0;	// any key pressed ?
	if(key == 0x03E0)		// if no key, check key off
	{	if(joy_flag == 0)
			return key;
		else
		{	DelayMS(10);
			joy_flag = 0;
			return key;
		}
	}
	else				// if key input, check continuous key
	{	if(joy_flag != 0)	// if continuous key, treat as no key input
			return 0x03E0;
		else			// if new key,delay for debounce
		{	joy_flag = 1;
			DelayMS(10);
			return key;
		}
	}
}

/* Buzzer: Beep for 30 ms */
void BEEP(void)			
{ 	
	GPIOF->ODR |=  0x0200;	// PF9 'H' Buzzer on
	DelayMS(30);		// Delay 30 ms
	GPIOF->ODR &= ~0x0200;	// PF9 'L' Buzzer off
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

void Red_time(void) // 적돌 차례일 때 조이스틱으로 착돌할 곳 정하기
{
  switch(JOY_Scan())
  {  
    case 0x01E0 :	// NAVI_LEFT                    
      
      red_x--;
      if(red_x < 0) red_x = 0;
      
      red_position_x -= 10;    
      if(red_position_x < 23) red_position_x = 23;
      BEEP();
      display_red();
      break;
    
    case 0x02E0:	// NAVI_RIGHT
      red_x++;
      if(red_x > 9) red_x = 9;
      
      red_position_x += 10;
      if(red_position_x > 113) red_position_x = 113;
      BEEP();
      display_red();
      break;
   
    case 0x03A0:	// NAVI_UP
      red_y--;
      if(red_y < 0) red_y = 0;
      
      red_position_y -= 10;
      if(red_position_y < 23) red_position_y = 23;
      BEEP();
      display_red();
      break;
                                            
    case 0x0360:	// NAVI_DOWN
      red_y++;
      if(red_y > 9) red_y = 9;
      
      red_position_y += 10;
      if(red_position_y > 113) red_position_y = 113;
      BEEP();
      display_red();
      break;    
  }  // switch(JOY_Scan()) 
}

void Blue_time(void) //청돌 차례일 때 조이스틱으로 착돌할 곳 정하기
{
  switch(JOY_Scan())
  {  
    case 0x01E0 :	// NAVI_LEFT                    
      blue_x--;
      if(blue_x < 0) blue_x = 0;
      
      blue_position_x -= 10;    
      if(blue_position_x < 23) blue_position_x = 23;
      BEEP();
      display_blue();
      break;
    
    case 0x02E0:	// NAVI_RIGHT
      blue_x++;
      if(blue_x > 9) blue_x = 9;
      
      blue_position_x += 10;
      if(blue_position_x > 113) blue_position_x = 113;
      BEEP();
      display_blue();
      break;
   
    case 0x03A0:	// NAVI_UP
      blue_y--;
      if(blue_y < 0) blue_y = 0;
      
      blue_position_y -= 10;
      if(blue_position_y < 23) blue_position_y = 23;
      BEEP();
      display_blue();
      break;
                                            
    case 0x0360:	// NAVI_DOWN
      blue_y++;
      if(blue_y > 9) blue_y = 9;
      
      blue_position_y += 10;
      if(blue_position_y > 113) blue_position_y = 113;
      BEEP();
      display_blue();
      break;    
  }  // switch(JOY_Scan())   
}

void display_red(void)
{
  LCD_SetTextColor(RGB_RED);	// 글자색 : RED
  LCD_DisplayChar(9,2,red_x+0x30);  // 적돌 x좌표 display
  LCD_DisplayChar(9,4,red_y+0x30);  // 적돌 y좌표 disply  
}

void display_blue(void)
{
  LCD_SetTextColor(RGB_BLUE);	// 글자색 : BLUE
  LCD_DisplayChar(9,13,blue_x+0x30);  // 청돌 x좌표 disply
  LCD_DisplayChar(9,15,blue_y+0x30);  // 청돌 y좌표 disply 

}

void PlaceStone(int x, int y, int color) {  //착돌 조건 확인 후 착돌
    if (board[x][y] == 0) { //착돌할 x,y좌표에 돌이 놓여져 있는지
        board[x][y] = color;  // 착돌할 x,y좌표에 착돌할 돌 색깔 입력
        if (color == 1) { //적돌 일 때 적돌  착돌한거 화면에 표시
            LCD_SetBrushColor(RGB_RED);
            LCD_DrawFillRect(red_position_x, red_position_y, 5, 5);
            SerialSendChar('R');
            SerialSendChar(red_x + 0x30);
            SerialSendChar(red_y + 0x30);
            BEEP();
        } 
        else if (color == 2) { //청돌 일 때
            LCD_SetBrushColor(RGB_BLUE);
            LCD_DrawFillRect(blue_position_x, blue_position_y, 5, 5);
            SerialSendChar('B');
            SerialSendChar(blue_x + 0x30);
            SerialSendChar(blue_y + 0x30);
            BEEP();
            
        }
        if (CheckWin(x, y, color)) { //착돌 후 오목 승리 조건 알고리즘을 통해 확인
            if (color == 1) { //적돌이 이겼을 경우 적돌 승리 스코어 +1
                red_score++;      
                if(red_score > 9) red_score = 0;
                Fram_Write(300,red_score);
                LCD_SetTextColor(RGB_RED);	// 글자색 : RED
                LCD_DisplayChar(9,7,red_score+0x30);  // 적돌 점수 disply
            } 
            else { //청돌이 이겼을 경우 청돌 승리 스코어 +1
                blue_score++;             
                if(blue_score > 9) blue_score = 0;
                Fram_Write(301,blue_score);
                LCD_SetTextColor(RGB_BLUE);	// 글자색 : BLUE
                LCD_DisplayChar(9,10,blue_score+0x30);  // 청돌 점수 disply
            }
            //승리 부저 알림 후 게임 재시작
            DelayMS(1000);
            BEEP();
            DelayMS(1000);
            BEEP();
            DelayMS(1000);
            BEEP();
            DelayMS(1000);
            BEEP();
            DelayMS(1000);
            BEEP();                        
            ResetGame(); //게임 재시작
        }
    } 
    else // 이미 돌이 놓여진 좌표에 착돌할 경우 부저 알림
    {
        BEEP();
        DelayMS(100);
        BEEP();
        DelayMS(100);
        BEEP();
        SerialSendString("Not running");
        if(color == 1) //적돌 차례인 경우 차례 유지
        {
          red_flag = 1;
          selected_stone = 1;
        }
        else if(color == 2) //청돌 차례인 경우 차례 유지
        {
          blue_flag = 1;
          selected_stone = 2;
        }
    }
}


int CheckWin(int x, int y, int color) {
    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};  // 가로(1,0), 세로(0,1), 대각선 오른쪽 아래(1,1), 반대 대각선 방향(1,-1) 
    for (int d = 0; d < 4; d++) { //4개의 방향을 확인
        int count = 1; //연속된 돌의 수를 세는 변수
        // 한 방향으로 체크
        for (int i = 1; i < 5; i++) {
            int nx = x + directions[d][0] * i; //탐색할 새로운 좌표를 방향에따라 한칸씩 이동 가로 방향
            int ny = y + directions[d][1] * i;// 세로 방향
            if (nx < 0 || nx >= 10 || ny < 0 || ny >= 10 || board[nx][ny] != color) break; //범위를 벗어나거나 위치에 놓인 돌의 색이 같지 않으면 탐색 중단
            count++; //조건이 맞으면 +1
        }
        // 반대 방향으로 체크
        for (int i = 1; i < 5; i++) {
            int nx = x - directions[d][0] * i;
            int ny = y - directions[d][1] * i;
            if (nx < 0 || nx >= 10 || ny < 0 || ny >= 10 || board[nx][ny] != color) break;
            count++;
        }
        if (count == 5) return 1;  // 승리 조건 만족
    }
    return 0;
}

void ResetGame(void) {
    // 바둑판 초기화
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            board[i][j] = 0;  // 바둑판 비우기
        }
    }

    // 좌표 초기화
    red_x = 5; 
    red_y = 5;
    blue_x = 5;
    blue_y = 5;

    red_position_x = 73;
    red_position_y = 73;
    blue_position_x = 73;
    blue_position_y = 73;

    // 차례 초기화
    red_flag = 0;  // 적돌 차례로 시작
    blue_flag = 0;
    selected_stone = 0; 
    // LCD 초기화면
    DisplayInitScreen();
    GPIOG->ODR &= ~0x00FF;	// 초기값: LED0~7 Off
}
