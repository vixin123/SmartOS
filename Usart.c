#include "System.h"
#include "Pin_STM32F0.h"
#include <stdio.h>

#define IS_REMAP(com) ((Usart_Remap >> com) & 0x01 == 0x01)
// 默认波特率
#define USART_DEFAULT_BAUDRATE 115200

static USART_TypeDef* g_Uart_Ports[] = UARTS; 
static const Pin g_Uart_Pins[] = UART_PINS;
static const Pin g_Uart_Pins_Map[] = UART_PINS_FULLREMAP;

// 指定哪个串口采用重映射
static byte Usart_Remap;
// 串口状态
static bool Usart_opened[6];

// 获取引脚
void TUsart_GetPins(int com, Pin* txPin, Pin* rxPin)
{
	const Pin* p;

    *rxPin = *txPin = P0;
	
	p = g_Uart_Pins;
	if(IS_REMAP(com)) p = g_Uart_Pins_Map;

	com = com << 2;
	*txPin  = p[com];
	*rxPin  = p[com + 1];
}

// 打开串口
bool TUsart_Open2(int com, int baudRate, int parity, int dataBits, int stopBits)
{
	USART_InitTypeDef  p;
    USART_TypeDef* port = g_Uart_Ports[com];
    Pin tx, rx;
    TUsart_GetPins(com, &tx, &rx);
    
    USART_DeInit(port);

	// 检查重映射
#ifdef STM32F1XX
	if(IS_REMAP(com))
	{
		switch (com) {
		case 0: AFIO->MAPR |= AFIO_MAPR_USART1_REMAP; break;
		case 1: AFIO->MAPR |= AFIO_MAPR_USART2_REMAP; break;
		case 2: AFIO->MAPR |= AFIO_MAPR_USART3_REMAP_FULLREMAP; break;
		}
	}
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE );
#endif

    // 打开 UART 时钟。必须先打开串口时钟，才配置引脚
#ifdef STM32F0XX
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);//开启时钟
#else
	if (com) { // COM2-5 on APB1
        RCC->APB1ENR |= RCC_APB1ENR_USART2EN >> 1 << com;
    } else { // COM1 on APB2
        RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    }
#endif

	//串口引脚初始化
#ifdef STM32F0XX
    Sys.IO.OpenPort(tx, GPIO_Mode_AF, GPIO_Speed_10MHz, GPIO_OType_PP);
    Sys.IO.OpenPort(rx, GPIO_Mode_AF, GPIO_Speed_10MHz, GPIO_OType_PP);
    GPIO_PinAFConfig(_GROUP(tx), _PIN(tx), GPIO_AF_1);//将IO口映射为USART接口
    GPIO_PinAFConfig(_GROUP(rx), _PIN(rx), GPIO_AF_1);
#else
    Sys.IO.OpenPort(tx, GPIO_Mode_AF_PP, GPIO_Speed_50MHz);
    Sys.IO.Open(rx, GPIO_Mode_IN_FLOATING);
#endif

    USART_StructInit(&p);
	p.USART_BaudRate = baudRate;
	p.USART_WordLength = dataBits;
	p.USART_StopBits = stopBits;
	p.USART_Parity = parity;
	USART_Init(port, &p);

	USART_ITConfig(port, USART_IT_RXNE, ENABLE);//串口接收中断配置

	USART_Cmd(port, ENABLE);//使能串口

    Usart_opened[com] = true;

    return true;
}

// 打开串口
bool TUsart_Open(int com, int baudRate)
{
    return TUsart_Open2(com, baudRate, 
        USART_Parity_No,        //无奇偶校验
        USART_WordLength_8b,    //8位数据长度
        USART_StopBits_1        //1位停止位
    );
}

// 关闭端口
void TUsart_Close(int com)
{
    USART_TypeDef* port = g_Uart_Ports[com];
    Pin tx, rx;
    TUsart_GetPins(com, &tx, &rx);

    USART_DeInit(port);
    
    Sys.IO.Close(tx);
    Sys.IO.Close(rx);

	// 检查重映射
#ifdef STM32F1XX
	if(IS_REMAP(com))
	{
		switch (com) {
		case 0: AFIO->MAPR &= ~AFIO_MAPR_USART1_REMAP; break;
		case 1: AFIO->MAPR &= ~AFIO_MAPR_USART2_REMAP; break;
		case 2: AFIO->MAPR &= ~AFIO_MAPR_USART3_REMAP_FULLREMAP; break;
		}
	}
#endif
    
    Usart_opened[com] = false;
}

// 发送单一字节数据
void TUsart_SendData(USART_TypeDef* port, char* data)
{
    //USART_SendData(port, (ushort)*data);
    while(USART_GetFlagStatus(port, USART_FLAG_TXE) == RESET);//等待发送完毕
    USART_SendData(port, (ushort)*data);
}

// 向某个端口写入数据。如果size为0，则把data当作字符串，一直发送直到遇到\0为止
void TUsart_Write(int com, const string data, int size)
{
    int i;
    string byte = data;
    USART_TypeDef* port = g_Uart_Ports[com];

    if(!Usart_opened[com]) TUsart_Open(com, USART_DEFAULT_BAUDRATE);
    
    if(size > 0)
    {
        for(i=0; i<size; i++) TUsart_SendData(port, byte++);
    }
    else
    {
        while(*byte) TUsart_SendData(port, byte++);
    }
}

// 从某个端口读取数据
int TUsart_Read(int com, string data, uint size)
{
    //USART_TypeDef* port = g_Uart_Ports[com];

    if(!Usart_opened[com]) TUsart_Open(com, USART_DEFAULT_BAUDRATE);

    return 0;
}

// 刷出某个端口中的数据
void TUsart_Flush(int com)
{
    USART_TypeDef* port = g_Uart_Ports[com];

    while(USART_GetFlagStatus(port, USART_FLAG_TXE) == RESET);//等待发送完毕

    if(!Usart_opened[com]) TUsart_Open(com, USART_DEFAULT_BAUDRATE);
}

// 指定哪个串口采用重映射
void TUsart_SetRemap(int com)
{
	Usart_Remap |= (1 << com);
}

void OnReceive(USART_TypeDef* u)
{
    char c;
    if(USART_GetITStatus(u, USART_IT_RXNE) != RESET)
    { 	
        c = u->DR;
        //USART_SendByte(u, c); 	    
    } 
}

void USART1_IRQHandler(void) { OnReceive(USART1); }
void USART2_IRQHandler(void) { OnReceive(USART2); }
void USART3_IRQHandler(void) { OnReceive(USART3); }
void USART4_IRQHandler(void) { OnReceive(UART4); }
void USART5_IRQHandler(void) { OnReceive(UART5); }

// 初始化串口函数
void TUsart_Init(TUsart* this)
{
    this->Open  = TUsart_Open;
    this->Open2 = TUsart_Open2;
    this->Close = TUsart_Close;
    this->Write = TUsart_Write;
    this->Read  = TUsart_Read;
    this->Flush = TUsart_Flush;
	this->SetRemap = TUsart_SetRemap;
}

/* 重载fputc可以让用户程序使用printf函数 */
int fputc(int ch, FILE *f)
{
    USART_TypeDef* port;
    int com = Sys.MessagePort;

    if(com == COM_NONE) return ch;

    if(!Usart_opened[com]) TUsart_Open(com, USART_DEFAULT_BAUDRATE);

    port = g_Uart_Ports[com];

    //while(!((port->ISR)&(1<<6)));//等待缓冲为空
    //port->TDR = (byte) ch;
    //USART_SendData(port, (unsigned char) ch);

#ifdef STM32F0XX
    //while(!((port->ISR)&(1<<6)));//等待缓冲为空
#else
    //while (!(port->SR & USART_FLAG_TXE));
#endif
    //USART_SendData(port, (unsigned char) ch);
    TUsart_SendData(port, (char*)&ch);

    return ch;
}
