#include "stm32f4xx.h"
#include "GLCD.h"

void _GPIO_Init(void);
uint16_t KEY_Scan(void);

void BEEP(void);
void DisplayInitScreen(void);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

uint8_t	SW0_Flag, SW1_Flag;

int main(void)
{
    _GPIO_Init(); // GPIO (LED & SW) 초기화
    LCD_Init();	// LCD 모듈 초기화
    DelayMS(100);
    BEEP();

    GPIOG->ODR = 0xFF00;	// LED 초기값: LED0~7 Off
    DisplayInitScreen();	// LCD 초기화면
 
    while(1)
    {
        switch(KEY_Scan())
        {
        	case 0xFE00 : 	//SW0
			if (SW0_Flag==0) {
				GPIOG->ODR |= 0x0001; // LED0 ON		
                 		LCD_DisplayText(1,0,"0.LED0 ON ");
				SW0_Flag = 1;
			}
			else {
				GPIOG->ODR &= ~0x0001; // LED0 OFF
				LCD_DisplayText(1,0,"0.LED0 OFF");
				SW0_Flag = 0;
			}
	        break;
         }  // switch(KEY_Scan())
    }  // while(1)
}

void _GPIO_Init(void)
{
      	// LED (GPIO G) 설정
    	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x00FF;	        // GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
 	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
    
	// SW (GPIO H) 설정 
	RCC->AHB1ENR    |=  0x00000080;	        // RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	// Buzzer (GPIO F) 설정 
    	RCC->AHB1ENR	|=  0x00000020;         // RCC_AHB1ENR : GPIOF(bit#5) Enable							
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
	GPIOF->OTYPER 	&= ~0x0200;	        // GPIOF 9 : Push-pull  	
 	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 

}	

void DisplayInitScreen(void)
{
    LCD_Clear(RGB_WHITE);	// 화면 클리어
    LCD_SetFont(&Gulim8);	// 폰트 : 굴림 8
    LCD_SetBackColor(RGB_GREEN);// 글자배경색 : Green
    LCD_SetTextColor(RGB_BLACK);// 글자색 : Black
    LCD_DisplayText(0,0,"MENU");  // Title

    LCD_SetBackColor(RGB_YELLOW);	//글자배경색 : Yellow
          
    LCD_DisplayText(1,0,"0.LED0 OFF");
    LCD_DisplayText(2,0,"1.LED1 OFF");
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

void BEEP(void)			/* beep for 30 ms */
{ 	
	GPIOF->ODR |=  0x0200;	// PF9 'H' Buzzer on
	DelayMS(30);		// Delay 30 ms
	GPIOF->ODR &= ~0x0200;	// PF9 'L' Buzzer off
}

void DelayMS(unsigned short wMS)
{
    register unsigned short i;
    for (i=0; i<wMS; i++)
        DelayUS(1000);         		// 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
    volatile int Dly = (int)wUS*17;
    for(; Dly; Dly--);
}


