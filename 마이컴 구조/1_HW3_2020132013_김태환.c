/////////////////////////////////////////////////////////////
 // ������: Great Escape
 // ��������:  ROOM ���� ��й�ȣ(1�ڸ� ����)�� ���� Ż��
 // ����� �ϵ����(���): GPIO, Joy-stick, EXTI, GLCD, LED, Buzzer
 // ������: 2024. 6. 07
 // ������ Ŭ����: ȭ���Ϲ�
// �й�: 2020132013
 // �̸�: ����ȯ
 ///////////////////////////////////////////////////////////////

#include "stm32f4xx.h"
#include "GLCD.h"

#define RGB_PINK GET_RGB(255,0,255)     //��ũ�� 

void _GPIO_Init(void);
void _EXTI_Init(void);

void DisplayInitScreen(void);
uint16_t KEY_Scan(void);
uint16_t JOY_Scan(void);
void BEEP(void);
void BEEP_F(void);    //Ż�� ���� ���� 500ms ON�ϴ� �Լ�

void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

uint8_t	SW0_Flag, SW1_Flag;

int main(void)
{
	LCD_Init();	// LCD ��� �ʱ�ȭ
	DelayMS(10);
	_GPIO_Init();	// GPIO �ʱ�ȭ
	_EXTI_Init();	// EXTI �ʱ�ȭ

	DisplayInitScreen();	// LCD �ʱ�ȭ��
	GPIOG->ODR &= ~0x00FF;	// �ʱⰪ: LED0~7 Off
	
        while(1)
	{
		//EXTIȮ���� ����
	}
	
}

/* GLCD �ʱ�ȭ�� ���� �Լ� */
void DisplayInitScreen(void)
{
	LCD_Clear(RGB_WHITE);		// ȭ�� Ŭ����
	LCD_SetFont(&Gulim8);		// ��Ʈ : ���� 8
	LCD_SetBackColor(RGB_WHITE);	// ���ڹ��� : White
	LCD_SetTextColor(RGB_BLUE);	// ���ڻ� : BLUE
	LCD_DisplayText(0,0,"2020132013 KTH");  	// �й� �� �̸� ����

	LCD_SetTextColor(RGB_BLACK);	// ���ڻ� : BLACK
        LCD_DisplayText(1,0,"R 01234567");  	// room��ȣ 

        LCD_SetPenColor(RGB_BLACK);
        LCD_DrawRectangle(17, 25, 5, 5);  //0�� ��
        LCD_DrawRectangle(24, 25, 5, 5);  //1�� ��
        LCD_DrawRectangle(33, 25, 5, 5);  //2�� ��
        LCD_DrawRectangle(41, 25, 5, 5);  //3�� ��
        LCD_DrawRectangle(49, 25, 5, 5);  //4�� ��
        LCD_DrawRectangle(57, 25, 5, 5);  //5�� ��
        LCD_DrawRectangle(66, 25, 5, 5);  //6�� ��
        LCD_DrawRectangle(74, 25, 5, 5);  //7�� ��
        LCD_DisplayText(2,11,"S");  	  // ���ۻ�Ȳ ǥ��
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
   
	// SW (GPIO H) ���� : Input mode 
	RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	// Buzzer (GPIO F) ���� : Output mode
	RCC->AHB1ENR	|=  0x00000020;	// RCC_AHB1ENR : GPIOF(bit#5) Enable							
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
	GPIOF->OTYPER 	&= ~0x0200;	// GPIOF 9 : Push-pull  	
	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 
        
        //Joy Stick SW ����
        RCC->AHB1ENR	|= 0x00000100;	// RCC_AHB1ENR GPIOI Enable
	GPIOI->MODER	&= ~0x000FFC00;	// GPIOI 5~9 : Input mode (reset state)
	GPIOI->PUPDR	&= ~0x000FFC00;	  

}	

/* EXTI (EXTI8(GPIOH.8, SW0), EXTI9(GPIOH.9, SW1)) �ʱ� ����  */
void _EXTI_Init(void)
{
	RCC->AHB1ENR 	|= 0x00000080;	// RCC_AHB1ENR GPIOH Enable
	RCC->AHB1ENR	|= 0x00000100;	// RCC_AHB1ENR GPIOI Enable
        RCC->APB2ENR 	|= 0x4000;	// Enable System Configuration Controller Clock
	
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH PIN8~PIN15 Input mode (reset state)
        GPIOI->MODER	&= ~0x0000C000;	// GPIOI (J-D) : Input mode (reset state)                            
        
        SYSCFG->EXTICR[1] &= ~0x8000;   //�ʱ�ȭ
        SYSCFG->EXTICR[2] &= ~0x7077;   //�ʱ�ȭ
        SYSCFG->EXTICR[3] &= ~0x7777;   //�ʱ�ȭ

	SYSCFG->EXTICR[1] |= 0x8000;	// EXTI 7 �� ���� �ҽ� �Է��� GPIOI�� ����
        SYSCFG->EXTICR[2] |= 0x7077;	// EXTI 8,9,11 �� ���� �ҽ� �Է��� GPIOH�� ����
        SYSCFG->EXTICR[3] |= 0x7777;    // EXTI 12,13,14,15 �� ���� �ҽ� �Է��� GPIOH�� ����
	
        EXTI->FTSR |= 0xfb80;		//7~9 11~15 falling trigger ���� 
	EXTI->IMR  |= 0x0800;		// EXTI 11 ���ͷ�Ʈ mask (Interrupt Enable) ����
		
	NVIC->ISER[0] |= ( 1 << 23  );	// Enable 'Global Interrupt'  
        NVIC->ISER[1] |= ( 1 << 8  );	// Enable 'Global Interrupt'  
					
}

/* EXTI5~9 ���ͷ�Ʈ �ڵ鷯(ISR: Interrupt Service Routine) */
void EXTI9_5_IRQHandler(void)   
{		
	if(EXTI->PR & 0x0080)			// EXTI7 Interrupt Pending(�߻�) ����
	{                                                 
		EXTI->PR |= 0x0080;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
                GPIOG->ODR |= 0x04;             // LED2 on
                LCD_SetBrushColor(RGB_PINK);
                LCD_DrawFillRect(33, 25, 5, 5); // 2�� �� open
                LCD_DisplayText(2,11,"C");  	  // ���ۻ�Ȳ ǥ��
                BEEP(); //�Է� ����

                DelayMS(1000); //1���� ���� 0.5�ʰ������� 3�� on
                BEEP();
                DelayMS(500);
                BEEP();
                DelayMS(500);
                BEEP();
                
                DelayMS(3000); //3����
                BEEP_F();                             //Ż�� �����Ͽ� ���� 500 ms ON
                
                //�ʱ�ȭ �۾�
                EXTI->IMR  &= ~0x0100;
                _EXTI_Init();	// EXTI �ʱ�ȭ
                DisplayInitScreen();	// LCD �ʱ�ȭ��
                GPIOG->ODR &= ~0x00FF;	// �ʱⰪ: LED0~7 Off

                
                
	}
        else if(EXTI->PR & 0x0100)		// EXTI8 Interrupt Pending(�߻�) ����
	{
		EXTI->PR |= 0x0100;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)		
		GPIOG->ODR |= 0x01;		// LED0 on
                LCD_SetBrushColor(RGB_PINK);
                LCD_DrawFillRect(17, 25, 5, 5); // 0�� �� open
                BEEP();//�Է� ����
                EXTI->IMR  &= ~0x0100;         //���ͷ�Ʈ ����ũ disable
                EXTI->IMR  |= 0x0200;           //���� ���ͷ�Ʈ ����ũ enable
	}	

        else if(EXTI->PR & 0x0200)		// EXTI9 Interrupt Pending(�߻�) ����
	{
		EXTI->PR |= 0x0200;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)		
		GPIOG->ODR |= 0x02;		// LED1 on
                LCD_SetBrushColor(RGB_PINK);
                LCD_DrawFillRect(24, 25, 5, 5); // 1�� �� open
                BEEP();//�Է� ����
                EXTI->IMR  &= ~0x0200;         //���ͷ�Ʈ ����ũ disable
                EXTI->IMR  |= 0x0080;           //���� ���ͷ�Ʈ ����ũ enable
        }
}

void EXTI15_10_IRQHandler(void)
{
             if(EXTI->PR & 0x0800)              // EXTI11 Interrupt Pending(�߻�) ����
            {                                                  
                    EXTI->PR |= 0x0800;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)                                                        
                    GPIOG->ODR |= 0x08;         // LED3 on        		                                     
                    LCD_SetBrushColor(RGB_PINK);
                    LCD_DrawFillRect(41,25,5,5); // 3�� �� open
                    LCD_DisplayText(2,11,"W");  	  // ���ۻ�Ȳ ǥ��
                    BEEP();//�Է� ����
                    EXTI->IMR  &= ~0x0800;         //���ͷ�Ʈ ����ũ disable
                    EXTI->IMR  |= 0x1000;           //���� ���ͷ�Ʈ ����ũ enable
            }

            else if(EXTI->PR & 0x1000)          // EXTI12 Interrupt Pending(�߻�) ����
            {
                    EXTI->PR |= 0x1000;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)                                        
                    GPIOG->ODR |= 0x10;         // LED4 on
                    LCD_SetBrushColor(RGB_PINK);
                    LCD_DrawFillRect(49, 25, 5, 5); // 4�� �� open
                    BEEP();//�Է� ����
                    EXTI->IMR  &= ~0x1000;         //���ͷ�Ʈ ����ũ disable
                    EXTI->IMR  |= 0x2000;           //���� ���ͷ�Ʈ ����ũ enable
            }
            else if(EXTI->PR & 0x2000)          // EXTI13 Interrupt Pending(�߻�) ����
            {
                    EXTI->PR |= 0x2000;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)             
                    GPIOG->ODR |= 0x20;         // LED5 on			
                    LCD_SetBrushColor(RGB_PINK);
                    LCD_DrawFillRect(57, 25, 5, 5); // 5�� �� open
                    BEEP();//�Է� ����
                    EXTI->IMR  &= ~0x2000;         //���ͷ�Ʈ ����ũ disable
                    EXTI->IMR  |= 0x4000;           //���� ���ͷ�Ʈ ����ũ enable
            }
            else if(EXTI->PR & 0x4000)          // EXTI14 Interrupt Pending(�߻�) ����
            {                                            
                    EXTI->PR |= 0x4000;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)                                        
                    GPIOG->ODR |= 0x40;	        // LED6 on
                    LCD_SetBrushColor(RGB_PINK);
                    LCD_DrawFillRect(66, 25, 5, 5); // 6�� �� open
                    BEEP();//�Է� ����
                    EXTI->IMR  &= ~0x4000;         //���ͷ�Ʈ ����ũ disable
                    EXTI->IMR  |= 0x8000;           //���� ���ͷ�Ʈ ����ũ enable
            }
            else if(EXTI->PR&0x8000)            // EXTI15 Interrupt Pending(�߻�) ����
            {
                    EXTI->PR |= 0x8000;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)                                        
                    GPIOG->ODR |= 0x80;         // LED7 on	
                    LCD_SetBrushColor(RGB_PINK);
                    LCD_DrawFillRect(74, 25, 5, 5); // 7�� �� open
                    BEEP();//�Է� ����
                    EXTI->IMR  &= ~0x8000;         //���ͷ�Ʈ ����ũ disable
                    EXTI->IMR  |= 0x0100;           //���� ���ͷ�Ʈ ����ũ enable
            }


}

/* Switch�� �ԷµǾ����� ���ο� � switch�� �ԷµǾ������� ������ return�ϴ� �Լ�  */ 
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

void BEEP_F(void)	//Ż�� ���� ���� 500 ms on		
{ 	
	GPIOF->ODR |=  0x0200;	// PF9 'H' Buzzer on
	DelayMS(500);		// Delay 500 ms
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
