#include "stm32f4xx.h"
#include "GLCD.h"
#include "FRAM.h"
#define NAVI_PUSH	0x03C0	//PI5 0000 0011 1100 0000 
#define NAVI_UP		0x03A0	//PI6 0000 0011 1010 0000 
#define NAVI_DOWN	0x0360	//PI7 0000 0011 0110 0000 
#define NAVI_RIGHT	0x02E0	//PI8 0000 0010 1110 0000 
#define NAVI_LEFT	0x01E0	//PI9 0000 0001 1110 0000 

#define RGB_GREY GET_RGB(128,128,128)

void _GPIO_Init(void);
void _EXTI_Init(void);

void DisplayInitScreen(void);
uint16_t KEY_Scan(void);
uint16_t JOY_Scan(void);
void BEEP(void);

void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

uint8_t	SW0_Flag, SW1_Flag;

int price;
int in_coin;
int total_income;
int noc_num;

int cp = 9;
int sg = 5;
int mk = 5;
int cf = 9;

char in[10];
char tot[10];
char noc[10];

void change(void);
void in_display(void);
void tot_display(void);
void noc_display(void);
void inventory(void);
void working(void);
void refill(void);
void clear(void);
void return_coin(void);

int main(void)
{
  LCD_Init();	// LCD 모듈 초기화
  DelayMS(10);
  _GPIO_Init();	// GPIO (LED & SW) 초기화
  _EXTI_Init();	// EXTI 초기화
  Fram_Init();                    // FRAM 초기화 H/W 초기화
  Fram_Status_Config();   // FRAM 초기화 S/W 초기화

  in_coin = Fram_Read(602);
  total_income = Fram_Read(603);
  noc_num = Fram_Read(604);
  
  if(in_coin > 200) in_coin = 0;
  if(total_income > 99) total_income = 0;
  if(noc_num > 50) noc_num = 0;

  DisplayInitScreen();	// LCD 초기화면

  GPIOG->ODR &= ~0x00FF;		// 초기값: LED0~7 Off

  while(1)
  {
    if(cp <= 0 || sg <= 0 || mk <= 0 || cf <= 0) EXTI->IMR &= ~0x0300;
    if(cp > 0 && sg > 0 && mk > 0 && cf > 0) EXTI->IMR |= 0x0300;
    if(cp > 0 && sg > 0 && mk > 0 && cf > 0)
    {
      switch(JOY_Scan())
      {
        case NAVI_LEFT:
          price = 10;
          LCD_SetBackColor(RGB_BLUE);
          LCD_SetTextColor(RGB_RED);
          LCD_DisplayText(4,1,"B");
          LCD_SetBackColor(RGB_BLUE);
          LCD_SetTextColor(RGB_WHITE);
          LCD_DisplayText(4,3,"M");
          LCD_DisplayText(3,2,"S");
        break;
        case NAVI_RIGHT:
          price = 30;
          LCD_SetBackColor(RGB_BLUE);
          LCD_SetTextColor(RGB_RED);
          LCD_DisplayText(4,3,"M");
          LCD_SetBackColor(RGB_BLUE);
          LCD_SetTextColor(RGB_WHITE);
          LCD_DisplayText(3,2,"S");        
          LCD_DisplayText(4,1,"B");
        break;
        case NAVI_UP:
          price = 20;
          LCD_SetBackColor(RGB_BLUE);
          LCD_SetTextColor(RGB_RED);
          LCD_DisplayText(3,2,"S");
          LCD_SetBackColor(RGB_BLUE);
          LCD_SetTextColor(RGB_WHITE);
          LCD_DisplayText(4,3,"M");
          LCD_DisplayText(4,1,"B");
        break;
      }
      if(KEY_Scan() == 0xFb00)
      {
        if(price == 10)
        {
          if(in_coin >= 10)
          {
            in_coin -= 10;
            total_income +=1;
            noc_num++;
            cp--;
            cf--;
            working();
            inventory();
            in_display();
            tot_display();
            noc_display();
            EXTI->IMR  |= 0x3F00;
          }
        }
        else if(price == 20)
        {
          if(in_coin >= 20)
          {
            in_coin -= 20;
            total_income +=2;
            noc_num++;
            cp--;
            cf--;
            sg--;
            working();
            inventory();
            in_display();
            tot_display();
            noc_display();
            EXTI->IMR  |= 0x3F00;
          }
        }
        else if(price == 30)
        {
          if(in_coin >= 30)
          {
            in_coin -= 30;
            total_income +=3;
            noc_num++;
            cp--;
            cf--;
            sg--;
            mk--;
            working();
            inventory();
            in_display();
            tot_display();
            noc_display();
            EXTI->IMR  |= 0x3F00;
          }
        }
      }
    }
  }
}

/* GLCD 초기화면 설정 함수 */
void DisplayInitScreen(void)
{
  change();
  LCD_Clear(RGB_WHITE);		// 화면 클리어
  LCD_SetFont(&Gulim8);		// 폰트 : 굴림 8
  LCD_SetBackColor(RGB_YELLOW);	// 글자배경색 : yellow
  LCD_SetTextColor(RGB_BLACK);	// 글자색 : Black
  LCD_DisplayText(1,1,"KTH coffee");
  
  LCD_SetBackColor(RGB_WHITE);
  LCD_SetTextColor(RGB_BLACK);
  LCD_DisplayText(1,12,"\\10");
  LCD_DisplayText(3,12,"\\50");
  LCD_DisplayText(0,16,"IN");
  LCD_DisplayText(2,16,"TOT");
  LCD_DisplayText(5,16,"RF");
  LCD_DisplayText(6,16,"NoC");
  LCD_DisplayText(8,1,"cp sg mk cf");
  LCD_DisplayChar(7,1,cp+0x30);
  LCD_DisplayChar(7,4,sg+0x30);
  LCD_DisplayChar(7,7,mk+0x30);
  LCD_DisplayChar(7,10,cf+0x30);

  LCD_SetBackColor(RGB_BLUE);
  LCD_SetTextColor(RGB_WHITE);
  LCD_DisplayText(4,1,"B");
  LCD_DisplayText(4,3,"M");
  LCD_DisplayText(3,2,"S");

  LCD_SetBackColor(RGB_RED);
  LCD_SetTextColor(RGB_WHITE);
  LCD_DisplayText(5,2,"W");
  
  LCD_SetBackColor(RGB_BLACK);
  LCD_SetTextColor(RGB_YELLOW);
  LCD_DisplayText(1,16,in);
  LCD_DisplayText(3,16,tot);
  LCD_DisplayText(3,18,"0");

  LCD_SetBackColor(RGB_YELLOW);
  LCD_SetTextColor(RGB_BLACK);
  LCD_DisplayText(7,16,noc);

  LCD_SetPenColor(RGB_BLACK);
  LCD_DrawRectangle(7,12,81,13);//title

  LCD_SetPenColor(RGB_GREEN);
  LCD_DrawRectangle(7,51,9,13); //B
  LCD_DrawRectangle(23,51,9,13); //M
  LCD_DrawRectangle(15,38,9,13); //S
  LCD_DrawRectangle(15,64,9,13); //W
  LCD_DrawRectangle(127,12,25,13); //IN
  LCD_DrawRectangle(127,38,25,13); //TOT
  LCD_DrawRectangle(127,90,17,13); //NoC
  LCD_DrawRectangle(7,90,9,13); //cp
  LCD_DrawRectangle(31,90,9,13); //sg
  LCD_DrawRectangle(55,90,9,13); //mk
  LCD_DrawRectangle(79,90,9,13); //cf

  LCD_DrawRectangle(145,64,9,9); //rf
  LCD_DrawRectangle(100,27,9,9); //10
  LCD_DrawRectangle(100,53,9,9); //50

  LCD_SetBrushColor(RGB_GREY);
  LCD_DrawFillRect(101,28,8,8); //10
  LCD_DrawFillRect(101,54,8,8);

  LCD_SetBrushColor(RGB_GREEN);
  LCD_DrawFillRect(145,64,9,9); //rf
}

/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer)) 초기 설정	*/
void _GPIO_Init(void)
{
	// LED (GPIO G) 설정
	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
   
	// SW (GPIO H) 설정 
	RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	// Buzzer (GPIO F) 설정 
	RCC->AHB1ENR	|=  0x00000020; // RCC_AHB1ENR : GPIOF(bit#5) Enable							
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
	GPIOF->OTYPER 	&= ~0x0200;	// GPIOF 9 : Push-pull  	
	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 
//joystick
	RCC->AHB1ENR    |= 0x00000100;	// RCC_AHB1ENR GPIOI Enable
	GPIOI->MODER 	&=~0x000FFC00;	// GPIOI 5~9 : Input mode (reset state)
	GPIOI->PUPDR    &=~0x000FFC00;	// GPIOI 5~9 : Floating input (No Pull-up, pull-down) (reset state)
}	

/* EXTI (EXTI8(GPIOH.8, SW0), EXTI9(GPIOH.9, SW1)) 초기 설정  */
void _EXTI_Init(void)
{
	RCC->AHB1ENR 	|= 0x00000080;	// RCC_AHB1ENR GPIOH Enable
	RCC->APB2ENR 	|= 0x00004000;	// Enable System Configuration Controller Clock
	
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH PIN8~PIN15 Input mode (reset state)				 
	
	SYSCFG->EXTICR[2] |= 0x7777;	// EXTI8,9에 대한 소스 입력은 GPIOH로 설정
	SYSCFG->EXTICR[3] |= 0x0077;	// EXTI8 <- PH8, EXTI9 <- PH9 
					// EXTICR3(EXTICR[2])를 이용 
					// reset value: 0x0000	

	EXTI->FTSR |= 0x3F00;		// EXTI8: Falling Trigger Enable
	EXTI->IMR  |= 0x3F00;		// EXTI8,9 인터럽트 mask (Interrupt Enable) 설정
		
	NVIC->ISER[0] |= (1 << 23);	// 0x00800000
					// Enable 'Global Interrupt EXTI8,9'
					// Vector table Position 참조
        NVIC->ISER[1] |= (1 << 8);	// 0x00800000
}

/* EXTI5~9 인터럽트 핸들러(ISR: Interrupt Service Routine) */
void EXTI9_5_IRQHandler(void)		
{
	if(EXTI->PR & 0x0100)                   // EXTI8 Interrupt Pending(발생) 여부?
	{
		EXTI->PR |= 0x0100; 		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
                in_coin +=10;
                in_display();
                LCD_SetBrushColor(RGB_YELLOW);
                LCD_DrawFillRect(101,28,8,8); //10
                DelayMS(1000);
                LCD_SetBrushColor(RGB_GREY);
                LCD_DrawFillRect(101,28,8,8); //10
	}
	//EXTI Example(2) : SW1가 Low에서 High가 될 때(Rising edge Trigger mode) EXTI9 인터럽트 발생, LED1 toggle
	else if(EXTI->PR & 0x0200) 		// EXTI9 Interrupt Pending(발생) 여부?
	{
		EXTI->PR |= 0x0200; 		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
                in_coin +=50;
                in_display();
                LCD_SetBrushColor(RGB_YELLOW);
                LCD_DrawFillRect(101,54,8,8);
                DelayMS(1000);
                LCD_SetBrushColor(RGB_GREY);
                LCD_DrawFillRect(101,54,8,8);	
	}
}

void EXTI15_10_IRQHandler(void)
{
  if(EXTI->PR & 0x1000) //12 refill
  {
    EXTI-> PR |= 0x1000;
    refill();
    inventory();
  }
  else if(EXTI->PR & 0x2000) //13 clear
  {
    EXTI-> PR |= 0x2000;
    clear();
  }
  else if(EXTI->PR & 0x0800) //11 return coin
  {
    EXTI-> PR |= 0x0800;
    return_coin();
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
void change(void)
{
  sprintf(in,"%03d",in_coin);
  sprintf(tot,"%02d",total_income);
  sprintf(noc,"%02d",noc_num);
}
void in_display(void)
{
  if(in_coin>200) in_coin = 200;
  Fram_Write(602,in_coin);
  change();
  LCD_SetBackColor(RGB_BLACK);
  LCD_SetTextColor(RGB_YELLOW);
  LCD_DisplayText(1,16,in);
}
void tot_display(void)
{
  if(total_income>99) total_income = 99;
  Fram_Write(603,total_income);
  change();
  LCD_SetBackColor(RGB_BLACK);
  LCD_SetTextColor(RGB_YELLOW);
  LCD_DisplayText(3,16,tot);
}
void noc_display(void)
{
  if(noc_num>50) noc_num = 50;
  Fram_Write(604,noc_num);
  change();
  LCD_SetBackColor(RGB_YELLOW);
  LCD_SetTextColor(RGB_BLACK);
  LCD_DisplayText(7,16,noc);
}
void inventory(void)
{
  if(cp == 0)
  {
  LCD_SetBrushColor(RGB_RED);
  LCD_DrawFillRect(145,64,9,9); //rf
  }
  else if(sg == 0)
  {
  LCD_SetBrushColor(RGB_RED);
  LCD_DrawFillRect(145,64,9,9); //rf
  }
  else if(mk == 0)
  {
  LCD_SetBrushColor(RGB_RED);
  LCD_DrawFillRect(145,64,9,9); //rf
  }
  else if(cf == 0)
  {
  LCD_SetBrushColor(RGB_RED);
  LCD_DrawFillRect(145,64,9,9); //rf
  }
  else
  {
  LCD_SetBrushColor(RGB_GREEN);
  LCD_DrawFillRect(145,64,9,9); //rf
  }

  LCD_SetBackColor(RGB_WHITE);
  LCD_SetTextColor(RGB_BLACK);
  LCD_DisplayChar(7,1,cp+0x30);
  LCD_DisplayChar(7,4,sg+0x30);
  LCD_DisplayChar(7,7,mk+0x30);
  LCD_DisplayChar(7,10,cf+0x30);
}
void working(void)
{
  EXTI->IMR  &= ~0x3F00;
  GPIOG->ODR |= 0x00FF;
  price = 0;
  LCD_SetBackColor(RGB_RED);
  LCD_SetTextColor(RGB_WHITE);
  LCD_DisplayText(5,2,"0");
  DelayMS(1000);
  LCD_DisplayText(5,2,"1");
  DelayMS(1000);
  LCD_DisplayText(5,2,"2");
  DelayMS(1000);
  LCD_DisplayText(5,2,"W");
  GPIOG->ODR &= ~0x00FF;
  LCD_SetBackColor(RGB_BLUE);
  LCD_SetTextColor(RGB_WHITE);
  LCD_DisplayText(4,1,"B");
  LCD_DisplayText(4,3,"M");
  LCD_DisplayText(3,2,"S");
}
void refill(void)
{
  int flag = 0;
  if(cp == 0)
  {
    cp = 9;
    flag = 1;
  }
  if(sg == 0)
  {
    sg = 5;
    flag = 1;
  }
  if(mk == 0)
  {
    mk = 5;
    flag = 1;
  }
  if(cf == 0)
  {
    cf = 9;
    flag = 1;
  }
  if(flag == 1)
  {
    BEEP();
  }
}
void clear(void)
{
  total_income = 0;
  noc_num = 0;
  Fram_Write(603,total_income);
  Fram_Write(604,noc_num);
  change();
  LCD_SetBackColor(RGB_YELLOW);
  LCD_SetTextColor(RGB_BLACK);
  LCD_DisplayText(7,16,noc);
  LCD_SetBackColor(RGB_BLACK);
  LCD_SetTextColor(RGB_YELLOW);
  LCD_DisplayText(3,16,tot);
}
void return_coin(void)
{
  in_coin = 0;
  Fram_Write(602,in_coin);
  change();
  LCD_SetBackColor(RGB_BLACK);
  LCD_SetTextColor(RGB_YELLOW);
  LCD_DisplayText(1,16,in);
}