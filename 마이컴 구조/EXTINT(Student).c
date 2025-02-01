//************************************//
// EXTI 실습(EXTI8, 9)
// SW0(PH8) 입력: EXTI8 발생
// SW1(PH9) 입력: EXTI9 발생

#include "stm32f4xx.h"
#include "GLCD.h"

void _GPIO_Init(void);
void _EXTI_Init(void);

void DisplayInitScreen(void);
uint16_t KEY_Scan(void);
void BEEP(void);

void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

uint8_t	SW0_Flag, SW1_Flag;

int main(void)
{
	LCD_Init();	// LCD 모듈 초기화
	DelayMS(10);
	_GPIO_Init();	// GPIO 초기화
	_EXTI_Init();	// EXTI 초기화

	DisplayInitScreen();	// LCD 초기화면

	GPIOG->ODR &= ~0x00FF;	// 초기값: LED0~7 Off
	GPIOG->ODR |= 0x0001;	// GPIOG->ODR.0 'H'(LED ON)
    
	while(1)
	{
		//EXTI Example(1) : SW0가 High에서 Low가 될 때(Falling edge Trigger mode) EXTI8 인터럽트 발생, LED0 toggle
		if(SW0_Flag)	// 인터럽트 (EXTI8) 발생
		{
			GPIOG->ODR ^= 0x01;	// LED0 Toggle
			SW0_Flag = 0;
		}
	}
}

/* GLCD 초기화면 설정 함수 */
void DisplayInitScreen(void)
{
	LCD_Clear(RGB_WHITE);		// 화면 클리어
	LCD_SetFont(&Gulim8);		// 폰트 : 굴림 8
	LCD_SetBackColor(RGB_GREEN);	// 글자배경색 : Green
	LCD_SetTextColor(RGB_BLACK);	// 글자색 : Black
	LCD_DisplayText(0,0,"EXTI Test");  	// Title

	LCD_SetBackColor(RGB_YELLOW);	//글자배경색 : Yellow
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
}	

/* EXTI (EXTI8(GPIOH.8, SW0), EXTI9(GPIOH.9, SW1)) 초기 설정  */
void _EXTI_Init(void)
{
	RCC->AHB1ENR 	|= 0x0080;	// RCC_AHB1ENR GPIOH Enable
	RCC->APB2ENR 	|= 0x4000;	// Enable System Configuration Controller Clock
	
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH PIN8~PIN15 Input mode (reset state)				 
	
        SYSCFG->EXTICR[2] &= ~0x7077; // 원래 0으로 초기화하고 하는 것이 더 정확한 코드임

	SYSCFG->EXTICR[2] |= 0x7077;	// EXTI8,9에 대한 소스 입력은 GPIOH로 설정
					// EXTI8 <- PH8, EXTI9 <- PH9 
					// EXTICR3(EXTICR[2])를 이용 
					// reset value: 0x0000	
        SYSCFG->EXTICR[3] |= 0x7777;
	
//	EXTI->FTSR |= 0x0100;		// EXTI8: Falling Trigger Enable 
//	EXTI->RTSR |= 0x0200;		// EXTI9: Rising Trigger  Enable
        EXTI->RTSR |= 0xfb00;		//8~9 11~15

	EXTI->IMR  |= 0xfb00;		// EXTI8,9 인터럽트 mask (Interrupt Enable) 설정
		
	NVIC->ISER[0] |= ( 1 << 23  );	// Enable 'Global Interrupt EXTI8,9'  
					// Vector table Position 참조
        NVIC->ISER[1] |= (1 <<8 );
}

/* EXTI5~9 인터럽트 핸들러(ISR: Interrupt Service Routine) */
void EXTI9_5_IRQHandler(void)   //  5~9번까지 if문을 통해 만들어냄 
{		
	if(EXTI->PR & 0x0100)			// EXTI8 Interrupt Pending(발생) 여부?    PR-> Pending Register(기록지)를 통해 확인
	{                                                  // 0x0000 or 0x0100 둘중에 하나 발생  8번이 발생하면 실행문으로 감
     
		EXTI->PR |= 0x0100;		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
                                    // 지우기 위해 1을 써줌 안지우면 계속 인터럽트가 발생함
		SW0_Flag = 1;			// SW0_Flag: EXTI8이 발생되었음을 알리기 위해 만든 변수(main문의 mission에 사용) 
		LCD_DisplayText(1,0,"EXTI8 " );
	}
	//EXTI Example(2) : SW1가 Low에서 High가 될 때(Rising edge Trigger mode) EXTI9 인터럽트 발생, LED1 toggle
	else if(EXTI->PR & 0x0200)		// EXTI9 Interrupt Pending(발생) 여부?
	{
		EXTI->PR |= 0x0200;		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
		LCD_DisplayText(1,0,"EXTI9 " );
		GPIOG->ODR ^= 0x02;		// LED1 Toggle
	}
}
//40번
void EXTI15_10_IRQHandler(void)
{
             if(EXTI->PR & 0x0800)
            {                                                  // 0x0000 or 0x0100 둘중에 하나 발생  8번이 발생하면 실행문으로 감

                    EXTI->PR |= 0x0800;		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
                                        // 지우기 위해 1을 써줌 안지우면 계속 인터럽트가 발생함
                    		// SW0_Flag: EXTI8이 발생되었음을 알리기 위해 만든 변수(main문의 mission에 사용) 
                    GPIOG->ODR ^= 0x08;		
                    LCD_DisplayText(1,0,"EXTI11");
            }

            else if(EXTI->PR & 0x1000)
            {                                                  // 0x0000 or 0x0100 둘중에 하나 발생  8번이 발생하면 실행문으로 감

                    EXTI->PR |= 0x1000;		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
                                        // 지우기 위해 1을 써줌 안지우면 계속 인터럽트가 발생함
                    GPIOG->ODR ^= 0x10;			// SW0_Flag: EXTI8이 발생되었음을 알리기 위해 만든 변수(main문의 mission에 사용) 
                    LCD_DisplayText(1,0,"EXTI12");
            }
            else if(EXTI->PR & 0x2000)
            {                                                  // 0x0000 or 0x0100 둘중에 하나 발생  8번이 발생하면 실행문으로 감

                    EXTI->PR |= 0x2000;		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
                                        // 지우기 위해 1을 써줌 안지우면 계속 인터럽트가 발생함
                    GPIOG->ODR ^= 0x20;			// SW0_Flag: EXTI8이 발생되었음을 알리기 위해 만든 변수(main문의 mission에 사용) 
                    LCD_DisplayText(1,0,"EXTI13");
            }
            else if(EXTI->PR & 0x4000)
            {                                                  // 0x0000 or 0x0100 둘중에 하나 발생  8번이 발생하면 실행문으로 감

                    EXTI->PR |= 0x4000;		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
                                        // 지우기 위해 1을 써줌 안지우면 계속 인터럽트가 발생함
                    GPIOG->ODR ^= 0x40;		// SW0_Flag: EXTI8이 발생되었음을 알리기 위해 만든 변수(main문의 mission에 사용) 
                    LCD_DisplayText(1,0,"EXTI14");
            }
            else if(EXTI->PR&0x8000)
            {                                                  // 0x0000 or 0x0100 둘중에 하나 발생  8번이 발생하면 실행문으로 감

                    EXTI->PR |= 0x8000;		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
                                        // 지우기 위해 1을 써줌 안지우면 계속 인터럽트가 발생함
                    GPIOG->ODR ^= 0x80;	// SW0_Flag: EXTI8이 발생되었음을 알리기 위해 만든 변수(main문의 mission에 사용) 
                    LCD_DisplayText(1,0,"EXTI15");
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
