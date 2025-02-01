//**************************************************************************//
// GLCD �ǽ�(����, �׸�)
// 	LCD_Init()(calling in GLCD_Display.c, called in GLCD.c)
//		ILI_Init() in calling in GLCD.c, called in ILI3168B.c
//			FSMC_GPIO_Config() in ILI3168B.c   
//			FSMC_LCD_Init() in ILI3168B.c
//	DisplayInitScreen();

#include "stm32f4xx.h"
#include "GLCD.h"
#define RGB_PURPLE GET_RGB(255,0,255)
void _GPIO_Init(void);
uint16_t KEY_Scan(void);

void BEEP(void);
void DisplayInitScreen(void);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

uint8_t	SW0_Flag, SW1_Flag; 	// Switch �Է��� Ȧ����°����? ¦����°����? �� �˱����� ���� 

int main(void)
{
	_GPIO_Init(); 		// GPIO (LED, SW, Buzzer) �ʱ�ȭ
	LCD_Init();		// LCD ��� �ʱ�ȭ
	DelayMS(100);
	BEEP();

	GPIOG->ODR &= ~0x00FF;	// LED �ʱⰪ: LED0~7 Off
	DisplayInitScreen();	// LCD �ʱ�ȭ��
 
	/* ����ǥ�ÿ����� ���� ����ϴ� �Լ��� */ 
	//void LCD_DrawHorLine(UINT16 x, UINT16 y, UINT16 width)  
	//void LCD_DrawVerLine(UINT16 x, UINT16 y, UINT16 height)
	//void LCD_DrawRectangle(UINT16 x, UINT16 y, UINT16 width, UINT16 height)  
	//void LCD_DrawFillRect(UINT16 x, UINT16 y, UINT16 width, UINT16 height)  
	//void LCD_DrawLine(UINT16 x1, UINT16 y1, UINT16 x2, UINT16 y2)
//	
//        LCD_SetTextColor(RGB_YELLOW);	// ���ڻ� : Black
//        LCD_SetBackColor(RGB_BLUE);
//        LCD_DisplayText(3,3,"TU KOREA");
//        LCD_DisplayChar(4,0,'A');
//        unsigned char i;
        //i = 0;
        
	while(1)
	{
		switch(KEY_Scan())	// �Էµ� Switch ���� �з� 
		{
			case 0xFE00 : 	//SW0
				if (SW0_Flag==0) {
					GPIOG->ODR |= 0x0001;		// LED0 ON		
					LCD_DisplayText(1,5,"ON ");	// Text(String) "ON " ǥ��
					SW0_Flag = 1;
				}
				else {
					GPIOG->ODR &= ~0x0001;		// LED0 OFF
					LCD_DisplayText(1,5,"OFF");	// Text(String) "OFF" ǥ��
					SW0_Flag = 0;
				}
			break;
			case 0xFD00 : 	//SW1
				if (SW1_Flag==0) {
					GPIOG->BSRRL = 0x0002;		// LED1 ON		
					LCD_DisplayChar(2,5,'O');	// Charater 'O' ǥ��
					SW1_Flag = 1;
				}
				else {
					GPIOG->BSRRH = 0x0002;		// LED1 OFF
					LCD_DisplayChar(2,5,0x58);	// Charater 'X' ǥ��
									// 'X'=0x58 (ASCII code)
					SW1_Flag = 0;
				}	
			break;
//                        
//                        case 0x7F00:
//                              i++;
//                              if(i==10) i=0;
//                              LCD_DisplayChar(4,2,i+0x30);
//                          break;
		}  // switch(KEY_Scan())
	}  // while(1)
}

/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer)) �ʱ� ����	*/
void _GPIO_Init(void)
{
	// LED (GPIO G) ���� : Output mode
	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	&=  ~0x0000FFFF;	// GPIOG 0~7 : Clear (0b00)						
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						

	GPIOG->OTYPER	&= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	

	GPIOG->OSPEEDR &= ~0x0000FFFF;	// GPIOG 0~7 : Clear (0b00) 
	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
   
	// SW (GPIO H) ���� : Input mode 
	RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	// Buzzer (GPIO F) ���� : Output mode
	RCC->AHB1ENR	|=  0x00000020;	// RCC_AHB1ENR : GPIOF(bit#5) Enable							

	GPIOF->MODER 	&= ~0x000C0000;	// GPIOF 9 : Clear (0b00)						
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						

	GPIOF->OTYPER 	&= ~0x0200;	// GPIOF 9 : Push-pull  	

	GPIOF->OSPEEDR 	&= ~0x000C0000;	// GPIOF 9 : Clear (0b00) 
	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 
}

/* GLCD �ʱ�ȭ�� ���� �Լ� */
void DisplayInitScreen(void)
{
	LCD_Clear(RGB_WHITE);		// ȭ�� Ŭ����
	LCD_SetFont(&Gulim8);		// ��Ʈ : ���� 8
	LCD_SetBackColor(RGB_GREEN);	// ���ڹ��� : Green
	LCD_SetTextColor(RGB_BLACK);	// ���ڻ� : Black
	LCD_DisplayText(0,0,"GLCD Test");	// Title

	LCD_SetBackColor(RGB_YELLOW);	//���ڹ��� : Yellow
          
	LCD_DisplayText(1,0,"LED0 OFF");
	LCD_DisplayText(2,0,"LED1 X");

	/* �׸�������� ���� ����ϴ� �Լ��� */
	//void LCD_SetPenColor(UINT32 Color); //���(�׸����)����
	//void LCD_DrawHorLine(UINT16 x, UINT16 y, UINT16 width)  // ���� �׸���
	//void LCD_DrawVerLine(UINT16 x, UINT16 y, UINT16 height) // ������ �׸���
	//void LCD_DrawRectangle(UINT16 x, UINT16 y, UINT16 width, UINT16 height) // �簢�� �׸���  
	//void LCD_DrawFillRect(UINT16 x, UINT16 y, UINT16 width, UINT16 height)  // �簢��(fille) �׸���
	//void LCD_DrawLine(UINT16 x1, UINT16 y1, UINT16 x2, UINT16 y2)  // ���� �׸���
    	
	/* �׸��� �׽�Ʈ�� ����� */
//	LCD_SetPenColor(RGB_RED);
//	LCD_DrawHorLine(0, 60, 100);   //�ȼ� ����
//	LCD_SetPenColor(RGB_BLUE);
//	LCD_DrawVerLine(0, 60, 110);
//	LCD_SetPenColor(RGB_RED);
//	LCD_DrawRectangle(10, 70, 30, 20);
//	LCD_SetBrushColor(RGB_GRAY);
//	LCD_DrawFillRect(50, 70, 40, 30);
//	LCD_SetPenColor(RGB_BLACK);  
//	LCD_DrawLine(0, 60, 100, 100);
        LCD_SetPenColor(RGB_RED);
        LCD_DrawVerLine(30, 40, 60);
        LCD_SetPenColor(RGB_BLUE);
        LCD_DrawLine(30, 60, 70, 40);
        LCD_DrawLine(50, 50, 70, 100);
        LCD_SetPenColor(RGB_RED);
        LCD_DrawRectangle(30, 100, 40, 5);
        LCD_SetBrushColor(RGB_PURPLE);
        LCD_DrawFillRect(30, 100, 40, 5);
        
}

/* Switch�� �ԷµǾ������� ���ο� � switch�� �ԷµǾ������� ������ return�ϴ� �Լ�  */ 
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
