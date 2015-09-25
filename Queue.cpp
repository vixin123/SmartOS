﻿#include "Queue.h"

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

void Queue::Push(byte dat)
{
	SmartIRQ irq;

	_s[_head++] = dat;
	_head %= _s.Capacity();
	_size++;
}

byte Queue::Pop()
{
	SmartIRQ irq;

	if(_size == 0) return 0;
	_size--;

	/*
	昨晚发现串口频繁收发一段数据后出现丢数据现象，也即是size为0，然后tail比head小，刚开始小一个字节，然后会逐步拉大。
	经过分析得知，ARM指令没有递加递减指令，更没有原子操作。
	size拿出来减一，然后再保存回去，但是保存回去之前，串口接收中断写入，拿到了旧的size，导致最后的size比原计划要小1。
	该问题只能通过关闭中断来解决。为了减少关中断时间以提升性能，增加了专门的Read方法。
	*/

	byte dat	= _s[_tail++];
	_tail		%= _s.Capacity();

	return dat;
}

uint Queue::Write(const ByteArray& bs, bool safe)
{
	SmartIRQ irq;

	/*
	1，数据写入队列末尾
	2，如果还剩有数据，则从开头开始写入
	3，循环处理2
	4，如果队列过小，很有可能后来数据会覆盖前面数据
	*/

	byte*	buf	= bs.GetBuffer();
	uint	len	= bs.Length();

	uint rs = 0;
	while(true)
	{
		// 计算这一个循环剩下的位置
		uint remain = _s.Capacity() - _head;
		// 如果要写入的数据足够存放
		if(len <= remain)
		{
			//memcpy(_s.GetBuffer() + _head, buf, len);
			_s.Copy(buf, len, _head);
			rs		+= len;
			_head	+= len;
			_head	%= _s.Capacity();

			break;
		}

		// 否则先写一段，指针回到开头
		//memcpy(_s.GetBuffer() + _head, buf, remain);
		_s.Copy(buf, remain, _head);
		buf		+= remain;
		len		-= remain;
		rs		+= remain;
		_head	= 0;
	}

	_size += rs;

	return rs;
}

uint Queue::Read(ByteArray& bs, bool safe)
{
	if(_size == 0) return 0;

	//debug_printf("_head=%d _tail=%d _size=%d \r\n", _head, _tail, _size);

	SmartIRQ irq;

	/*
	1，读取当前数据到末尾
	2，如果还剩有数据，则从头开始读取
	3，循环处理2
	4，如果队列过小，很有可能后来数据会覆盖前面数据
	*/

	byte*	buf	= bs.GetBuffer();
	uint	len	= bs.Capacity();

	if(len > _size) len = _size;

	uint rs = 0;
	while(true)
	{
		// 计算这一个循环剩下的位置
		uint remain = _s.Capacity() - _tail;
		// 如果要读取的数据都在这里
		if(len <= remain)
		{
			//memcpy(buf, _s.GetBuffer() + _tail, len);
			_s.CopyTo(buf, len, _tail);
			rs		+= len;
			_tail	+= len;
			_tail	%= _s.Capacity();

			break;
		}

		// 否则先读一段，指针回到开头
		//memcpy(buf + _tail, _s.GetBuffer(), remain);
		_s.CopyTo(buf, remain, _tail);
		buf		+= remain;
		len		-= remain;
		rs		+= remain;
		_tail	= 0;
	}

	_size -= rs;
	//if(_size == 0) _tail = _head;

	bs.SetLength(rs, false);

	return rs;
}
