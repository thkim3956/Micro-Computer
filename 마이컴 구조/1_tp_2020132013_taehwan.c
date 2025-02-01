/////////////////////////////////////////////////////////////
 // ������: Ŀ�� �ڵ� �Ǹű�
 // ��������:   Ŀ�Ǹ� �ڵ����� ���� �� �Ǹ��ϴ� Ŀ�����Ǳ� ���� ���α׷��� �ۼ���
 // ����� �ϵ����(���): GPIO, Joy-stick, EXTI, GLCD, LED, Buzzer,Fram
 // ������: 2024. 6. 15
 // ������ Ŭ����: ȭ���Ϲ�
// �й�: 2020132013
 // �̸�: ����ȯ
 ///////////////////////////////////////////////////////////////

#include "stm32f4xx.h"
#include "GLCD.h"
#include "FRAM.h"
#define RGB_GREY GET_RGB(128,128,128)    //GREY ���� RGB����

void _GPIO_Init(void);
void _EXTI_Init(void);

void DisplayInitScreen(void);
uint16_t KEY_Scan(void);
uint16_t JOY_Scan(void);
void BEEP(void);


void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

void change_text(void);           // ���ڸ� ���ڿ��� �ٲ��ִ� �Լ�

void inventory_display(void);   //��� ���� ȭ�� ���� �Լ�
void input_coin_display(void);  // ���� �Է� ȭ�� ����
void tot_display(void);     // tot ȭ�� ����
void NoC_display(void); // NoC ȭ�� ����
void working_display(void);   //���� ȭ�� �Լ�
void refill(void);                      // ��� refill ���ִ� �Լ�

void return_coin(void);  // IN coin reset
void clear(void); // NoC, TOT reset

uint8_t price;   // Ŀ�� ���� ���� Ȯ���� ���� ����

// �Է� ���� �� �� ����(fram read�� ����)
int input_coin;
int total_income;
int num_cf;


//���
uint8_t inventory_cup = 9;
uint8_t inventory_sugar = 5;
uint8_t inventory_milk = 5;
uint8_t inventory_coffee = 9;

//���ڸ� ���ڿ��� �ٲ��ְ� �̰��� display�� �� ����ϱ� ���� �Լ�
char coin[10];
char total[10];
char num_coffee[10];

int main(void)
{
	LCD_Init();	// LCD ��� �ʱ�ȭ
	DelayMS(10);
	_GPIO_Init();	// GPIO �ʱ�ȭ
	_EXTI_Init();	// EXTI �ʱ�ȭ

	Fram_Init();            // FRAM �ʱ�ȭ H/W �ʱ�ȭ
	Fram_Status_Config();   // FRAM �ʱ�ȭ S/W �ʱ�ȭ
        
        
        input_coin = Fram_Read(602);   // input coin`
        if(input_coin>200 || input_coin % 10 !=0) input_coin = 0;  // fram���� �о� �� �� 200 �̻�(or 1�� �ڸ��� ������ ���� �� �ִ� ���)�̸� 0���� ǥ��

        total_income = Fram_Read(603);  // tot
        if(total_income>99) total_income = 0;  // fram���� �о� �� �� 990�̻��̸� 0���� ǥ��
        
        num_cf = Fram_Read(604);          // noc
        if(num_cf > 50) num_cf = 0; // fram���� �о� �� �� 50 �̻��̸� 0���� ǥ��

	DisplayInitScreen();	// LCD �ʱ�ȭ��
	GPIOG->ODR &= ~0x00FF;	// �ʱⰪ: LED0~7 Off
         
        
        
        
	while (1)
	{
                if(inventory_sugar== 0 || inventory_cup== 0 || inventory_milk == 0 || inventory_coffee == 0) //��� �ϳ��� 0�̸��� ���� �Է� �ȵ�               
                {
                      EXTI->IMR  &= ~0x0300;		// EXTI 8, 9 (���� �Է�) ���ͷ�Ʈ mask (Interrupt disable) ����
                }

                if(inventory_sugar> 0&& inventory_cup> 0 && inventory_milk > 0 && inventory_coffee > 0) //��� ���� 0�̻��̸� ���� �Է� ����               
                {
                      EXTI->IMR  |= 0x0300;		// EXTI 8, 9 (���� �Է�) ���ͷ�Ʈ mask (Interrupt Enable) ����
                }

                if(inventory_sugar> 0 && inventory_cup> 0 && inventory_milk > 0 && inventory_coffee > 0) //��� ��� 1 �̻��̾��  ���� ����
                {          
                      switch (JOY_Scan())
                      {
                      case 0x01E0:	// NAVI_LEFT: Black ����
                              BEEP();   //��� ���� Buzzer
                              LCD_SetBackColor(RGB_BLUE);	// ���ڹ��� : BLUE
                              LCD_SetTextColor(RGB_RED);	// ���ڻ� : RED
                              LCD_DisplayText(3,1,"B");

                              LCD_SetBackColor(RGB_BLUE);	// ���ڹ��� : blue
                              LCD_SetTextColor(RGB_WHITE);	// ���ڻ� : white                        
                              LCD_DisplayText(3,3,"M");
                              LCD_DisplayText(2,2,"S"); 
                              price =  10;    // black ����
                              break;

                      case 0x03A0:	// NAVI_UP: Sugar ����
                              BEEP();   //��� ���� Buzzer
                              LCD_SetBackColor(RGB_BLUE);	// ���ڹ��� : BLUE
                              LCD_SetTextColor(RGB_RED);	// ���ڻ� : RED
                              LCD_DisplayText(2,2,"S");

                              LCD_SetBackColor(RGB_BLUE);	// ���ڹ��� : blue
                              LCD_SetTextColor(RGB_WHITE);	// ���ڻ� : white                        
                              LCD_DisplayText(3,3,"M");
                              LCD_DisplayText(3,1,"B"); 
                              price =  20;    // Sugar ����

                              break;

                      case 0x02E0:	// NAVI_RIGHT: MIX ����
                              BEEP();
                              LCD_SetBackColor(RGB_BLUE);	// ���ڹ��� : BLUE
                              LCD_SetTextColor(RGB_RED);	// ���ڻ� : RED
                              LCD_DisplayText(3,3,"M");

                              LCD_SetBackColor(RGB_BLUE);	// ���ڹ��� : blue
                              LCD_SetTextColor(RGB_WHITE);	// ���ڻ� : white                        
                              LCD_DisplayText(3,1,"B");
                              LCD_DisplayText(2,2,"S"); 
                              price =  30;    // MIX ����

                              break;			
                      }
                      
                      
                      if (KEY_Scan() ==  0xFB00) // SW2: Working   
                      {                    
                            if(price == 10)  //black coffee ����
                            {
                                  
                                    if(input_coin >= 10)        // in ���� 10�� �̻��� �� ���� ����
                                    {

                                        working_display();          // Ŀ�� �ӽ� ���� �Լ�
                                        input_coin -= 10;         // coin -10
                                        inventory_cup--;     // �� ��� -1
                                        inventory_coffee-- ; //Ŀ�� ��� -1
                                        num_cf++;                      // Ŀ�� �Ǹ� �� ����
                                        total_income+= 1;              // �� ����  +10                                                             
                                                                            
                                        inventory_display();   //��� ���� ����
                                        input_coin_display();  //IN ���� ����
                                        tot_display();              // TOT ���� ����
                                        NoC_display();           // NoC ���� ����
                                        EXTI->IMR  |= 0x3b00;		// EXTI 8, 9, 11, 12, 13 ���ͷ�Ʈ mask (Interrupt Enable) ����
                                    }
                                
                                
                            }

                            else if(price == 20) // sugar coffee
                            {
                                
                                    if(input_coin >= 20)        // in ���� 20�� �̻��� �� ���� ����
                                    {

                                        working_display();         // Ŀ�� �ӽ� ���� �Լ�
                                        input_coin -= 20;         // coin -20
                                        inventory_cup--;     // �� ��� -1
                                        inventory_coffee-- ; //Ŀ�� ��� -1
                                        inventory_sugar-- ; //������� -1
                                        num_cf++;                      // Ŀ�� �Ǹ� �� ����
                                        total_income+= 2;              // �� ����  +20                                                             
                                                                            
                                        inventory_display();   //��� ���� ����
                                        input_coin_display();  //IN ���� ����
                                        tot_display();             //TOT ���� ����
                                        NoC_display();         //NoC ���� ����
                                        EXTI->IMR  |= 0x3b00;		// EXTI 8, 9, 11, 12, 13 ���ͷ�Ʈ mask (Interrupt Enable) ����
                                    }
                                
                            }

                            else if(price == 30)  // Mix coffee
                            {
                                
                                    if(input_coin >= 30)   // in ���� 30�� �̻��� �� ���� ����
                                    {

                                        working_display();         //Ŀ�� �ӽ� ���� �Լ�
                                        input_coin -= 30;         // coin -20
                                        inventory_cup--;     // �� ��� -1
                                        inventory_coffee-- ; //Ŀ�� ��� -1
                                        inventory_sugar-- ; //������� -1
                                        inventory_milk--;      // ���� ��� -1
                                        num_cf++;                      // Ŀ�� �Ǹ� �� ����
                                        total_income+= 3;              // �� ����  +30                                                             
                                                                            
                                        inventory_display();   //��� ���� ����
                                        input_coin_display();  //IN ���� ����
                                        tot_display();             //TOT ���� ����
                                        NoC_display();         //NoC ���� ����
                                        EXTI->IMR  |= 0x3b00;		// EXTI 8, 9, 11, 12, 13 ���ͷ�Ʈ mask (Interrupt Enable) ����
                                    }
                                
                            }
                             
                             
                      }
                } 
	}
}

/* GLCD �ʱ�ȭ�� ���� �Լ� */
void DisplayInitScreen(void)
{
        change_text();              // ���÷��̿� �Է��ϱ� ���� ���ڸ� ���ڿ��� ��ȭ
	LCD_Clear(RGB_WHITE);		// ȭ�� Ŭ����
	LCD_SetFont(&Gulim8);		// ��Ʈ : ���� 8
	LCD_SetBackColor(RGB_YELLOW);	// ���ڹ��� : Yellow
	LCD_SetTextColor(RGB_BLACK);	// ���ڻ� : BLACK
	LCD_DisplayText(1,1,"KTH coffe"); // Title
        LCD_SetPenColor(RGB_BLACK);
        LCD_DrawRectangle(7, 12, 73, 12);     
        LCD_SetBackColor(RGB_WHITE);	// ���ڹ��� : White
        
        LCD_DisplayText(0,15,"IN");             // IN display
        LCD_DisplayText(3,15,"TOT");             // TOT display
        LCD_DisplayText(6,15,"RF");             // RF display
        LCD_DisplayText(7,15,"NoC");             // NoC display
        LCD_DisplayText(6,1,"cp sg mk cf");    // cp sg mk cf display
        LCD_DisplayText(1,11,"\\10");      // \10 display
        LCD_DisplayText(3,11,"\\50");     // 50 display
        LCD_DisplayChar(5,1,inventory_cup+0x30);  // ��� ���� display
        LCD_DisplayChar(5,4,inventory_sugar+0x30);
        LCD_DisplayChar(5,7,inventory_milk+0x30);
        LCD_DisplayChar(5,10,inventory_coffee+0x30);
        
        LCD_SetBackColor(RGB_BLUE);	// ���ڹ��� : blue
	LCD_SetTextColor(RGB_WHITE);	// ���ڻ� : white
        LCD_DisplayText(3,1,"B");
        LCD_DisplayText(3,3,"M");
        LCD_DisplayText(2,2,"S"); 
        
        LCD_SetBackColor(RGB_RED);	// ���ڹ��� : RED
	LCD_SetTextColor(RGB_WHITE);	// ���ڻ� : white     
        LCD_DisplayText(4,2,"W");
        
        LCD_SetPenColor(RGB_GREEN);       // �ڽ� �׵θ� ���
        LCD_DrawRectangle(7, 38, 9, 13);     // B �ڽ�
        LCD_DrawRectangle(23, 38, 9, 13);   // M �ڽ�
        LCD_DrawRectangle(15, 26, 9, 13);   // S �ڽ�
        LCD_DrawRectangle(15, 51, 9, 13);   // W �ڽ�
        LCD_DrawRectangle(100, 26, 8, 8);   // 10�� �ڽ�
        LCD_DrawRectangle(100, 52, 8, 8);   // 50�� �ڽ�
        LCD_DrawRectangle(7, 64, 9, 13);     // �� ��� �ڽ�
        LCD_DrawRectangle(31, 64, 9, 13);     // ������� �ڽ�
        LCD_DrawRectangle(55, 64, 9, 13);     // ���� ��� �ڽ�
        LCD_DrawRectangle(79, 64, 9, 13);     // Ŀ�� ��� �ڽ�
        LCD_DrawRectangle(136, 77, 9, 12);     // RF�ڽ�
        LCD_DrawRectangle(119, 12, 25, 13);   // IN �ڽ�
        LCD_DrawRectangle(119, 51, 25, 13);   //TOT �ڽ�
        LCD_DrawRectangle(127, 103, 17, 13);   // NoC �ڽ�
        
        LCD_SetBackColor(RGB_BLACK);	// ���ڹ��� : Black
	LCD_SetTextColor(RGB_YELLOW);	// ���ڻ� : Yellow
        LCD_DisplayText(1,15,coin);  //  IN  ����

        LCD_DisplayText(4,15,total);  // TOT  ����
        LCD_DisplayText(4,17,"0");   //TOT ����  1���ڸ� ���ڴ� 0���� �����Ǿ� �����Ƿ� ����

        LCD_SetBackColor(RGB_YELLOW);	// ���ڹ��� : Yellow
	LCD_SetTextColor(RGB_BLACK);	// ���ڻ� : Black
        LCD_DisplayText(8,16,num_coffee);  // NoC ����
        
        LCD_SetBrushColor(RGB_GREY);
        LCD_DrawFillRect(101, 27, 7, 7);        // 10�� �ڽ� ȸ��
        LCD_DrawFillRect(101, 53, 7, 7);        // 50�� �ڽ� ȸ��
        
        LCD_SetBrushColor(RGB_GREEN);
        LCD_DrawFillRect(137, 78, 8, 11);        // RF�ڽ� ���
        
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

/* EXTI (EXTI8(GPIOH.8, SW0), EXTI9(GPIOH.9, SW1)), EXTI11(GPIOH.11, SW3)), EXTI12(GPIOH.12, SW4)), EXTI13(GPIOH.13, SW5)) �ʱ� ����  */
void _EXTI_Init(void)
{
	RCC->AHB1ENR 	|= 0x0080;	// RCC_AHB1ENR GPIOH Enable
	
        RCC->APB2ENR 	|= 0x4000;	// Enable System Configuration Controller Clock
	
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH PIN8~PIN15 Input mode (reset state)
                                  
        
        
        SYSCFG->EXTICR[2] &= ~0x7077;   //�ʱ�ȭ
        SYSCFG->EXTICR[3] &= ~0x7777;   //�ʱ�ȭ

	
        SYSCFG->EXTICR[2] |= 0x7077;	// EXTI 8,9,11 �� ���� �ҽ� �Է��� GPIOH�� ����
        SYSCFG->EXTICR[3] |= 0x0077;    // EXTI 12,13 �� ���� �ҽ� �Է��� GPIOH�� ����
	
        EXTI->FTSR |= 0x3b00;		//8, 9, 11, 12, 13 falling trigger ���� 
	EXTI->IMR  |= 0x3b00;		// EXTI 8, 9, 11, 12, 13 ���ͷ�Ʈ mask (Interrupt Enable) ����
		
	NVIC->ISER[0] |= ( 1 << 23  );	// Enable 'Global Interrupt'  
        NVIC->ISER[1] |= ( 1 << 8  );	// Enable 'Global Interrupt'  
					
}



/* EXTI5~9 ���ͷ�Ʈ �ڵ鷯(ISR: Interrupt Service Routine) */
void EXTI9_5_IRQHandler(void)   
{		
                
		if(EXTI->PR & 0x0100)		// SW0(EXTI8): Coin 10�� �Է�
		{
			EXTI->PR |= 0x0100;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)                       
                        BEEP();                        
                        LCD_SetBrushColor(RGB_YELLOW);
                        LCD_DrawFillRect(101, 27, 7, 7);        // 10�� �ڽ� ���
                        
                        input_coin+= 10;                            // coin 10�� �߰�
                        input_coin_display();
                        
                        DelayMS(1000);

                        LCD_SetBrushColor(RGB_GREY);
                        LCD_DrawFillRect(101, 27, 7, 7);        // 10�� �ڽ� ȸ��
                			

        
        
		}	

        else if(EXTI->PR & 0x0200)	//SW1(EXTI9) : Coin 50�� �Է�
		{
			EXTI->PR |= 0x0200;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)		
                        BEEP();
                        LCD_SetBrushColor(RGB_YELLOW);
                        LCD_DrawFillRect(101, 53, 7, 7);        // 50�� �ڽ� ���

                        input_coin += 50;     // Coin 50�� �߰�
                                  

                        input_coin_display();   // Coin �߰� �� IN�ڽ� �� ���÷��� ���� ����
                        
                        DelayMS(1000);

                        LCD_SetBrushColor(RGB_GREY);
                        LCD_DrawFillRect(101, 53, 7, 7);        // 50�� �ڽ� ȸ��
                 }
}

void EXTI15_10_IRQHandler(void)
{
             if(EXTI->PR & 0x0800)              // SW3(EXTI11): �ܵ���ȯ
            {                                                  
                    EXTI->PR |= 0x0800;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
                    return_coin();                    // �ܵ� ��ȯ �Լ�               
                    
            }

            else if(EXTI->PR & 0x1000)          // SW4(EXTI12): RF(Cup, Sugar, Milk, Coffee����)
            {
                    EXTI->PR |= 0x1000;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)                  
                    refill();                              // ��� ���� �Լ�

            }
            else if(EXTI->PR & 0x2000)          // SW5(EXTI13)�Է½� NoC, TOT ���� ��0������ ����
            {
                    EXTI->PR |= 0x2000;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)             
                    clear();                              // NoC, TOT �� ���� �Լ�

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
void change_text(void)   //���ڸ� ���ڿ��� �ٲ��ִ� �Լ�
{
        sprintf(coin,"%03d",input_coin);
        sprintf(total,"%02d",total_income);
        sprintf(num_coffee,"%02d",num_cf);
}

void inventory_display(void)    // ��� 0�� �Ǹ� RF�ڽ� �������� �� ��� ���� �Լ�
{
        if(inventory_sugar<=0)
        {
          
          LCD_SetBrushColor(RGB_RED);
          LCD_DrawFillRect(137, 78, 8, 11);         // RF�ڽ� ����
        }
        else if(inventory_coffee <= 0)
        {
          
          LCD_SetBrushColor(RGB_RED);
          LCD_DrawFillRect(137, 78, 8, 11);         // RF�ڽ� ����
        }

        else if(inventory_cup <= 0)
        {
          
          LCD_SetBrushColor(RGB_RED);
          LCD_DrawFillRect(137, 78, 8, 11);         // RF�ڽ� ����
        }

        else if(inventory_milk <= 0)
        {
          
          LCD_SetBrushColor(RGB_RED);
          LCD_DrawFillRect(137, 78, 8, 11);         // RF�ڽ� ����
        }
        else
        {
          LCD_SetBrushColor(RGB_GREEN);
          LCD_DrawFillRect(137, 78, 8, 11);         // RF�ڽ� green
        }       

        LCD_SetBackColor(RGB_WHITE);	// ���ڹ��� : White
	LCD_SetTextColor(RGB_BLACK);	// ���ڻ� : BLACK
        LCD_DisplayChar(5,1,inventory_cup+0x30);
        LCD_DisplayChar(5,4,inventory_sugar+0x30);
        LCD_DisplayChar(5,7,inventory_milk+0x30);
        LCD_DisplayChar(5,10,inventory_coffee+0x30);

}

void input_coin_display(void)   // IN fram ���� �� ���� �Է� ȭ�� ����
{
        if(input_coin >=200)   //IN�� 200�̻��� �� 200���� ����
        {
            input_coin = 200;
            
        }        
        Fram_Write(602, input_coin);    // IN coin �� Fram�� ����
        change_text();                              // ���÷��̿� �Է��ϱ� ���� ���ڸ� ���ڿ��� ��ȯ
        LCD_SetBackColor(RGB_BLACK);	// ���ڹ��� : Black
        LCD_SetTextColor(RGB_YELLOW);	// ���ڻ� : Yellow
        LCD_DisplayText(1,15, coin);
}

void tot_display(void)  // TOT fram ���� �� ���÷��� ����
{
        if(total_income > 99)   // TOT�� 990�̻��� �� 990���� ����
        {
            total_income = 99;                      
        }        

        Fram_Write(603,total_income);          //TOT �� Fram�� ����
        change_text();                              // ���÷��̿� �Է��ϱ� ���� ���ڸ� ���ڿ��� ��ȯ        

        LCD_SetBackColor(RGB_BLACK);	// ���ڹ��� : Black
        LCD_SetTextColor(RGB_YELLOW);	// ���ڻ� : Yellow
        LCD_DisplayText(4,15,total);
}

void NoC_display(void)   // NoC fram ���� �� ���÷��� ����
{

        if(num_cf >= 50)   // Noc�� 50�̻��� �� 50���� ����
        {
           num_cf = 50;           
        }
        

        Fram_Write(604,num_cf);             // NoC�� Fram�� ����
        change_text();                              // ���÷��̿� �Է��ϱ� ���� ���ڸ� ���ڿ��� ��ȯ
        LCD_SetBackColor(RGB_YELLOW);	// ���ڹ��� : Yellow
	LCD_SetTextColor(RGB_BLACK);	// ���ڻ� : Black
        LCD_DisplayText(8,16,num_coffee);  
}

void working_display(void)
{
        EXTI->IMR  &= ~0x3b00;		// EXTI 8, 9, 11, 12, 13 ���ͷ�Ʈ mask (Interrupt disable) ����    working�߿��� ��� ���ͷ�Ʈ unmask
        GPIOG->ODR |= 0x00FF;          // LED 0~7 ON
        BEEP();     //���� Buzzer
        LCD_SetBackColor(RGB_RED);	// ���ڹ��� : RED
	LCD_SetTextColor(RGB_WHITE);	// ���ڻ� : white
        LCD_DisplayText(4,2,"0");             // 3�ʰ� ���� ���� ���� ���÷��� ǥ��
        DelayMS(1000);
        LCD_DisplayText(4,2,"1");
        DelayMS(1000);
        LCD_DisplayText(4,2,"2");
        DelayMS(1000);
        LCD_DisplayText(4,2,"W");
        GPIOG->ODR &= ~0x00FF;    // LED 0~7 OFF


        // ���� �� Ŀ�� ���� ���� �� ���÷��� �ʱ�ȭ
        LCD_SetBackColor(RGB_BLUE);	// ���ڹ��� : blue
        LCD_SetTextColor(RGB_WHITE);	// ���ڻ� : white                        
        LCD_DisplayText(3,3,"M");
        LCD_DisplayText(3,1,"B"); 
        LCD_DisplayText(2,2,"S"); 
        price = 0;

        BEEP();   // ���� Buzzer�� 3�� ����
        DelayMS(500);
        BEEP();
        DelayMS(500);
        BEEP();
        
}

void return_coin(void)
{
        input_coin = 0;
        Fram_Write(602, input_coin);        
        change_text();                              // ���÷��̿� �Է��ϱ� ���� ���ڸ� ���ڿ��� ��ȯ                      
        LCD_SetBackColor(RGB_BLACK);	// ���ڹ��� : Black
        LCD_SetTextColor(RGB_YELLOW);	// ���ڻ� : Yellow
        LCD_DisplayText(1,15, coin);  

}

void clear(void)   // TOT, Noc �ʱ�ȭ
{
        total_income = 0;  // 0 ���� �ʱ�ȭ
        num_cf = 0;
        Fram_Write(604,num_cf);
        Fram_Write(603,total_income);

        change_text();                              // ���÷��̿� �Է��ϱ� ���� ���ڸ� ���ڿ��� ��ȯ
        
        LCD_SetBackColor(RGB_BLACK);	// ���ڹ��� : Black
        LCD_SetTextColor(RGB_YELLOW);	// ���ڻ� : Yellow
        LCD_DisplayText(4,15, total);  

        LCD_SetBackColor(RGB_YELLOW);	// ���ڹ��� : Yellow
	LCD_SetTextColor(RGB_BLACK);	// ���ڻ� : Black
        LCD_DisplayText(8,16,num_coffee);  
}

void refill(void)   // refill �Լ�
{
        int flag = 0;
        if(inventory_sugar==0)  //sugar ��� 0�϶� ����
        {
            inventory_sugar= 5;
            inventory_display();  //��� display����
            flag = 1;
        }
        if(inventory_coffee == 0)  //coffee��� 0�϶� ����
        {
            inventory_coffee = 9; 
            inventory_display();
            flag = 1;

        }
        if(inventory_cup == 0) //cup��� 0�϶� ����
        {
            inventory_cup = 9;
            inventory_display();
            flag = 1;

        }
        if(inventory_milk == 0) //milk��� 0�϶� ����
        {
            inventory_milk = 5;
            inventory_display();
            flag = 1;

        }
        if(flag == 1) // ����  ���ְ� buzzer�Ҹ�
        {
            BEEP();
            DelayMS(500);
            BEEP();
            flag = 0;
        } 
        
}


