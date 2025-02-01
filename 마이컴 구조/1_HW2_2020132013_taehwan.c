//************************************//
/////////////////////////////////////////////////////////////
//  ������: ����������
//  ��������:  �� �Է� ��ɿ� ���� ���������� �̵� ������ ������
// �� �Է� ��� ���
//  (1) SW(0~7)�� ���� �� ��� �Է�
//  (2) JS(Joy-Stick)(LEFT, RIGHT)�� ���� �� ��� �Է�
 
// �� �̵��� LED(0~7)�� ����� ǥ��
// Reset State: 0�� (0������ ����)

 //LED ���� ����� �ݵ�� ���� �� �����ؼ� ����� �� 
  //(1) GPIOG?ODR |= 0x?? , GPIOG?ODR &= 0x??
  //(2) GPIOG?BSRRL = 0x??, GPIOG?BSRRH = 0x??
  //(3) GPIOG?ODR <<= 1; (Shift ������ �̿�)
  //* GPIOG?ODR = 0x?? �� ���� ������ ������� ����!

//                *
//  ����� �ϵ����(���): GPIO, Joy-stick, Buzzer, LED, SW
//  ������: 2023. 5.  19
//  ������ Ŭ����:  ȭ���Ϲ� 
//            �й�: 2020132013
//            �̸�: ����ȯ
/////////////////////////////////////////////////////////////
//

#include "stm32f4xx.h"


void _GPIO_Init(void);
uint16_t KEY_Scan(void);
uint16_t JOY_Scan(void);

void BEEP(void);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

void elevator(uint16_t input_sw); //LED�� �̵���Ű�� �Լ�

uint8_t	SW0_Flag, SW1_Flag;

int main(void)
{
	_GPIO_Init();		// GPIO (LED & SW & Buzzer & Joystick)     �ʱ�ȭ

       GPIOG->ODR &= 0xFF00;	// LED �ʱⰪ
       GPIOG->ODR |= 0x0001;  // �ʱ���� 0��
       
	while(1)
	{
          switch(KEY_Scan())
          {
                    case 0xFE00:  // sw0
                      BEEP();
                      if(GPIOG->ODR != 0x0001){
                          elevator(0x0001);    //LED0 �� �� 
                          BEEP();
                          DelayMS(500);
                          BEEP();
                          DelayMS(500);
                          BEEP();
                      }
                      else{   //���� ���� ���� SW�� �Է� SW �Է¸��� ����Ǵ� ����1ȸ ����ǳ�, LED ��ȭ�� ����. �߰��� 0.2���� ����1ȸ �� ���� (�� Beep() 2ȸ ����)
                            DelayMS(200);
                            BEEP();
                      }
                      break;
                      
                    case 0xFD00:   // sw1
                      BEEP();
                      if(GPIOG->ODR != 0x0002){
                          elevator(0x0002);     //LED1 �� ��
                          BEEP();
                          DelayMS(500);
                          BEEP();
                          DelayMS(500);
                          BEEP();
                      }
                      else{   //���� ���� ���� SW�� �Է� SW �Է¸��� ����Ǵ� ����1ȸ ����ǳ�, LED ��ȭ�� ����. �߰��� 0.2���� ����1ȸ �� ���� (�� Beep() 2ȸ ����)
                            DelayMS(200);
                            BEEP();
                      }
                      break;
                      
                    case 0xFB00:   // sw2
                      BEEP();
                      if(GPIOG->ODR != 0x0004){
                          elevator(0x0004);     //LED2 �� ��                      
                          BEEP();
                          DelayMS(500);
                          BEEP();
                          DelayMS(500);
                          BEEP();
                      }
                      else{   //���� ���� ���� SW�� �Է� SW �Է¸��� ����Ǵ� ����1ȸ ����ǳ�, LED ��ȭ�� ����. �߰��� 0.2���� ����1ȸ �� ���� (�� Beep() 2ȸ ����)
                            DelayMS(200);
                            BEEP();
                      }
                      break;  
                      
                    case 0xF700:   // sw3
                      BEEP();
                      if(GPIOG->ODR != 0x0008){
                          elevator(0x0008);         //LED3 �� ��                      
                          BEEP();
                          DelayMS(500);
                          BEEP();
                          DelayMS(500);
                          BEEP();
                      }
                      else{   //���� ���� ���� SW�� �Է� SW �Է¸��� ����Ǵ� ����1ȸ ����ǳ�, LED ��ȭ�� ����. �߰��� 0.2���� ����1ȸ �� ���� (�� Beep() 2ȸ ����)
                            DelayMS(200);
                            BEEP();
                      }
                      break;
                      
                    case 0xEF00:  //sw4
                      BEEP();
                      if(GPIOG->ODR != 0x0010){
                          elevator(0x0010);       //LED4 �� ��                      
                          BEEP();
                          DelayMS(500);
                          BEEP();
                          DelayMS(500);
                          BEEP();
                      }
                      else{   //���� ���� ���� SW�� �Է� SW �Է¸��� ����Ǵ� ����1ȸ ����ǳ�, LED ��ȭ�� ����. �߰��� 0.2���� ����1ȸ �� ���� (�� Beep() 2ȸ ����)
                            DelayMS(200);
                            BEEP();
                      }
                      break;         
                      
                    case 0xDF00: //sw5
                      BEEP();
                      if(GPIOG->ODR != 0x0020){
                          elevator(0x0020);        //LED5 �� ��                      
                          BEEP();
                          DelayMS(500);
                          BEEP();
                          DelayMS(500);
                          BEEP();
                      }
                      else{   //���� ���� ���� SW�� �Է� SW �Է¸��� ����Ǵ� ����1ȸ ����ǳ�, LED ��ȭ�� ����. �߰��� 0.2���� ����1ȸ �� ���� (�� Beep() 2ȸ ����)
                            DelayMS(200);
                            BEEP();
                      }
                      break;
                      
                    case 0xBF00:  //sw6
                      BEEP();
                      if(GPIOG->ODR != 0x0040){
                          elevator(0x0040);      //LED6�� ��                      
                          BEEP();
                          DelayMS(500);
                          BEEP();
                          DelayMS(500);
                          BEEP();
                      }
                      else{   //���� ���� ���� SW�� �Է� SW �Է¸��� ����Ǵ� ����1ȸ ����ǳ�, LED ��ȭ�� ����. �߰��� 0.2���� ����1ȸ �� ���� (�� Beep() 2ȸ ����)
                            DelayMS(200);
                            BEEP();
                      }
                      break;
                      
                    case 0x7F00:   //sw7
                      BEEP();
                      if(GPIOG->ODR != 0x0080){
                          elevator(0x0080);     //LED7 �� ��                      
                          BEEP();
                          DelayMS(500);
                          BEEP();
                          DelayMS(500);
                          BEEP();
                      }
                      else{   //���� ���� ���� SW�� �Է� SW �Է¸��� ����Ǵ� ����1ȸ ����ǳ�, LED ��ȭ�� ����. �߰��� 0.2���� ����1ȸ �� ���� (�� Beep() 2ȸ ����)
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
                      GPIOG->ODR >>=1;           //LED �������� �̵�
                  }
                  
                  if(GPIOG->ODR ==0x0001)     //�������� 0���� �� JS-LEFT�Է� �Ͽ� ����1ȸ ���� LED ��ȭ�� ����. �߰� �� 0.2���� ���� 1�� �� ���� (�� Beep() 2�� ����)
                  {
                      DelayMS(200);
                      BEEP();
                  }
                  break;
                  
                case 0x02E0:   //  RIGHT
                  BEEP();
                  if(GPIOG->ODR !=0x0080)        
                  {
                      GPIOG->ODR <<=1;               //LED ���������� �̵�
                  }
                  else if(GPIOG->ODR ==0x0080)    //�������� 7���� �� JS-RIGHT�Է� �Ͽ� ����1ȸ ���� LED ��ȭ�� ����. �߰� �� 0.2���� ���� 1�� �� ���� (�� Beep() 2�� ����)
                  {
                      DelayMS(200);
                      BEEP();
                  }
                  break;
                  
                  
          }
          
	}  // while(1)
}

/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer), GPIOI(Joystick)) �ʱ� ����	*/
void _GPIO_Init(void)
{
	// LED (GPIO G) ���� : Output mode
	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
   
	// SW (GPIO H) ���� : Input mode 
	RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	// Buzzer (GPIO F) ���� : Output mode
	RCC->AHB1ENR	|=  0x00000020;	// RCC_AHB1ENR : GPIOF(bit#5) Enable							
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
	GPIOF->OTYPER 	&= ~0x0200;	// GPIOF 9 : Push-pull  	
	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 

	//Joy Stick SW(PORT I) ����
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

void elevator(uint16_t input_sw)
{
  if( GPIOG->ODR < input_sw )  //������ ���� �����ִ� ������ �������  --> ���������� ��������
  {
    while(GPIOG->ODR != input_sw) //���� sw�� ���� ������ LED �̵�
    {
      DelayMS(500); //0.5�� delay
      GPIOG->ODR <<= 1; // LED 1ĭ�� �̵�
      DelayMS(500); //0.5�� delay
    }
  }
  
  else if(GPIOG->ODR > input_sw)  //������ ���� �����ִ� ������ ū ��� --> ���������� �ö󰡱�
  {
    while(GPIOG->ODR != input_sw) //���� sw�� ���� ������ LED �̵�
    {
      DelayMS(500); //0.5�� delay
      GPIOG->ODR >>= 1; // LED 1ĭ�� �̵�
      DelayMS(500); //0.5�� delay
    }
  }
  
}