//************************************//
// EXTI �ǽ�(EXTI8, 15), Priority ó��
//  -SW0(PH8) �Է�:  EXTI8 �߻�
//  -SW7(PH15) �Է�: EXTI15 �߻�
//  (1) High Priority ���ͷ�Ʈ(EXTI8) �߻��� ó���� 
//       Low Priority ���ͷ�Ʈ(EXTI15) �߻��� ����� ó�� 
//  (2) Low Priority ���ͷ�Ʈ(EXTI15) �߻��� ó���� 
//      High Priority ���ͷ�Ʈ(EXTI8) �߻��� ����� ó��  

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
	LCD_Init();	// LCD ��� �ʱ�ȭ
	DelayMS(10);
	_GPIO_Init();	// GPIO (LED & SW) �ʱ�ȭ
	_EXTI_Init();	// EXTI �ʱ�ȭ

        DisplayInitScreen();	// LCD �ʱ�ȭ��

	GPIOG->ODR &= ~0x00FF;		// �ʱⰪ: LED0~7 Off
	GPIOG->ODR |= 0x0001;		// GPIOG->ODR.0 'H'(LED ON)
    
	while(1)
	{
		//EXTI Example(1) : SW0�� High���� Low�� �� ��(Falling edge Trigger mode) EXTI8 ���ͷ�Ʈ �߻�, LED0 toggle
		if(SW0_Flag)	// ���ͷ�Ʈ (EXTI8) �߻�
		{
			GPIOG->ODR ^= 0x01;	// LED0 Toggle
			SW0_Flag = 0;
		}
	}
}

/* GLCD �ʱ�ȭ�� ���� �Լ� */
void DisplayInitScreen(void)
{
	LCD_Clear(RGB_WHITE);		// ȭ�� Ŭ����
	LCD_SetFont(&Gulim8);		// ��Ʈ : ���� 8
	LCD_SetBackColor(RGB_GREEN);	// ���ڹ��� : Green
	LCD_SetTextColor(RGB_BLACK);	// ���ڻ� : Black
	LCD_DisplayText(0,0,"EXTI Test");  	// Title

	LCD_SetBackColor(RGB_YELLOW);	//���ڹ��� : Yellow
}

/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer)) �ʱ� ����	*/
void _GPIO_Init(void)
{
	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	// LED (GPIO G) ���� : Output mode
	GPIOG->MODER 	&= ~0x0000FFFF;	// GPIOG 0~7 : Clear(0b00)						
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						

	GPIOG->OTYPER	&= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	

	GPIOG->OSPEEDR 	&= ~0x0000FFFF;	// GPIOG 0~7 : Clear(0b00)
	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
	// PUPDR : Default (floating) 
   
	// SW (GPIO H) ���� 
	RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	// Buzzer (GPIO F) ���� 
    	RCC->AHB1ENR	|=  0x00000020; // RCC_AHB1ENR : GPIOF(bit#5) Enable							
	GPIOF->MODER 	&= ~0x000C0000;	// GPIOF 9 : Clear(0b00)
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
	GPIOF->OTYPER 	&= ~0x0200;	// GPIOF 9 : Push-pull  	
 	GPIOF->OSPEEDR 	&= ~0x000C0000;	// GPIOF 9 : Clear(0b00)	
 	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 
}	

/* EXTI (EXTI8(GPIOH.8, SW0), EXTI9(GPIOH.9, SW1)) �ʱ� ����  */
void _EXTI_Init(void)
{
    	RCC->AHB1ENR 	|= 0x00000080;	// RCC_AHB1ENR GPIOH Enable
	RCC->APB2ENR 	|= 0x00004000;	// Enable System Configuration Controller Clock
	
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH PIN8~PIN15 Input mode (reset state)				 
	
// EXTI8,9 ����
	SYSCFG->EXTICR[2] |= 0x0077; 	// EXTI8,9�� ���� �ҽ� �Է��� GPIOH�� ����
					// EXTI8 <- PH8, EXTI9 <- PH9 
					// EXTICR3(EXTICR[2])�� �̿� 
					// reset value: 0x0000	

	EXTI->FTSR |= 0x000100;		// EXTI8: Falling Trigger Enable
	EXTI->RTSR |= 0x000200;		// EXTI9: Rising Trigger  Enable
    	EXTI->IMR  |= 0x000300;  	// EXTI8,9 ���ͷ�Ʈ mask (Interrupt Enable) ����
		
	NVIC->ISER[0] |= (1 << 23);	// 0x00800000
					// Enable 'Global Interrupt EXTI8,9'
					// Vector table Position ����

// EXTI15 ����
	SYSCFG->EXTICR[3] |= 0x7000; 	// EXTI15�� ���� �ҽ� �Է��� GPIOH�� ����
					// EXTI15 <- PH15
					// EXTICR4(EXTICR[3])�� �̿� 
					// reset value: 0x0000	

	EXTI->FTSR |= 0x008000;		// EXTI15: Falling Trigger Enable
    	EXTI->IMR  |= 0x008000;  	// EXTI15 ���ͷ�Ʈ mask (Interrupt Enable) ����
		
	NVIC->ISER[1] |= (1 << (40-32));// 0x00000100
					// Enable 'Global Interrupt EXTI15'
					// Vector table Position ����

//	NVIC_SetPriority(23, 0x0E);  // EXTI8
//	NVIC_SetPriority(40, 0x0F);  // EXTI15

	NVIC->IP[23]= 0xE0;  // High Priority   (14) -> �켱������ �� ����
	NVIC->IP[40]= 0xF0;  // Low Priority   (15)

}

/* EXTI5~9 ISR */
void EXTI9_5_IRQHandler(void)		
{
	if(EXTI->PR & 0x0100)                   // EXTI8 Interrupt Pending(�߻�) ����?
	{
		EXTI->PR |= 0x0100; 		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
		LCD_DisplayText(1,0,"EXTI8 ");
		while(KEY_Scan()  != 0xFB00) {		// SW2
			GPIOG->ODR ^= 0x01;		// LED0 Toggle  
		        DelayMS(200); 
		}    	
	}
}

/* EXTI15 ISR */
void EXTI15_10_IRQHandler(void)		
{
	if(EXTI->PR & 0x8000)                   // EXTI15 Interrupt Pending(�߻�) ����?
	{
		EXTI->PR |= 0x8000; 		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
               	LCD_DisplayText(1,0,"EXTI15");
		while(KEY_Scan()  != 0xBF00) {		// SW6
			GPIOG->ODR ^= 0x80;		// LED7 Toggle  
		        DelayMS(200); 
		}    	
	}
}
                
/* Switch�� �ԷµǾ����� ���ο� � switch�� �ԷµǾ������� ������ return�ϴ� �Լ�  */ 
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
        DelayUS(1000);         		// 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
    volatile int Dly = (int)wUS*17;
    for(; Dly; Dly--);
}
