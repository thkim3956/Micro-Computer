/////////////////////////////////////////////////////////////
 // 과제명: 커피 자동 판매기
 // 과제개요:   커피를 자동으로 제조 및 판매하는 커피자판기 제어 프로그램을 작성함
 // 사용한 하드웨어(기능): GPIO, Joy-stick, EXTI, GLCD, LED, Buzzer,Fram
 // 제출일: 2024. 6. 15
 // 제출자 클래스: 화요일반
// 학번: 2020132013
 // 이름: 김태환
 ///////////////////////////////////////////////////////////////

#include "stm32f4xx.h"
#include "GLCD.h"
#include "FRAM.h"
#define RGB_GREY GET_RGB(128,128,128)    //GREY 색깔 RGB설정

void _GPIO_Init(void);
void _EXTI_Init(void);

void DisplayInitScreen(void);
uint16_t KEY_Scan(void);
uint16_t JOY_Scan(void);
void BEEP(void);


void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

void change_text(void);           // 숫자를 문자열로 바꿔주는 함수

void inventory_display(void);   //재고 수량 화면 갱신 함수
void input_coin_display(void);  // 코인 입력 화면 갱신
void tot_display(void);     // tot 화면 갱신
void NoC_display(void); // NoC 화면 갱신
void working_display(void);   //동작 화면 함수
void refill(void);                      // 재료 refill 해주는 함수

void return_coin(void);  // IN coin reset
void clear(void); // NoC, TOT reset

uint8_t price;   // 커피 종류 선택 확인을 위한 변수

// 입력 코인 및 총 수입(fram read용 변수)
int input_coin;
int total_income;
int num_cf;


//재고
uint8_t inventory_cup = 9;
uint8_t inventory_sugar = 5;
uint8_t inventory_milk = 5;
uint8_t inventory_coffee = 9;

//숫자를 문자열로 바꿔주고 이것을 display할 때 사용하기 위한 함수
char coin[10];
char total[10];
char num_coffee[10];

int main(void)
{
	LCD_Init();	// LCD 모듈 초기화
	DelayMS(10);
	_GPIO_Init();	// GPIO 초기화
	_EXTI_Init();	// EXTI 초기화

	Fram_Init();            // FRAM 초기화 H/W 초기화
	Fram_Status_Config();   // FRAM 초기화 S/W 초기화
        
        
        input_coin = Fram_Read(602);   // input coin`
        if(input_coin>200 || input_coin % 10 !=0) input_coin = 0;  // fram에서 읽어 올 때 200 이상(or 1의 자리에 쓰레기 값이 들어가 있는 경우)이면 0으로 표시

        total_income = Fram_Read(603);  // tot
        if(total_income>99) total_income = 0;  // fram에서 읽어 올 때 990이상이면 0으로 표시
        
        num_cf = Fram_Read(604);          // noc
        if(num_cf > 50) num_cf = 0; // fram에서 읽어 올 때 50 이상이면 0으로 표시

	DisplayInitScreen();	// LCD 초기화면
	GPIOG->ODR &= ~0x00FF;	// 초기값: LED0~7 Off
         
        
        
        
	while (1)
	{
                if(inventory_sugar== 0 || inventory_cup== 0 || inventory_milk == 0 || inventory_coffee == 0) //재고가 하나라도 0이면은 코인 입력 안됨               
                {
                      EXTI->IMR  &= ~0x0300;		// EXTI 8, 9 (코인 입력) 인터럽트 mask (Interrupt disable) 설정
                }

                if(inventory_sugar> 0&& inventory_cup> 0 && inventory_milk > 0 && inventory_coffee > 0) //재고가 전부 0이상이면 코인 입력 가능               
                {
                      EXTI->IMR  |= 0x0300;		// EXTI 8, 9 (코인 입력) 인터럽트 mask (Interrupt Enable) 설정
                }

                if(inventory_sugar> 0 && inventory_cup> 0 && inventory_milk > 0 && inventory_coffee > 0) //모든 재고가 1 이상이어야  동작 가능
                {          
                      switch (JOY_Scan())
                      {
                      case 0x01E0:	// NAVI_LEFT: Black 선택
                              BEEP();   //재고 선택 Buzzer
                              LCD_SetBackColor(RGB_BLUE);	// 글자배경색 : BLUE
                              LCD_SetTextColor(RGB_RED);	// 글자색 : RED
                              LCD_DisplayText(3,1,"B");

                              LCD_SetBackColor(RGB_BLUE);	// 글자배경색 : blue
                              LCD_SetTextColor(RGB_WHITE);	// 글자색 : white                        
                              LCD_DisplayText(3,3,"M");
                              LCD_DisplayText(2,2,"S"); 
                              price =  10;    // black 선택
                              break;

                      case 0x03A0:	// NAVI_UP: Sugar 선택
                              BEEP();   //재고 선택 Buzzer
                              LCD_SetBackColor(RGB_BLUE);	// 글자배경색 : BLUE
                              LCD_SetTextColor(RGB_RED);	// 글자색 : RED
                              LCD_DisplayText(2,2,"S");

                              LCD_SetBackColor(RGB_BLUE);	// 글자배경색 : blue
                              LCD_SetTextColor(RGB_WHITE);	// 글자색 : white                        
                              LCD_DisplayText(3,3,"M");
                              LCD_DisplayText(3,1,"B"); 
                              price =  20;    // Sugar 선택

                              break;

                      case 0x02E0:	// NAVI_RIGHT: MIX 선택
                              BEEP();
                              LCD_SetBackColor(RGB_BLUE);	// 글자배경색 : BLUE
                              LCD_SetTextColor(RGB_RED);	// 글자색 : RED
                              LCD_DisplayText(3,3,"M");

                              LCD_SetBackColor(RGB_BLUE);	// 글자배경색 : blue
                              LCD_SetTextColor(RGB_WHITE);	// 글자색 : white                        
                              LCD_DisplayText(3,1,"B");
                              LCD_DisplayText(2,2,"S"); 
                              price =  30;    // MIX 선택

                              break;			
                      }
                      
                      
                      if (KEY_Scan() ==  0xFB00) // SW2: Working   
                      {                    
                            if(price == 10)  //black coffee 선택
                            {
                                  
                                    if(input_coin >= 10)        // in 코인 10원 이상일 때 동작 가능
                                    {

                                        working_display();          // 커피 머신 동작 함수
                                        input_coin -= 10;         // coin -10
                                        inventory_cup--;     // 컵 재고 -1
                                        inventory_coffee-- ; //커피 재고 -1
                                        num_cf++;                      // 커피 판매 수 증가
                                        total_income+= 1;              // 총 수입  +10                                                             
                                                                            
                                        inventory_display();   //재고 숫자 갱신
                                        input_coin_display();  //IN 숫자 갱신
                                        tot_display();              // TOT 숫자 갱신
                                        NoC_display();           // NoC 숫자 갱신
                                        EXTI->IMR  |= 0x3b00;		// EXTI 8, 9, 11, 12, 13 인터럽트 mask (Interrupt Enable) 설정
                                    }
                                
                                
                            }

                            else if(price == 20) // sugar coffee
                            {
                                
                                    if(input_coin >= 20)        // in 코인 20원 이상일 때 동작 가능
                                    {

                                        working_display();         // 커피 머신 동작 함수
                                        input_coin -= 20;         // coin -20
                                        inventory_cup--;     // 컵 재고 -1
                                        inventory_coffee-- ; //커피 재고 -1
                                        inventory_sugar-- ; //설탕재고 -1
                                        num_cf++;                      // 커피 판매 수 증가
                                        total_income+= 2;              // 총 수입  +20                                                             
                                                                            
                                        inventory_display();   //재고 숫자 갱신
                                        input_coin_display();  //IN 숫자 갱신
                                        tot_display();             //TOT 숫자 갱신
                                        NoC_display();         //NoC 숫자 갱신
                                        EXTI->IMR  |= 0x3b00;		// EXTI 8, 9, 11, 12, 13 인터럽트 mask (Interrupt Enable) 설정
                                    }
                                
                            }

                            else if(price == 30)  // Mix coffee
                            {
                                
                                    if(input_coin >= 30)   // in 코인 30원 이상일 때 동작 가능
                                    {

                                        working_display();         //커피 머신 동작 함수
                                        input_coin -= 30;         // coin -20
                                        inventory_cup--;     // 컵 재고 -1
                                        inventory_coffee-- ; //커피 재고 -1
                                        inventory_sugar-- ; //설탕재고 -1
                                        inventory_milk--;      // 우유 재고 -1
                                        num_cf++;                      // 커피 판매 수 증가
                                        total_income+= 3;              // 총 수입  +30                                                             
                                                                            
                                        inventory_display();   //재고 숫자 갱신
                                        input_coin_display();  //IN 숫자 갱신
                                        tot_display();             //TOT 숫자 갱신
                                        NoC_display();         //NoC 숫자 갱신
                                        EXTI->IMR  |= 0x3b00;		// EXTI 8, 9, 11, 12, 13 인터럽트 mask (Interrupt Enable) 설정
                                    }
                                
                            }
                             
                             
                      }
                } 
	}
}

/* GLCD 초기화면 설정 함수 */
void DisplayInitScreen(void)
{
        change_text();              // 디스플레이에 입력하기 위해 숫자를 문자열로 변화
	LCD_Clear(RGB_WHITE);		// 화면 클리어
	LCD_SetFont(&Gulim8);		// 폰트 : 굴림 8
	LCD_SetBackColor(RGB_YELLOW);	// 글자배경색 : Yellow
	LCD_SetTextColor(RGB_BLACK);	// 글자색 : BLACK
	LCD_DisplayText(1,1,"KTH coffe"); // Title
        LCD_SetPenColor(RGB_BLACK);
        LCD_DrawRectangle(7, 12, 73, 12);     
        LCD_SetBackColor(RGB_WHITE);	// 글자배경색 : White
        
        LCD_DisplayText(0,15,"IN");             // IN display
        LCD_DisplayText(3,15,"TOT");             // TOT display
        LCD_DisplayText(6,15,"RF");             // RF display
        LCD_DisplayText(7,15,"NoC");             // NoC display
        LCD_DisplayText(6,1,"cp sg mk cf");    // cp sg mk cf display
        LCD_DisplayText(1,11,"\\10");      // \10 display
        LCD_DisplayText(3,11,"\\50");     // 50 display
        LCD_DisplayChar(5,1,inventory_cup+0x30);  // 재고 숫자 display
        LCD_DisplayChar(5,4,inventory_sugar+0x30);
        LCD_DisplayChar(5,7,inventory_milk+0x30);
        LCD_DisplayChar(5,10,inventory_coffee+0x30);
        
        LCD_SetBackColor(RGB_BLUE);	// 글자배경색 : blue
	LCD_SetTextColor(RGB_WHITE);	// 글자색 : white
        LCD_DisplayText(3,1,"B");
        LCD_DisplayText(3,3,"M");
        LCD_DisplayText(2,2,"S"); 
        
        LCD_SetBackColor(RGB_RED);	// 글자배경색 : RED
	LCD_SetTextColor(RGB_WHITE);	// 글자색 : white     
        LCD_DisplayText(4,2,"W");
        
        LCD_SetPenColor(RGB_GREEN);       // 박스 테두리 녹색
        LCD_DrawRectangle(7, 38, 9, 13);     // B 박스
        LCD_DrawRectangle(23, 38, 9, 13);   // M 박스
        LCD_DrawRectangle(15, 26, 9, 13);   // S 박스
        LCD_DrawRectangle(15, 51, 9, 13);   // W 박스
        LCD_DrawRectangle(100, 26, 8, 8);   // 10원 박스
        LCD_DrawRectangle(100, 52, 8, 8);   // 50원 박스
        LCD_DrawRectangle(7, 64, 9, 13);     // 컵 재고 박스
        LCD_DrawRectangle(31, 64, 9, 13);     // 설탕재고 박스
        LCD_DrawRectangle(55, 64, 9, 13);     // 우유 재고 박스
        LCD_DrawRectangle(79, 64, 9, 13);     // 커피 재고 박스
        LCD_DrawRectangle(136, 77, 9, 12);     // RF박스
        LCD_DrawRectangle(119, 12, 25, 13);   // IN 박스
        LCD_DrawRectangle(119, 51, 25, 13);   //TOT 박스
        LCD_DrawRectangle(127, 103, 17, 13);   // NoC 박스
        
        LCD_SetBackColor(RGB_BLACK);	// 글자배경색 : Black
	LCD_SetTextColor(RGB_YELLOW);	// 글자색 : Yellow
        LCD_DisplayText(1,15,coin);  //  IN  숫자

        LCD_DisplayText(4,15,total);  // TOT  숫자
        LCD_DisplayText(4,17,"0");   //TOT 숫자  1의자리 숫자는 0으로 고정되어 있으므로 설정

        LCD_SetBackColor(RGB_YELLOW);	// 글자배경색 : Yellow
	LCD_SetTextColor(RGB_BLACK);	// 글자색 : Black
        LCD_DisplayText(8,16,num_coffee);  // NoC 숫자
        
        LCD_SetBrushColor(RGB_GREY);
        LCD_DrawFillRect(101, 27, 7, 7);        // 10원 박스 회색
        LCD_DrawFillRect(101, 53, 7, 7);        // 50원 박스 회색
        
        LCD_SetBrushColor(RGB_GREEN);
        LCD_DrawFillRect(137, 78, 8, 11);        // RF박스 녹색
        
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
        
        //Joy Stick SW 설정
        RCC->AHB1ENR	|= 0x00000100;	// RCC_AHB1ENR GPIOI Enable
	GPIOI->MODER	&= ~0x000FFC00;	// GPIOI 5~9 : Input mode (reset state)
	GPIOI->PUPDR	&= ~0x000FFC00;	  

}	

/* EXTI (EXTI8(GPIOH.8, SW0), EXTI9(GPIOH.9, SW1)), EXTI11(GPIOH.11, SW3)), EXTI12(GPIOH.12, SW4)), EXTI13(GPIOH.13, SW5)) 초기 설정  */
void _EXTI_Init(void)
{
	RCC->AHB1ENR 	|= 0x0080;	// RCC_AHB1ENR GPIOH Enable
	
        RCC->APB2ENR 	|= 0x4000;	// Enable System Configuration Controller Clock
	
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH PIN8~PIN15 Input mode (reset state)
                                  
        
        
        SYSCFG->EXTICR[2] &= ~0x7077;   //초기화
        SYSCFG->EXTICR[3] &= ~0x7777;   //초기화

	
        SYSCFG->EXTICR[2] |= 0x7077;	// EXTI 8,9,11 에 대한 소스 입력은 GPIOH로 설정
        SYSCFG->EXTICR[3] |= 0x0077;    // EXTI 12,13 에 대한 소스 입력은 GPIOH로 설정
	
        EXTI->FTSR |= 0x3b00;		//8, 9, 11, 12, 13 falling trigger 설정 
	EXTI->IMR  |= 0x3b00;		// EXTI 8, 9, 11, 12, 13 인터럽트 mask (Interrupt Enable) 설정
		
	NVIC->ISER[0] |= ( 1 << 23  );	// Enable 'Global Interrupt'  
        NVIC->ISER[1] |= ( 1 << 8  );	// Enable 'Global Interrupt'  
					
}



/* EXTI5~9 인터럽트 핸들러(ISR: Interrupt Service Routine) */
void EXTI9_5_IRQHandler(void)   
{		
                
		if(EXTI->PR & 0x0100)		// SW0(EXTI8): Coin 10원 입력
		{
			EXTI->PR |= 0x0100;		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)                       
                        BEEP();                        
                        LCD_SetBrushColor(RGB_YELLOW);
                        LCD_DrawFillRect(101, 27, 7, 7);        // 10원 박스 노랑
                        
                        input_coin+= 10;                            // coin 10원 추가
                        input_coin_display();
                        
                        DelayMS(1000);

                        LCD_SetBrushColor(RGB_GREY);
                        LCD_DrawFillRect(101, 27, 7, 7);        // 10원 박스 회색
                			

        
        
		}	

        else if(EXTI->PR & 0x0200)	//SW1(EXTI9) : Coin 50원 입력
		{
			EXTI->PR |= 0x0200;		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)		
                        BEEP();
                        LCD_SetBrushColor(RGB_YELLOW);
                        LCD_DrawFillRect(101, 53, 7, 7);        // 50원 박스 노랑

                        input_coin += 50;     // Coin 50원 추가
                                  

                        input_coin_display();   // Coin 추가 후 IN박스 안 디스플레이 갱신 동작
                        
                        DelayMS(1000);

                        LCD_SetBrushColor(RGB_GREY);
                        LCD_DrawFillRect(101, 53, 7, 7);        // 50원 박스 회색
                 }
}

void EXTI15_10_IRQHandler(void)
{
             if(EXTI->PR & 0x0800)              // SW3(EXTI11): 잔돈반환
            {                                                  
                    EXTI->PR |= 0x0800;		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
                    return_coin();                    // 잔돈 반환 함수               
                    
            }

            else if(EXTI->PR & 0x1000)          // SW4(EXTI12): RF(Cup, Sugar, Milk, Coffee리필)
            {
                    EXTI->PR |= 0x1000;		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)                  
                    refill();                              // 재고 리필 함수

            }
            else if(EXTI->PR & 0x2000)          // SW5(EXTI13)입력시 NoC, TOT 값을 ‘0‘으로 리셋
            {
                    EXTI->PR |= 0x2000;		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)             
                    clear();                              // NoC, TOT 값 리셋 함수

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
void change_text(void)   //숫자를 문자열로 바꿔주는 함수
{
        sprintf(coin,"%03d",input_coin);
        sprintf(total,"%02d",total_income);
        sprintf(num_coffee,"%02d",num_cf);
}

void inventory_display(void)    // 재고가 0이 되면 RF박스 빨강변경 및 재고 갱신 함수
{
        if(inventory_sugar<=0)
        {
          
          LCD_SetBrushColor(RGB_RED);
          LCD_DrawFillRect(137, 78, 8, 11);         // RF박스 빨강
        }
        else if(inventory_coffee <= 0)
        {
          
          LCD_SetBrushColor(RGB_RED);
          LCD_DrawFillRect(137, 78, 8, 11);         // RF박스 빨강
        }

        else if(inventory_cup <= 0)
        {
          
          LCD_SetBrushColor(RGB_RED);
          LCD_DrawFillRect(137, 78, 8, 11);         // RF박스 빨강
        }

        else if(inventory_milk <= 0)
        {
          
          LCD_SetBrushColor(RGB_RED);
          LCD_DrawFillRect(137, 78, 8, 11);         // RF박스 빨강
        }
        else
        {
          LCD_SetBrushColor(RGB_GREEN);
          LCD_DrawFillRect(137, 78, 8, 11);         // RF박스 green
        }       

        LCD_SetBackColor(RGB_WHITE);	// 글자배경색 : White
	LCD_SetTextColor(RGB_BLACK);	// 글자색 : BLACK
        LCD_DisplayChar(5,1,inventory_cup+0x30);
        LCD_DisplayChar(5,4,inventory_sugar+0x30);
        LCD_DisplayChar(5,7,inventory_milk+0x30);
        LCD_DisplayChar(5,10,inventory_coffee+0x30);

}

void input_coin_display(void)   // IN fram 저장 후 코인 입력 화면 갱신
{
        if(input_coin >=200)   //IN이 200이상일 때 200으로 유지
        {
            input_coin = 200;
            
        }        
        Fram_Write(602, input_coin);    // IN coin 값 Fram에 저장
        change_text();                              // 디스플레이에 입력하기 위해 숫자를 문자열로 변환
        LCD_SetBackColor(RGB_BLACK);	// 글자배경색 : Black
        LCD_SetTextColor(RGB_YELLOW);	// 글자색 : Yellow
        LCD_DisplayText(1,15, coin);
}

void tot_display(void)  // TOT fram 저장 후 디스플레이 갱신
{
        if(total_income > 99)   // TOT가 990이상일 때 990으로 유지
        {
            total_income = 99;                      
        }        

        Fram_Write(603,total_income);          //TOT 값 Fram에 저장
        change_text();                              // 디스플레이에 입력하기 위해 숫자를 문자열로 변환        

        LCD_SetBackColor(RGB_BLACK);	// 글자배경색 : Black
        LCD_SetTextColor(RGB_YELLOW);	// 글자색 : Yellow
        LCD_DisplayText(4,15,total);
}

void NoC_display(void)   // NoC fram 저장 후 디스플레이 갱신
{

        if(num_cf >= 50)   // Noc가 50이상일 때 50으로 유지
        {
           num_cf = 50;           
        }
        

        Fram_Write(604,num_cf);             // NoC값 Fram에 저장
        change_text();                              // 디스플레이에 입력하기 위해 숫자를 문자열로 변환
        LCD_SetBackColor(RGB_YELLOW);	// 글자배경색 : Yellow
	LCD_SetTextColor(RGB_BLACK);	// 글자색 : Black
        LCD_DisplayText(8,16,num_coffee);  
}

void working_display(void)
{
        EXTI->IMR  &= ~0x3b00;		// EXTI 8, 9, 11, 12, 13 인터럽트 mask (Interrupt disable) 설정    working중에는 모든 인터럽트 unmask
        GPIOG->ODR |= 0x00FF;          // LED 0~7 ON
        BEEP();     //동작 Buzzer
        LCD_SetBackColor(RGB_RED);	// 글자배경색 : RED
	LCD_SetTextColor(RGB_WHITE);	// 글자색 : white
        LCD_DisplayText(4,2,"0");             // 3초간 실행 중인 것을 디스플레이 표시
        DelayMS(1000);
        LCD_DisplayText(4,2,"1");
        DelayMS(1000);
        LCD_DisplayText(4,2,"2");
        DelayMS(1000);
        LCD_DisplayText(4,2,"W");
        GPIOG->ODR &= ~0x00FF;    // LED 0~7 OFF


        // 종료 후 커피 종류 선택 및 디스플레이 초기화
        LCD_SetBackColor(RGB_BLUE);	// 글자배경색 : blue
        LCD_SetTextColor(RGB_WHITE);	// 글자색 : white                        
        LCD_DisplayText(3,3,"M");
        LCD_DisplayText(3,1,"B"); 
        LCD_DisplayText(2,2,"S"); 
        price = 0;

        BEEP();   // 종료 Buzzer음 3번 동작
        DelayMS(500);
        BEEP();
        DelayMS(500);
        BEEP();
        
}

void return_coin(void)
{
        input_coin = 0;
        Fram_Write(602, input_coin);        
        change_text();                              // 디스플레이에 입력하기 위해 숫자를 문자열로 변환                      
        LCD_SetBackColor(RGB_BLACK);	// 글자배경색 : Black
        LCD_SetTextColor(RGB_YELLOW);	// 글자색 : Yellow
        LCD_DisplayText(1,15, coin);  

}

void clear(void)   // TOT, Noc 초기화
{
        total_income = 0;  // 0 으로 초기화
        num_cf = 0;
        Fram_Write(604,num_cf);
        Fram_Write(603,total_income);

        change_text();                              // 디스플레이에 입력하기 위해 숫자를 문자열로 변환
        
        LCD_SetBackColor(RGB_BLACK);	// 글자배경색 : Black
        LCD_SetTextColor(RGB_YELLOW);	// 글자색 : Yellow
        LCD_DisplayText(4,15, total);  

        LCD_SetBackColor(RGB_YELLOW);	// 글자배경색 : Yellow
	LCD_SetTextColor(RGB_BLACK);	// 글자색 : Black
        LCD_DisplayText(8,16,num_coffee);  
}

void refill(void)   // refill 함수
{
        int flag = 0;
        if(inventory_sugar==0)  //sugar 재고가 0일때 리필
        {
            inventory_sugar= 5;
            inventory_display();  //재고 display갱신
            flag = 1;
        }
        if(inventory_coffee == 0)  //coffee재고가 0일때 리필
        {
            inventory_coffee = 9; 
            inventory_display();
            flag = 1;

        }
        if(inventory_cup == 0) //cup재고가 0일때 리필
        {
            inventory_cup = 9;
            inventory_display();
            flag = 1;

        }
        if(inventory_milk == 0) //milk재고가 0일때 리필
        {
            inventory_milk = 5;
            inventory_display();
            flag = 1;

        }
        if(flag == 1) // 리필  해주고 buzzer소리
        {
            BEEP();
            DelayMS(500);
            BEEP();
            flag = 0;
        } 
        
}


