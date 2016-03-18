﻿#include "String.h"
#include "Time.h"

/*static String TestMove(String& ss)
{
	//String ss = "Hello Move";
	ss += " Test ";
	//ss += dd;

	return ss;
}*/

static void TestCtor()
{
	TS("TestCtor");

	debug_printf("字符串构造函数测试\r\n");

	String str1("456");
	assert(str1 == "456", "String(const char* cstr)");
	assert(str1.GetBuffer() != "456", "String(const char* cstr)");

	String str2(str1);
	assert(str2 == str1, "String(const String& str)");
	assert(str2.GetBuffer() != str1.GetBuffer(), "String(const String& str)");

	//StringHelper str3(str1);
	//assert(str3 == str1, "String(StringHelper&& rval)");
	//assert(str3.GetBuffer() != str1.GetBuffer(), "String(StringHelper&& rval)");

	char cs[] = "Hello Buffer";
	String str4(cs, sizeof(cs));
	assert(str4 == cs, "String(char* str, int length)");
	assert(str4.GetBuffer() == cs, "String(char* str, int length)");

	/*debug_printf("move测试\r\n");
	auto tt	= TestMove(str1);
	tt.Show(true);
	str1.Show(true);*/

	String str5((char)'1');
	assert(str5 == "1", "String(char c)");
}

void TestNum10()
{
	TS("TestNum10");

	debug_printf("10进制构造函数:.....\r\n");
	String str1((byte)123, 10);
	assert(str1 == "123", "String(byte value, int radix = 10)");

	String str2((short)4567, 10);
	assert(str2 == "4567", "String(short value, int radix = 10)");

	String str3((int)-88996677, 10);
	assert(str3 == "-88996677", "String(int value, int radix = 10)");

	String str4((uint)0xFFFFFFFF, 10);
	assert(str4 == "4294967295", "String(uint value, int radix = 10)");

	String str5((Int64)-7744, 10);
	assert(str5 == "-7744", "String(Int64 value, int radix = 10)");

	String str6((UInt64)331144, 10);
	assert(str6 == "331144", "String(UInt64 value, int radix = 10)");

	// 默认2位小数，所以字符串要补零
	String str7((float)123.0);
	assert(str7 == "123.00", "String(float value, int decimalPlaces = 2)");

	// 浮点数格式化的时候，如果超过要求小数位数，则会四舍五入
	String str8((double)456.784);
	assert(str8 == "456.78", "String(double value, int decimalPlaces = 2)");

	String str9((double)456.789);
	assert(str9 == "456.79", "String(double value, int decimalPlaces = 2)");
}

void TestNum16()
{
	TS("TestNum16");

	debug_printf("16进制构造函数:.....\r\n");
	String str1((byte)0xA3, 16);
	assert(str1 == "a3", "String(byte value, int radix = 16)");
	assert(String((byte)0xA3, -16) == "A3", "String(byte value, int radix = 16)");

	String str2((short)0x4567, 16);
	assert(str2 == "4567", "String(short value, int radix = 16)");

	String str3((int)-0x7799, 16);
	assert(str3 == "ffff8867", "String(int value, int radix = 16)");

	String str4((uint)0xFFFFFFFF, 16);
	assert(str4 == "ffffffff", "String(uint value, int radix = 16)");

	String str5((Int64)0x331144997AC45566, 16);
	assert(str5 == "331144997ac45566", "String(Int64 value, int radix = 16)");

	String str6((UInt64)0x331144997AC45566, -16);
	assert(str6 == "331144997AC45566", "String(UInt64 value, int radix = 16)");
}

static void TestAssign()
{
	TS("TestAssign");

	debug_printf("赋值构造测试\r\n");

	String str = "万家灯火，无声物联！";
	debug_printf("TestAssign: %s\r\n", str.GetBuffer());
	str	= "无声物联";
	assert(str == "无声物联", "String& operator = (const char* cstr)");

	String str2	= "xxx";
	str2	= str;
	assert(str == "无声物联", "String& operator = (const char* cstr)");
	assert(str2.GetBuffer() != str.GetBuffer(), "String& operator = (const String& rhs)");
}

static void TestConcat()
{
	TS("TestConcat");

	debug_printf("字符串连接测试\r\n");

	auto now	= Time.Now();
	//char cs[32];
	//debug_printf("now: %d %s\r\n", now.Second, now.GetString('F', cs));

	String str;

	// 连接时间，继承自Object
	str	+= now;
	//str.Show(true);
	// yyyy-MM-dd HH:mm:ss
	assert(str.Length() == 19, "String& operator += (const Object& rhs)");

	// 连接其它字符串
	int len	= str.Length();
	String str2(" 中国时间");
	str += str2;
	assert(str.Length() == len + str2.Length(), "String& operator += (const String& rhs)");

	// 连接C格式字符串
	str	+= " ";

	// 连接整数
	len	= str.Length();
	str += 1234;
	assert(str.Length() == len + 4, "String& operator += (int num)");

	// 连接C格式字符串
	str	+= " ";

	// 连接浮点数
	len	= str.Length();
	str += -1234.8856;	// 特别注意，默认2位小数，所以字符串是-1234.89
	str.Show(true);
	assert(str.Length() == len + 8, "String& operator += (double num)");
}

static void TestConcat16()
{
	TS("TestConcat16");

	String str = "十六进制连接测试 ";

	// 连接单个字节的十六进制
	str.Concat((byte)0x20, 16);

	// 连接整数的十六进制，前面补零
	str	+= " @ ";
	str.Concat((ushort)0xE3F, 16);

	// 连接整数的十六进制（大写字母），前面补零
	str += " # ";
	str.Concat(0x73F88, -16);

	str.Show(true);
	// 十六进制连接测试 20 @ 00000e3f # 00073F88
	assert(str == "十六进制连接测试 20 @ 00000e3f # 00073F88", "bool Concat(int num, int radix = 16)");
}

static void TestAdd()
{
	TS("TestAdd");

	String str = R("字符串连加 ");
	str = str + 1234 + "#" + R("99xx") + '$' + -33.883 + "@" + Time.Now();
	str.Show(true);
	// 字符串连加 1234@0000-00-00 00:00:00#99xx
	assert(str.Contains("字符串连加 1234#99xx$-33.88@"), "friend StringHelper& operator + (const StringHelper& lhs, const char* cstr)");
}

static void TestEquals()
{
	TS("TestEquals");

	debug_printf("字符串相等测试\r\n");

	String str1 = "TestABC HH";
	String str2 = "TESTABC HH";
	assert(str1 >= str2, "bool operator <  (const String& rhs) const");
	assert(str1.EqualsIgnoreCase(str2), "bool EqualsIgnoreCase(const String& s)");
}

static void TestSet()
{
	TS("TestSet");

	debug_printf("字符串设置测试\r\n");

	String str = "ABCDEFG";
	assert(str[3] == 'D', "char operator [] (int index)");

	str[5]	= 'W';
	assert(str[5] == 'W', "char& operator [] (int index)");
	//debug_printf("%s 的第 %d 个字符是 %c \r\n", str.GetBuffer(), 5, str[5]);

	str	= "我是ABC";
	int len	= str.Length();
	auto bs	= str.GetBytes();
	assert(bs.Length() == str.Length(), "ByteArray GetBytes() const");
	assert(bs[len - 1] == (byte)'C', "ByteArray GetBytes() const");
	//assert(bs.GetBuffer() == (byte*)str.GetBuffer(), "ByteArray GetBytes() const");

	// 十六进制字符串转为二进制数组
	str	= "36-1f-36-35-34-3F-31-31-32-30-32-34";
	auto bs2	= str.ToHex();
	bs2.Show(true);
	assert(bs2.Length() == 12, "ByteArray ToHex()");
	assert(bs2[1] == 0x1F, "ByteArray ToHex()");
	assert(bs2[5] == 0x3F, "ByteArray ToHex()");

	// 字符串搜索
	assert(str.IndexOf("36") == 0, "int IndexOf(const char* str, int startIndex = 0)");
	assert(str.IndexOf("36", 1) == 6, "int IndexOf(const char* str, int startIndex = 0)");
	assert(str.LastIndexOf("36", 6) == 6, "int LastIndexOf(const char* str, int startIndex = 0)");
	assert(str.LastIndexOf("36", 7) == -1, "int LastIndexOf(const char* str, int startIndex = 0)");
	assert(str.Contains("34-3F-31"), "bool Contains(const char* str) const");
	assert(str.StartsWith("36-"), "bool StartsWith(const char* str, int startIndex = 0)");
	assert(str.EndsWith("-32-34"), "bool EndsWith(const char* str)");
	
	// 字符串截取
	str	= " 36-1f-36-35-34\n";
	len	= str.Length();
	str	= str.Trim();
	assert(str.Length() == len - 2, "String& Trim()");
	
	str	= str.Substring(3, 5).ToUpper();
	str.Show(true);
	assert(str == "1F-36", "String Substring(int start, int _Length)");
}

void TestString()
{
	TS("TestString");

	TestCtor();
	TestNum10();
	TestNum16();
	TestAssign();
	TestConcat();
	TestConcat16();
	TestAdd();
	TestEquals();

	TestSet();

	debug_printf("字符串单元测试全部通过！");
}

