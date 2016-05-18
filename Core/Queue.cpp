﻿#include "Queue.h"

extern void EnterCritical();
extern void ExitCritical();


/*// 智能IRQ，初始化时备份，销毁时还原
// SmartIRQ相当霸道，它直接关闭所有中断，再也没有别的任务可以跟当前任务争夺MCU
class SmartIRQ
{
public:
	SmartIRQ()
	{
		EnterCritical();
	}

	~SmartIRQ()
	{
		ExitCritical();
	}
};*/

Queue::Queue(uint len) : _s(len)
{
	Clear();
}

void Queue::SetCapacity(uint len)
{
	_s.SetLength(len);
}

void Queue::Clear()
{
	_s.SetLength(_s.Capacity());
	_head	= 0;
	_tail	= 0;
	_size	= 0;
}

#if !defined(TINY) && defined(STM32F0)
	#pragma arm section code = "SectionForSys"
#endif

void Queue::Push(byte dat)
{
	_s[_head++] = dat;
	//_head %= _s.Capacity();
	// 除法运算是一个超级大祸害，它浪费了大量时间，导致串口中断接收丢数据
	if(_head >= _s.Capacity()) _head -= _s.Capacity();

	//SmartIRQ irq;
	EnterCritical();
	_size++;
	ExitCritical();
}

byte Queue::Pop()
{
	if(_size == 0) return 0;
	{
		//SmartIRQ irq;
		EnterCritical();
		_size--;
		ExitCritical();
	}

	/*
	昨晚发现串口频繁收发一段数据后出现丢数据现象，也即是size为0，然后tail比head小，刚开始小一个字节，然后会逐步拉大。
	经过分析得知，ARM指令没有递加递减指令，更没有原子操作。
	size拿出来减一，然后再保存回去，但是保存回去之前，串口接收中断写入，拿到了旧的size，导致最后的size比原计划要小1。
	该问题只能通过关闭中断来解决。为了减少关中断时间以提升性能，增加了专门的Read方法。
	*/

	byte dat	= _s[_tail++];
	//_tail		%= _s.Capacity();
	if(_tail >= _s.Capacity()) _tail -= _s.Capacity();

	return dat;
}

#pragma arm section code

uint Queue::Write(const Buffer& bs)
{
	/*
	1，数据写入队列末尾
	2，如果还剩有数据，则从开头开始写入
	3，循环处理2
	4，如果队列过小，很有可能后来数据会覆盖前面数据
	*/

	//byte*	buf	= (byte*)bs.GetBuffer();
	uint	len	= bs.Length();

	uint rs = 0;
	while(true)
	{
		// 计算这一个循环剩下的位置
		uint remain = _s.Length() - _head;
		// 如果要写入的数据足够存放
		if(len <= remain)
		{
			//_s.Copy(buf, len, _head);
			_s.Copy(_head, bs, rs, len);
			rs		+= len;
			_head	+= len;
			if(_head >= _s.Length()) _head -= _s.Length();

			break;
		}

		// 否则先写一段，指针回到开头
		//_s.Copy(buf, remain, _head);
		_s.Copy(_head, bs, rs, remain);
		//buf		+= remain;
		len		-= remain;
		rs		+= remain;
		_head	= 0;
	}

	{
		//SmartIRQ irq;
		EnterCritical();
		_size += rs;
		ExitCritical();
	}

	return rs;
}

uint Queue::Read(Buffer& bs)
{
	if(_size == 0) return 0;

	/*
	1，读取当前数据到末尾
	2，如果还剩有数据，则从头开始读取
	3，循环处理2
	4，如果队列过小，很有可能后来数据会覆盖前面数据
	*/

	uint	len	= bs.Length();
	if(!len) return 0;
	//byte*	buf	= (byte*)bs.GetBuffer();

	if(len > _size) len = _size;

	uint rs = 0;
	while(true)
	{
		// 计算这一个循环剩下的位置
		uint remain = _s.Capacity() - _tail;
		// 如果要读取的数据都在这里
		if(len <= remain)
		{
			//_s.CopyTo(buf, len, _tail);
			//_s.CopyTo(_tail, bs, rs, len);
			bs.Copy(rs, _s, _tail, len);
			rs		+= len;
			_tail	+= len;
			if(_tail >= _s.Capacity()) _tail -= _s.Capacity();

			break;
		}

		// 否则先读一段，指针回到开头
		//_s.CopyTo(buf, remain, _tail);
		//_s.CopyTo(_tail, bs, rs, remain);
		bs.Copy(rs, _s, _tail, remain);
		//buf		+= remain;
		len		-= remain;
		rs		+= remain;
		_tail	= 0;
	}

	//bs.SetLength(rs, false);

	{
		//SmartIRQ irq;
		EnterCritical();
		_size -= rs;
		ExitCritical();
	}
	//if(_size == 0) _tail = _head;

	return rs;
}