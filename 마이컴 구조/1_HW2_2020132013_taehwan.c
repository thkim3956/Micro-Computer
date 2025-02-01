//************************************//
/////////////////////////////////////////////////////////////
//  과제명: 엘레베이터
//  과제개요:  층 입력 명령에 따라 엘리베이터 이동 동작을 실행함
// 층 입력 명령 방법
//  (1) SW(0~7)에 의한 층 명령 입력
//  (2) JS(Joy-Stick)(LEFT, RIGHT)에 의한 층 명령 입력
 
// 층 이동은 LED(0~7)의 점멸로 표현
// Reset State: 0층 (0층부터 시작)

 //LED 점멸 명령은 반드시 다음 중 선택해서 사용할 것 
  //(1) GPIOG?ODR |= 0x?? , GPIOG?ODR &= 0x??
  //(2) GPIOG?BSRRL = 0x??, GPIOG?BSRRH = 0x??
  //(3) GPIOG?ODR <<= 1; (Shift 연산자 이용)
  //* GPIOG?ODR = 0x?? 와 같은 구문은 사용하지 말것!

//                *
//  사용한 하드웨어(기능): GPIO, Joy-stick, Buzzer, LED, SW
//  제출일: 2023. 5.  19
//  제출자 클래스:  화요일반 
//            학번: 2020132013
//            이름: 김태환
/////////////////////////////////////////////////////////////
//

#include "stm32f4xx.h"


void _GPIO_Init(void);
uint16_t KEY_Scan(void);
uint16_t JOY_Scan(void);

void BEEP(void);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

void elevator(uint16_t input_sw); //LED를 이동시키는 함수

uint8_t	SW0_Flag, SW1_Flag;

int main(void)
{
	_GPIO_Init();		// GPIO (LED & SW & Buzzer & Joystick)     초기화

       GPIOG->ODR &= 0xFF00;	// LED 초기값
       GPIOG->ODR |= 0x0001;  // 초기상태 0층
       
	while(1)
	{
          switch(KEY_Scan())
          {
                    case 0xFE00:  // sw0
                      BEEP();
                      if(GPIOG->ODR != 0x0001){
                          elevator(0x0001);    //LED0 이 동 
                          BEEP();
                          DelayMS(500);
                          BEEP();
                          DelayMS(500);
                          BEEP();
                      }
                      else{   //현재 층과 같은 SW를 입력 SW 입력마다 실행되는 부저1회 실행되나, LED 변화는 없음. 추가로 0.2초후 부저1회 더 실행 (총 Beep() 2회 실행)
                            DelayMS(200);
                            BEEP();
                      }
                      break;
                      
                    case 0xFD00:   // sw1
                      BEEP();
                      if(GPIOG->ODR != 0x0002){
                          elevator(0x0002);     //LED1 이 동
                          BEEP();
                          DelayMS(500);
                          BEEP();
                          DelayMS(500);
                          BEEP();
                      }
                      else{   //현재 층과 같은 SW를 입력 SW 입력마다 실행되는 부저1회 실행되나, LED 변화는 없음. 추가로 0.2초후 부저1회 더 실행 (총 Beep() 2회 실행)
                            DelayMS(200);
                            BEEP();
                      }
                      break;
                      
                    case 0xFB00:   // sw2
                      BEEP();
                      if(GPIOG->ODR != 0x0004){
                          elevator(0x0004);     //LED2 이 동                      
                          BEEP();
                          DelayMS(500);
                          BEEP();
                          DelayMS(500);
                          BEEP();
                      }
                      else{   //현재 층과 같은 SW를 입력 SW 입력마다 실행되는 부저1회 실행되나, LED 변화는 없음. 추가로 0.2초후 부저1회 더 실행 (총 Beep() 2회 실행)
                            DelayMS(200);
                            BEEP();
                      }
                      break;  
                      
                    case 0xF700:   // sw3
                      BEEP();
                      if(GPIOG->ODR != 0x0008){
                          elevator(0x0008);         //LED3 이 동                      
                          BEEP();
                          DelayMS(500);
                          BEEP();
                          DelayMS(500);
                          BEEP();
                      }
                      else{   //현재 층과 같은 SW를 입력 SW 입력마다 실행되는 부저1회 실행되나, LED 변화는 없음. 추가로 0.2초후 부저1회 더 실행 (총 Beep() 2회 실행)
                            DelayMS(200);
                            BEEP();
                      }
                      break;
                      
                    case 0xEF00:  //sw4
                      BEEP();
                      if(GPIOG->ODR != 0x0010){
                          elevator(0x0010);       //LED4 이 동                      
                          BEEP();
                          DelayMS(500);
                          BEEP();
                          DelayMS(500);
                          BEEP();
                      }
                      else{   //현재 층과 같은 SW를 입력 SW 입력마다 실행되는 부저1회 실행되나, LED 변화는 없음. 추가로 0.2초후 부저1회 더 실행 (총 Beep() 2회 실행)
                            DelayMS(200);
                            BEEP();
                      }
                      break;         
                      
                    case 0xDF00: //sw5
                      BEEP();
                      if(GPIOG->ODR != 0x0020){
                          elevator(0x0020);        //LED5 이 동                      
                          BEEP();
                          DelayMS(500);
                          BEEP();
                          DelayMS(500);
                          BEEP();
                      }
                      else{   //현재 층과 같은 SW를 입력 SW 입력마다 실행되는 부저1회 실행되나, LED 변화는 없음. 추가로 0.2초후 부저1회 더 실행 (총 Beep() 2회 실행)
                            DelayMS(200);
                            BEEP();
                      }
                      break;
                      
                    case 0xBF00:  //sw6
                      BEEP();
                      if(GPIOG->ODR != 0x0040){
                          elevator(0x0040);      //LED6이 동                      
                          BEEP();
                          DelayMS(500);
                          BEEP();
                          DelayMS(500);
                          BEEP();
                      }
                      else{   //현재 층과 같은 SW를 입력 SW 입력마다 실행되는 부저1회 실행되나, LED 변화는 없음. 추가로 0.2초후 부저1회 더 실행 (총 Beep() 2회 실행)
                            DelayMS(200);
                            BEEP();
                      }
                      break;
                      
                    case 0x7F00:   //sw7
                      BEEP();
                      if(GPIOG->ODR != 0x0080){
                          elevator(0x0080);     //LED7 이 동                      
                          BEEP();
                          DelayMS(500);
                          BEEP();
                          DelayMS(500);
                          BEEP();
                      }
                      else{   //현재 층과 같은 SW를 입력 SW 입력마다 실행되는 부저1회 실행되나, LED 변화는 없음. 추가로 0.2초후 부저1회 더 실행 (총 Beep() 2회 실행)
                            DelayMS(200);
                            BEEP();
                      }
                      break; 
          }
          
          switch(JOY_Scan())
          {
                case 0x01E0:   // LEFT
                  BEEP();
                  if(GPIOG->ODR !=0x0001)
                  {
                      GPIOG->ODR >>=1;           //LED 왼쪽으로 이동
                  }
                  
                  if(GPIOG->ODR ==0x0001)     //현재층이 0층일 때 JS-LEFT입력 하여 부저1회 실행 LED 변화는 없음. 추가 로 0.2초후 부저 1번 더 실행 (총 Beep() 2번 실행)
                  {
                      DelayMS(200);
                      BEEP();
                  }
                  break;
                  
                case 0x02E0:   //  RIGHT
                  BEEP();
                  if(GPIOG->ODR !=0x0080)        
                  {
                      GPIOG->ODR <<=1;               //LED 오른쪽으로 이동
                  }
                  else if(GPIOG->ODR ==0x0080)    //현재층이 7층일 때 JS-RIGHT입력 하여 부저1회 실행 LED 변화는 없음. 추가 로 0.2초후 부저 1번 더 실행 (총 Beep() 2번 실행)
                  {
                      DelayMS(200);
                      BEEP();
                  }
                  break;
                  
                  
          }
          
	}  // while(1)
}

/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer), GPIOI(Joystick)) 초기 설정	*/
void _GPIO_Init(void)
{
	// LED (GPIO G) 설정 : Output mode
	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
   
	// SW (GPIO H) 설정 : Input mode 
	RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	// Buzzer (GPIO F) 설정 : Output mode
	RCC->AHB1ENR	|=  0x00000020;	// RCC_AHB1ENR : GPIOF(bit#5) Enable							
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
	GPIOF->OTYPER 	&= ~0x0200;	// GPIOF 9 : Push-pull  	
	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 

	//Joy Stick SW(PORT I) 설정
	RCC->AHB1ENR	|= 0x00000100;	// RCC_AHB1ENR GPIOI Enable
	GPIOI->MODER	&= ~0x000FFC00;	// GPIOI 5~9 : Input mode (reset state)
	GPIOI->PUPDR	&= ~0x000FFC00;	// GPIOI 5~9 : Floating input (No Pull-up, pull-down) (reset state)
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

/* Switch가 입력되었는지를 여부와 어떤 switch가 입력되었는지의 정보를 return하는 함수  */ 
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

void elevator(uint16_t input_sw)
{
  if( GPIOG->ODR < input_sw )  //선택한 층이 현재있는 층보다 작은경우  --> 엘레베이터 내려가기
  {
    while(GPIOG->ODR != input_sw) //선택 sw와 같을 때까지 LED 이동
    {
      DelayMS(500); //0.5초 delay
      GPIOG->ODR <<= 1; // LED 1칸씩 이동
      DelayMS(500); //0.5초 delay
    }
  }
  
  else if(GPIOG->ODR > input_sw)  //선택한 층이 현재있는 층보다 큰 경우 --> 엘레베이터 올라가기
  {
    while(GPIOG->ODR != input_sw) //선택 sw와 같을 때까지 LED 이동
    {
      DelayMS(500); //0.5초 delay
      GPIOG->ODR >>= 1; // LED 1칸씩 이동
      DelayMS(500); //0.5초 delay
    }
  }
  
}