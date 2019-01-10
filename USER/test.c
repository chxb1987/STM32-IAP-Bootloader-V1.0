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
uint32_t CID[3];
uint16_t Id_Crc=0;
void GetSTM32MCUID(uint32_t *id)
{
	uint8_t data[12];
	uint8_t i;
	if(id!=NULL)
	{
		id[0]=*(vu32*)(0x1FFFF7E8);

		id[1]=*(vu32*)(0x1FFFF7EC);

		id[2]=*(vu32*)(0x1FFFF7F0);
		
		printf("MUCID=%x,%x,%x\r\n",id[0],id[1],id[2]);

		data[0]=id[0]>>24;
		data[1]=id[0]>>16;
		data[2]=id[0]>>8;
		data[3]=id[0];
		data[4]=id[1]>>24;
		data[5]=id[1]>>16;
		data[6]=id[1]>>8;
		data[7]=id[1];
		data[8]=id[2]>>24;
		data[9]=id[2]>>16;
		data[10]=id[2]>>8;
		data[11]=id[2];
		Id_Crc = CRC16(data,12);
		for(i=0;i<12;i++)
		{
			printf("data[%d]=%x\r\n",i,data[i]);
		}
		printf("Id_Crc=%x\r\n",Id_Crc);
	}
}

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
	uint8_t dev_sta[4];
	u8 t; 
	u16 applenth=0;
	uint8_t key_num = 0;
			//���յ���app���볤��
 	Stm32_Clock_Init(9);		//ϵͳʱ������
	uart_init(72,115200);		//���ڳ�ʼ��Ϊ115200
	uart2_init(36,115200);
	delay_init(72);	   	 		//��ʱ��ʼ�� 
  	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
	KEY_Init();					//��ʼ������
	GetSTM32MCUID(CID);
	CAN1_Mode_Init(1,2,3,12,0);      //=====CAN��ʼ��	
	Flash_Read();
	delay_ms(500);
	printf("SYSTEM RUN!\r\n");
	while(1)
	{
		key_num = KEY_Scan(0);
		if(key_num == 1)
		{
			Update_Flag = 0;
			Flash_Write();
		}
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
				printf("Crc_In=%x,Crc_Out=%x\r\n",Crc_In,Crc_Out);
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
				if(Update_Error == 1)
				{
					CAN1_Send_ID(0x20000|Local_ID,&Update_Error);
					Update_Error = 0;
				}
				Clear_Buff(USART_RX_BUF,USART_RX_CNT);
				USART_RX_CNT=0;
				Rev_Finish = 0;
			}
			else 
			{
				//printf("û�п��Ը��µĹ̼�!\r\n");
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
		if(Dev_MSG == 1)
		{
			Dev_MSG = 0;
			dev_sta[0] = 0x01;//����̬
			dev_sta[1] = Sys_Ver;//�汾��
			dev_sta[2] = Id_Crc;
			dev_sta[3] = Id_Crc>>8;
			CAN1_Send_Msg(dev_sta,4);//�����ϴ��汾��������״̬
		}
		t++;
		delay_ms(10);
		if(t==10)
		{
			LED0=!LED0;
			LED1=!LED1;
			t=0;

		}	  	 				   	 
	}   	   
}










