﻿#ifndef _IOK0612_H_
#define _IOK0612_H_

#include "Sys.h"
#include "Net\ITransport.h"

#include "TokenNet\TokenClient.h"
#include "App\Alarm.h"
#include "Device\RTC.h"

// WIFI WRGB调色
class IOK0612
{
public:
	List<Pin>	LedPins;
	List<OutputPort*>	Leds;
	List<Pin>	ButtonPins;
	List<InputPort*>	Buttons;

	bool LedsShow;					// LED 显示状态开关

	ISocketHost*	Host;			// 网络主机
	TokenClient*	Client;			//
	Alarm*			AlarmObj;
	uint			LedsTaskId;

	IOK0612();

	void Init(ushort code, cstring name, COM message = COM1);

	void* InitData(void* data, int size);
	void Register(int index, IDataPort& dp);

	void InitLeds();
	void FlushLed();			// 刷新led状态输出
	void InitButtons(InputPort::IOReadHandler press = nullptr);

	bool LedStat(bool enable);

	ISocketHost* Create8266();

	void InitClient();
	void InitNet();
	void InitAlarm();

	void Restore();
	static void OnLongPress(InputPort* port, bool down);

	static IOK0612* Current;

private:
	void*	Data;
	int		Size;

	void OpenClient(ISocketHost& host);
	TokenController* AddControl(ISocketHost& host, const NetUri& uri, ushort localPort);
};

#endif