/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xiic.h"
#include "Xscugic.h"


typedef enum {up,down} position_t;

void SetUpInterruptSystem();
void SendHandler(XIic *InstancePtr);
void ReceiveHandler(XIic *InstancePtr);
void StatusHandler(XIic *InstancePtr, int Event);

void checki2c();
void WriteLeg(int leg, position_t position );


#define IIC_dev 			XPAR_IIC_0_DEVICE_ID
#define int_dev 			XPAR_SCUGIC_0_DEVICE_ID
#define IIC_SLAVE_ADDR		0x40
#define INTC_DEVICE_INT_ID	XPAR_FABRIC_IIC_0_VEC_ID
#define BUFFER_SIZE		    6
#define up_h_on				0x07
#define up_l_on				0xff
#define up_h_off			0x07
#define up_l_off			0xff
#define down_h_on			0x01
#define down_l_on 			0xef
#define down_h_off			0x0e
#define down_l_off			0x10

XIic  iic;
XScuGic Intc;

volatile u8 TransmitComplete;
volatile u8 ReceiveComplete;

int main()
{

	u32  Status;
    XIic_Config *iic_conf;
    u8 Options;
    u8 SendBuffer [1];
    u8 RecvBuffer [1];

   init_platform();

    print("Hello World\n\r");

    iic_conf = XIic_LookupConfig(IIC_dev);
    XIic_CfgInitialize(&iic, iic_conf, iic_conf->BaseAddress);

    SetUpInterruptSystem();

	XIic_SetSendHandler(&iic, &iic,	(XIic_Handler) SendHandler);
	XIic_SetRecvHandler(&iic, &iic, (XIic_Handler) ReceiveHandler);
	XIic_SetStatusHandler(&iic, &iic,(XIic_StatusHandler) StatusHandler);

    XIic_SetAddress(&iic,XII_ADDR_TO_SEND_TYPE,IIC_SLAVE_ADDR );
    XIic_SetOptions(&iic, XII_REPEATED_START_OPTION);

   // XIic_Start(&iic);

    checki2c();
    usleep(100000);
    WriteLeg(0, up );

    while(1){

    }

    cleanup_platform();
    return 0;
}

void checki2c()
//this fucntion checks the shield is present following power on by reading register
//0 of the i2c chip on the shield and looks for the default power on configuration of the
//register 0 this should be 0x11
{

    u8 SendBuffer [1];
    u8 RecvBuffer [1];
   // u32  Status;
  //  XIic_Reset(&iic);
   XIic_Start(&iic);
	SendBuffer[0]= 0x00;
    TransmitComplete = 1;
    ReceiveComplete = 1;
    XIic_MasterSend(&iic, SendBuffer,1);
    while ((TransmitComplete)) {
    }
    XIic_MasterRecv(&iic, RecvBuffer,  2);
	while ((ReceiveComplete)) {
	}
	if (RecvBuffer[0] == 0x11) {
		printf("shield detected\n\r");
		printf("Rx Buff %x\n\r",RecvBuffer[0]);
	}
	else
	{
		printf("shield not detected\n\r");
		return 0;
	}

    XIic_Stop(&iic);
}


void WriteLeg(int leg, position_t position )
//this positions a leg to either up or dwn
{
    u8 SendBuffer [2];
    u8 RecvBuffer [1];
    u32  Status;

    TransmitComplete = 1;
    ReceiveComplete = 1;

    XIic_Reset(&iic);


    if (position == up){

    	XIic_Start(&iic);
    	usleep(100000);
    	SendBuffer[0] = 0x06;
        XIic_MasterSend(&iic, SendBuffer,1);
        while ((TransmitComplete)) {
        }
        usleep(100000);
        TransmitComplete = 1;
    	SendBuffer[0] = up_l_on;
        XIic_MasterSend(&iic, SendBuffer,1);
        while ((TransmitComplete)) {
        }
        XIic_Stop(&iic);
        XIic_Reset(&iic);
        XIic_Start(&iic);
    	usleep(100000);
    	TransmitComplete = 1;
    	SendBuffer[0] = 0x07;
        XIic_MasterSend(&iic, SendBuffer,1);
        while ((TransmitComplete)) {
        }
        usleep(100000);
        TransmitComplete = 1;
    	SendBuffer[0] = up_h_on;
        XIic_MasterSend(&iic, SendBuffer,1);
        while ((TransmitComplete)) {
        }
        XIic_Stop(&iic);
        XIic_Start(&iic);
        usleep(100000);
    	SendBuffer[0]= 0x06;
        TransmitComplete = 1;
        ReceiveComplete = 1;
        XIic_MasterSend(&iic, SendBuffer,1);
        while ((TransmitComplete)) {
        }
        XIic_MasterRecv(&iic, RecvBuffer,  2);
    	while ((ReceiveComplete)) {
    	}
    	XIic_Stop(&iic);

        printf("Rx Buff %x\n\r",RecvBuffer[0]);

    }



}

void SetUpInterruptSystem()
{
	XScuGic_Config *IntcConfig;
	u32 Status;

	IntcConfig = XScuGic_LookupConfig(int_dev);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&Intc, IntcConfig,IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XScuGic_SetPriorityTriggerType(&Intc, INTC_DEVICE_INT_ID,
					0xA0, 0x3);

	Status = XScuGic_Connect(&Intc, INTC_DEVICE_INT_ID,
				 (Xil_InterruptHandler)XIic_InterruptHandler,
				 &iic);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	XScuGic_Enable(&Intc, INTC_DEVICE_INT_ID);

	Xil_ExceptionInit();

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 (Xil_ExceptionHandler)XScuGic_InterruptHandler, &Intc);

	Xil_ExceptionEnable();
}


void StatusHandler(XIic *InstancePtr, int Event)
{

}

void ReceiveHandler(XIic *InstancePtr)
{
	ReceiveComplete = 0;
    //printf("here rx \n\r");
}

void SendHandler(XIic *InstancePtr)
{
	TransmitComplete = 0;
	//printf("here tx \n\r");
}
