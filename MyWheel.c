/************************************************************************************
*                    项目名称：MyWheel.c                                         *
*                    项目描述：通过遥控器控制小车（modbus协议）                       *
*                    生成环境：Linux 下gcc                                          *
*                    生成日期：2019-3-14                                            *
*                    作者：mgdtsxc                                                  *       
************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <modbus.h>
#include <unistd.h>
#include <stdbool.h>

#define SpinSpeed 100
#define MaxSpeed 400
#define MinSpeed -400
#define SpeedDiff 30
#define acceleratedSpeed 25
#define TurnSpeed1 100
#define TurnSpeed2 200
#define TurnSpeed3 300
#define TurnSpeed4 400

/*
bool isStop;//小车是否停止
bool isSpin;//小车是否旋转
bool isQuit;//程序是否退出

bool isTurn;
*/
bool isChange;//寄存器状态是否需要发生改变
int WriteRegNum; // 0:停止    1:前进后退    2:转弯    3:原地转圈   4:程序退出
unsigned short leftSpeed;
unsigned short rightSpeed;

struct Wheel
{
    modbus_t *ctx;
    int speed;
};
struct Wheel lfwheel;//左前轮
struct Wheel lbwheel;//左后轮
struct Wheel rfwheel;//右前轮
struct Wheel rbwheel;//右后轮
int keyGetch()
{
    int ch;
    system("stty -echo");
    system("stty -icanon");
    ch = getchar();
    system("stty icanon");    
    system("stty echo");
    return ch;
}
/*****************************************************************************************
  Start                                                                                  *
                                      Initialization                                     *
                                                                                  Start  *
*****************************************************************************************/
//初始化四个驱动串口
void InitDriver()
{
	//初始化变量
	//isStop = true;
	//isSpin = false;
	//isQuit = false;
	//isChange = true;
	//isTurn = false;
	WriteRegNum = 0;
	isChange = false;

	lfwheel.speed=0;
	lbwheel.speed=0;
	rfwheel.speed=0;
	rbwheel.speed=0;
	

	//初始化驱动
	lfwheel.ctx = modbus_new_rtu("/dev/ttyUSB2",9600,'E',8,1);
	lbwheel.ctx = modbus_new_rtu("/dev/ttyUSB3",9600,'E',8,1);
	rfwheel.ctx = modbus_new_rtu("/dev/ttyUSB0",9600,'E',8,1);
	rbwheel.ctx = modbus_new_rtu("/dev/ttyUSB1",9600,'E',8,1);
	modbus_set_slave(lfwheel.ctx, 1);
	modbus_set_slave(lbwheel.ctx, 1);
	modbus_set_slave(rfwheel.ctx, 1);
	modbus_set_slave(rbwheel.ctx, 1);
	if(modbus_connect(lfwheel.ctx) == -1)
	{
		fprintf(stderr, "lfwheel Connction failed: \n");
		modbus_free(lfwheel.ctx);
	}
	if(modbus_connect(lbwheel.ctx) == -1)
	{
		fprintf(stderr, "lbwheel Connction failed: \n");
		modbus_free(lbwheel.ctx);
	}
	if(modbus_connect(rfwheel.ctx) == -1)
	{
		fprintf(stderr, "rfwheel Connction failed: \n");
		modbus_free(rfwheel.ctx);
	}
	if(modbus_connect(rbwheel.ctx) == -1)
	{
		fprintf(stderr, "rbwheel Connction failed: \n");
		modbus_free(rbwheel.ctx);
	}
	printf("成功初始化");
}
/*****************************************************************************************
  End                                                                                    *
                                      Initialization                                     *
                                                                                    End  *
*****************************************************************************************/

/*****************************************************************************************
  Start                                                                                  *
                            Functions of wheel controller                                *
                                                                                  Start  *
*****************************************************************************************/
/*   speed up   */
void speedUp()
{
	lfwheel.speed += acceleratedSpeed;
	lbwheel.speed += acceleratedSpeed;
	rfwheel.speed += acceleratedSpeed;
	rbwheel.speed += acceleratedSpeed;
}
/*   speed down   */
void speedDown()
{
	lfwheel.speed -= acceleratedSpeed;
	lbwheel.speed -= acceleratedSpeed;
	rfwheel.speed -= acceleratedSpeed;
	rbwheel.speed -= acceleratedSpeed;
}
/*   turn left   */
void turnLeft()
{
	lfwheel.speed = -(rfwheel.speed / 2);
	lbwheel.speed = -(rbwheel.speed / 2);
	rfwheel.speed += rfwheel.speed / 2;
	rbwheel.speed += rbwheel.speed / 2;	
}
/*   turn right   */
void turnRight()
{
	rfwheel.speed = -(lfwheel.speed / 2);
	rbwheel.speed = -(lbwheel.speed / 2);
	lfwheel.speed += lfwheel.speed / 2;
	lbwheel.speed += lbwheel.speed / 2;
}
/*   anticlockwise spotturn   */
void antickSpin(int speed)
{
	unsigned short temp;
	temp = speed;
	rightSpeed = -temp;
	leftSpeed = temp;
	//leftSpeed = 0x0128;
	//rightSpeed = 0;
}
/*   clockwise spotturn   */
void ckSpin(int speed)
{
	unsigned short temp;
	temp = speed;
	leftSpeed = -temp;
	rightSpeed = temp;
	//leftSpeed = 0;
	//rightSpeed = 0x0128;
}
/*   balance   */
void balance()
{
	int temp;
	temp = (lfwheel.speed + rfwheel.speed);
	lfwheel.speed = temp;
	lbwheel.speed = temp;
	rfwheel.speed = temp;
	rbwheel.speed = temp;		
}
/*****************************************************************************************
  End                                                                                    *
                            Functions of wheel controller                                *
                                                                                    End  *
*****************************************************************************************/
/*****************************************************************************************
  Start                                                                                  *
                                        Update                                           *
                                                                                  Start  *
*****************************************************************************************/
//向驱动器写数据
void writeReg()
{
	unsigned short tempSpeed;
	printf("%d",WriteRegNum);
	switch(WriteRegNum)
	{
	case 0:
		modbus_write_register(lfwheel.ctx, 0x0040, 0);
		modbus_write_register(lbwheel.ctx, 0x0040, 0);
		modbus_write_register(rfwheel.ctx, 0x0040, 0);
		modbus_write_register(rbwheel.ctx, 0x0040, 0);
		break;
	case 1:
		if(lfwheel.speed > MinSpeed && lfwheel.speed < MaxSpeed)
		{
			tempSpeed = lfwheel.speed;
			modbus_write_register(lfwheel.ctx, 0x0042, tempSpeed);
		}
		if(lbwheel.speed > MinSpeed && lbwheel.speed < MaxSpeed)
		{
			tempSpeed = lbwheel.speed;
			modbus_write_register(lbwheel.ctx, 0x0042, tempSpeed);
		}
		if(rfwheel.speed > MinSpeed && rfwheel.speed < MaxSpeed)
		{
			tempSpeed = rfwheel.speed;
			modbus_write_register(rfwheel.ctx, 0x0042, tempSpeed);
		}
		if(rbwheel.speed > MinSpeed && rbwheel.speed < MaxSpeed)
		{
			tempSpeed = rbwheel.speed;
			modbus_write_register(rbwheel.ctx, 0x0042, tempSpeed);
		}
		break;
	case 2:
		if(lfwheel.speed > MinSpeed && lfwheel.speed < MaxSpeed)
		{
			tempSpeed = lfwheel.speed;
			modbus_write_register(lfwheel.ctx, 0x0042, tempSpeed);
		}
		if(lbwheel.speed > MinSpeed && lbwheel.speed < MaxSpeed)
		{
			tempSpeed = lbwheel.speed;
			modbus_write_register(lbwheel.ctx, 0x0042, tempSpeed);
		}
		if(rfwheel.speed > MinSpeed && rfwheel.speed < MaxSpeed)
		{
			tempSpeed = rfwheel.speed;
			modbus_write_register(rfwheel.ctx, 0x0042, tempSpeed);
		}
		if(rbwheel.speed > MinSpeed && rbwheel.speed < MaxSpeed)
		{
			tempSpeed = rbwheel.speed;
			modbus_write_register(rbwheel.ctx, 0x0042, tempSpeed);
		}
		break;
	case 3:
		modbus_write_register(lfwheel.ctx, 0x0040, 0);
		modbus_write_register(lbwheel.ctx, 0x0040, 0);
		modbus_write_register(rfwheel.ctx, 0x0040, 0);
		modbus_write_register(rbwheel.ctx, 0x0040, 0);
		usleep(500000);
		modbus_write_register(lfwheel.ctx, 0x0042, leftSpeed);
		modbus_write_register(lbwheel.ctx, 0x0042, leftSpeed);
		modbus_write_register(rfwheel.ctx, 0x0042, rightSpeed);
		modbus_write_register(rbwheel.ctx, 0x0042, rightSpeed);
		break;
	default:
		break;
	}
	/*
	if(false == isStop)
	{
		if(false == isSpin)
		{
			if(false == isTurn)
			{
				if(lfwheel.speed > MinSpeed && lfwheel.speed < MaxSpeed)
				{
					tempSpeed = lfwheel.speed;
					modbus_write_register(lfwheel.ctx, 0x0042, tempSpeed);
				}
				if(lbwheel.speed > MinSpeed && lbwheel.speed < MaxSpeed)
				{
					tempSpeed = lbwheel.speed;
					modbus_write_register(lbwheel.ctx, 0x0042, tempSpeed);
				}
				if(rfwheel.speed > MinSpeed && rfwheel.speed < MaxSpeed)
				{
					tempSpeed = rfwheel.speed;
					modbus_write_register(rfwheel.ctx, 0x0042, tempSpeed);
				}
				if(rbwheel.speed > MinSpeed && rbwheel.speed < MaxSpeed)
				{
					tempSpeed = rbwheel.speed;
					modbus_write_register(rbwheel.ctx, 0x0042, tempSpeed);
				}
			}
			else
			{
				modbus_write_register(lfwheel.ctx, 0x0040, 0);
				modbus_write_register(lbwheel.ctx, 0x0040, 0);
				modbus_write_register(rfwheel.ctx, 0x0040, 0);
				modbus_write_register(rbwheel.ctx, 0x0040, 0);
				usleep(500000);
				if(lfwheel.speed > MinSpeed && lfwheel.speed < MaxSpeed)
				{
					tempSpeed = lfwheel.speed;
					modbus_write_register(lfwheel.ctx, 0x0042, tempSpeed);
				}
				if(lbwheel.speed > MinSpeed && lbwheel.speed < MaxSpeed)
				{
					tempSpeed = lbwheel.speed;
					modbus_write_register(lbwheel.ctx, 0x0042, tempSpeed);
				}
				if(rfwheel.speed > MinSpeed && rfwheel.speed < MaxSpeed)
				{
					tempSpeed = rfwheel.speed;
					modbus_write_register(rfwheel.ctx, 0x0042, tempSpeed);
				}
				if(rbwheel.speed > MinSpeed && rbwheel.speed < MaxSpeed)
				{
					tempSpeed = rbwheel.speed;
					modbus_write_register(rbwheel.ctx, 0x0042, tempSpeed);
				}
			}
		}
		else
		{
			modbus_write_register(lfwheel.ctx, 0x0040, 0);
			modbus_write_register(lbwheel.ctx, 0x0040, 0);
			modbus_write_register(rfwheel.ctx, 0x0040, 0);
			modbus_write_register(rbwheel.ctx, 0x0040, 0);
			usleep(500000);
			modbus_write_register(lfwheel.ctx, 0x0042, leftSpeed);
			modbus_write_register(lbwheel.ctx, 0x0042, leftSpeed);
			modbus_write_register(rfwheel.ctx, 0x0042, rightSpeed);
			modbus_write_register(rbwheel.ctx, 0x0042, rightSpeed);
		}
	}
	else
	{
		modbus_write_register(lfwheel.ctx, 0x0040, 0);
		modbus_write_register(lbwheel.ctx, 0x0040, 0);
		modbus_write_register(rfwheel.ctx, 0x0040, 0);
		modbus_write_register(rbwheel.ctx, 0x0040, 0);
	}
	*/
	printf("lfsp:%d,lbsp:%d,rfsp:%d,rbsp:%d\n",
	lfwheel.speed,lbwheel.speed,rfwheel.speed,rbwheel.speed);
	isChange = false;
}
//监听遥控消息
void myController()
{
	int key = keyGetch();
	switch(key)
	{
	case 65:
		WriteRegNum = 1;
		isChange = true;
		speedUp();
		break;
	case 66:
		WriteRegNum = 1;
		isChange = true;
		speedDown();
		break;
	case 68:
		WriteRegNum = 2;
		isChange = true;
		turnLeft();
		break;
	case 67:
		WriteRegNum = 2;
		isChange = true;
		turnRight();
		break;
	case 10:
		WriteRegNum = 0;
		isChange = true;
		lfwheel.speed=0;
		lbwheel.speed=0;
		rfwheel.speed=0;
		rbwheel.speed=0;
		break;
	case 'q':
		WriteRegNum = 3;
		isChange = true;
		antickSpin(TurnSpeed1);
		break;	
	case 'e':
		WriteRegNum = 3;
		isChange = true;
		ckSpin(TurnSpeed1);
		break;
	case 'r':
		WriteRegNum = 1;
		isChange = true;
		balance();
		break;
	case 'a':
		WriteRegNum = 3;
		isChange = true;
		antickSpin(TurnSpeed3);
		break;
	case 'd':
		WriteRegNum = 3;
		isChange = true;
		ckSpin(TurnSpeed3);
		break;
	case 'z':
		WriteRegNum = 4;
		modbus_write_register(lfwheel.ctx, 0x0040, 0);
		modbus_write_register(lbwheel.ctx, 0x0040, 0);
		modbus_write_register(rfwheel.ctx, 0x0040, 0);
		modbus_write_register(rbwheel.ctx, 0x0040, 0);
		break;
	default:
		break;
	}
}
void TestControl()
{
	int key = keyGetch();

	if(key ==  65)
	{
		modbus_write_register(lfwheel.ctx, 0x0042, 0x0029);
		modbus_write_register(lbwheel.ctx, 0x0042, 0x0029);
		modbus_write_register(rfwheel.ctx, 0x0042, 0x0029);
		modbus_write_register(rbwheel.ctx, 0x0042, 0x0029);
	}
	if(key ==  66)
	{
		modbus_write_register(lfwheel.ctx, 0x0042, 0xffd7);
		modbus_write_register(lbwheel.ctx, 0x0042, 0xffd7);
		modbus_write_register(rfwheel.ctx, 0x0042, 0xffd7);
		modbus_write_register(rbwheel.ctx, 0x0042, 0xffd7);
	}
	if(key == 68)
	{
		modbus_write_register(lfwheel.ctx, 0x0040, 0);
		modbus_write_register(lbwheel.ctx, 0x0040, 0);
		modbus_write_register(rfwheel.ctx, 0x0040, 0);
		modbus_write_register(rbwheel.ctx, 0x0040, 0);
	}	
}
/*****************************************************************************************
  End                                                                                    *
                                        Update                                           *
                                                                                    End  *
*****************************************************************************************/
void endWheel()
{
	modbus_close(lfwheel.ctx);
	modbus_free(lfwheel.ctx);
	modbus_close(lbwheel.ctx);
	modbus_free(lbwheel.ctx);
	modbus_close(rfwheel.ctx);
	modbus_free(rfwheel.ctx);
	modbus_close(rbwheel.ctx);
	modbus_free(rbwheel.ctx);
}
int main()
{
	InitDriver();
	while(1)
	{
		myController();
		if(4 == WriteRegNum)
		{
			break;
		}
		if(true == isChange)
		{
			writeReg();
		}
	}
	endWheel();
	/*while(1)
	{
		TestControl();
	}*/
	
	return 0;
}


