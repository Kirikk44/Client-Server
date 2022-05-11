#pragma once
#include <iostream>
#include <vector>
#include <iterator>
#include <iomanip>
#include <sstream>
#include <cmath>

using namespace std;
class MD5
{
	typedef unsigned int uint4;
	typedef unsigned char uint1;


private:
	uint4 bits512[16];
	uint4 size_byte{ 0 };
	uint4 state[4]; 
	uint1 digest[16]; // the result

	uint4 T[64];
	uint4 S[64] = { 7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
					5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
					4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
					6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21 };

private:
	static inline uint4 F(uint4 x, uint4 y, uint4 z);
	static inline uint4 G(uint4 x, uint4 y, uint4 z);
	static inline uint4 H(uint4 x, uint4 y, uint4 z);
	static inline uint4 I(uint4 x, uint4 y, uint4 z);

	static inline uint4 CLS(uint4 x, int n);

	int to_uint32(const char data[], uint4 first, uint4 len); // строку в массив unsigned int
	void to_uint32_cut(int flag);
	void hmd5();

	void hexdigest(char buf[]) const;
	void encode();

	void _init()
	{
		state[0] = 0x67452301;
		state[1] = 0xefcdab89;
		state[2] = 0x98badcfe;
		state[3] = 0x10325476;

		size_byte = 0;
	}

public:
	void new_block(char data[], uint4 len);

	MD5()
	{
		_init();
		for (int i = 0; i < 64; i++)
			T[i] = uint4(pow(2, 32) * abs(sin(double(i) + 1)));
	}

};

void MD5::to_uint32_cut(int count)
{
	int i;

	for (i = 0; i < 16; i++)
	{
		bits512[i] = 0;
	}
	if (count == 0)
	{
		bits512[0] |= 1 << 31;
	}
	bits512[14] = (uint4)((long long)(size_byte) * 8);

	bits512[15] = (uint4)((long long)(size_byte) * 8 >> 32);
}


void MD5::new_block(char data[], uint4 count_bytes)
{
	to_uint32(data, 0, count_bytes);
	hmd5();
	if (count_bytes < 64)
	{
		if (count_bytes >= 56 || count_bytes == 0)
		{
			to_uint32_cut(count_bytes);
			hmd5();
		}
		encode();
		hexdigest(data);
		_init();
	}
}

int MD5::to_uint32(const char data[], uint4 first, uint4 len)
{
	int i = 0, cnt = 0;
	bool tmp = 1;
	size_byte += len;

	for (i = 0; i < 16; i++)
	{
		bits512[i] = 0;
	}
	
	for (i = first; i < 64 && i < len; i++)
	{
		bits512[cnt] |= ((uint4)data[i] << ((i % 4) * 8));
		if ((i + 1) % 4 == 0)
		{
			cnt++;
		}
	}

	if (i == 64)
		return 0;


	if (i * 8 % 512 < 448 && i * 8 % 512 != 0)
	{
		bits512[cnt] |= 1 << (((3 - (i) % 4) * 8) - 1);

		bits512[14] = (uint4)((long long)(size_byte) * 8);

		bits512[15] = (uint4)((long long)(size_byte) * 8 >> 32);
		return 1;
	}
	else
	{
		if (i * 8 % 512 != 0)
		{
			bits512[cnt] |= 1 << (((i % 4) * 8) - 1);
		}
		else
		{
			return 2;
		}
		return 1;
	}
}

void MD5::encode()
{

	for (uint4 i = 0, j = 0; j < 16; i++, j += 4) {
		digest[j] = state[i] & 0xff;
		digest[j + 1] = (state[i] >> 8) & 0xff;
		digest[j + 2] = (state[i] >> 16) & 0xff;
		digest[j + 3] = (state[i] >> 24) & 0xff;
	}
}

void MD5::hmd5()
{
	uint4 a = state[0];
	uint4 b = state[1];
	uint4 c = state[2];
	uint4 d = state[3];

	uint4 f, k;

	for (int i = 0; i < 64; ++i) {
		if (i < 16) {	//F
			f = F(b, c, d);
			k = i;
		}
		else if (i < 32) {
			f = G(b, c, d);
			k = (5 * i + 1) % 16;
		}
		else if (i < 48) {
			f = H(b, c, d);
			k = (3 * i + 5) % 16;
		}
		else if (i < 64) {
			f = I(b, c, d);
			k = (7 * i) % 16;
		}

		unsigned int t = b + CLS(a + f + bits512[k] + T[i], S[i]);
		unsigned int temp = d;
		d = c;
		c = b;
		b = t;
		a = temp;
	}

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;

}

inline MD5::uint4 MD5::F(uint4 x, uint4 y, uint4 z) {
	return (x & y) | (~x & z);
}


inline MD5::uint4 MD5::G(uint4 x, uint4 y, uint4 z) {
	return (x & z) | (y & ~z);
}


inline MD5::uint4 MD5::H(uint4 x, uint4 y, uint4 z) {
	return x ^ y ^ z;
}


inline MD5::uint4 MD5::I(uint4 x, uint4 y, uint4 z) {
	return y ^ (x | ~z);
}


inline MD5::uint4 MD5::CLS(uint4 x, int n) {
	return (x << n) | (x >> (32 - n));
}


void MD5::hexdigest(char buf[]) const
{

	for (int i = 0; i < 16; i++)
		sprintf(buf + i * 2, "%02x", digest[i]);
	buf[32] = 0;

}