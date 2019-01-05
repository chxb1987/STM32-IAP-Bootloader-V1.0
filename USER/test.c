#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h" 		 	 
#include "key.h"  
#include "stmflash.h" 
#include "iap.h"   
#include "crc16.h"

#if IAP_STY
#include "CAN.h"
#endif
uint8_t Rev_Finish = 0;
uint8_t Sys_Ver = 0x01;
/**************************************************************************
�������ܣ���մ��ڽ��ջ���
*buff:��ջ�����׵�ַ
len:��յ����ݳ���
**************************************************************************/
void Clear_Buff(uint8_t *buff,uint16_t len)
{
	uint16_t i;
	for(i=0;i<len;i++)
	{
		buff[i]=0;
	}
}

int main(void)
{	
	uint8_t init=0x01;//����
	u8 t; 
	u16 applenth=0;	
			//���յ���app���볤��
	
 	Stm32_Clock_Init(9);		//ϵͳʱ������
	uart_init(72,115200);		//���ڳ�ʼ��Ϊ115200
	delay_init(72);	   	 		//��ʱ��ʼ�� 
  	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
	KEY_Init();					//��ʼ������
	CAN1_Mode_Init(1,2,3,12,0);      //=====CAN��ʼ��	
	Flash_Read();
	if(Update_Flag!=0)
	{
		CAN1_Send_Msg(&init,1);//�ϴ�״̬���Ǹ���״̬��������״̬
	}
	while(1)
	{
		if(Update_Flag == 0)
		{
			printf("��ʼִ��FLASH�û�����!!\r\n");
			if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//�ж��Ƿ�Ϊ0X08XXXXXX.
			{		
				iap_load_app(FLASH_APP1_ADDR);//ִ��FLASH APP����
			}else 
			{
				printf("��FLASHӦ�ó���,�޷�ִ��!\r\n");  
			}
		}
		else
		{
#if IAP_STY
			if(Rev_Finish == 1)
			{
				Crc_Out = CRC16(USART_RX_BUF,USART_RX_CNT);
				if(Crc_Out == Crc_In)
				{
					applenth=USART_RX_CNT;
					printf("�û�����������!\r\n");
					printf("���볤��:%dBytes\r\n",applenth);
					printf("��ʼ���¹̼�...\r\n");	
					if(((*(vu32*)(0X20001000+4))&0xFF000000)==0x08000000)//�ж��Ƿ�Ϊ0X08XXXXXX.
					{	 
						iap_write_appbin(FLASH_APP1_ADDR,USART_RX_BUF,applenth);//����FLASH����   
						printf("�̼��������!\r\n");	
						Update_Flag = 0;
						Flash_Write();
					}else 
					{   
						printf("��FLASHӦ�ó���!\r\n");
					}
				}
				Clear_Buff(USART_RX_BUF,USART_RX_CNT);
				USART_RX_CNT=0;
				Rev_Finish = 0;
			}
			else 
			{
				printf("û�п��Ը��µĹ̼�!\r\n");
			}	
#else
			if(USART_RX_CNT)
			{
				if(oldcount==USART_RX_CNT)//��������,û���յ��κ�����,��Ϊ�������ݽ������.
				{
					applenth=USART_RX_CNT;
					oldcount=0;
					USART_RX_CNT=0;
					printf("�û�����������!\r\n");
					printf("���볤��:%dBytes\r\n",applenth);
				}else oldcount=USART_RX_CNT;			
			}
			if(applenth)
			{
				printf("��ʼ���¹̼�...\r\n");	
 				if(((*(vu32*)(0X20001000+4))&0xFF000000)==0x08000000)//�ж��Ƿ�Ϊ0X08XXXXXX.
				{	 
					iap_write_appbin(FLASH_APP1_ADDR,USART_RX_BUF,applenth);//����FLASH����   
					printf("�̼��������!\r\n");	
					Update_Flag = 0;
					Flash_Write();
				}else 
				{   
					printf("��FLASHӦ�ó���!\r\n");
				}
 			}else 
			{
				printf("û�п��Ը��µĹ̼�!\r\n");
			}
#endif
		}
		t++;
		delay_ms(10);
		if(t==10)
		{
			LED0=!LED0;
			t=0;

		}	  	 				   	 
	}   	   
}










