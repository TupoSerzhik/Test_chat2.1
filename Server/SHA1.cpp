#include "SHA1.h"
#include <sstream>
#include <iomanip>
#include <cstring>
#include <iostream>

#define LEFT_ROTATE(x,n) (((x) << (n)) | ((x) >> (32 -(n))))

void SHA1::processBlock(const uint8_t* block) const
{
	uint8_t w[80];
	for (int i = 0; i < 16; i++)
	{
		w[i] = (block[i * 4] << 24) |
			(block[i * 4 + 1] << 16) |
			(block[i * 4 + 2] << 8) |
			(block[i * 4 + 3]);
	}
	for (int i = 16; i < 80; i++)
	{
		w[i] = LEFT_ROTATE(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
	}
	uint32_t a = h0;
	uint32_t b = h1;
	uint32_t c = h2;
	uint32_t d = h3;
	uint32_t e = h4;

	for (int i = 0; i < 80; i++)
	{
		uint32_t f, k;

		if (i < 20)
		{
			f = (b & c) | ((~b) & d);
			k = 0x5A827999;
		}
		else if (i < 40)
		{
			f = b ^ c ^ d;
			k = 0x6ED9EBA1;

		}
		else if (i < 60)
		{
			f = (b & c) | (b & d) | (c & d);
			k = 0x8F1BBCDC;
		}
		else
		{
			f = b ^ c ^ d;
			k = 0xCA62C1D6;
		}

		uint32_t temp = LEFT_ROTATE(a, 5) + f + e + k + w[i];
		e = d;
		d = c;
		c = LEFT_ROTATE(b, 30);
		b = a;
		a = temp;
	}
	h0 += a;
	h1 += b;
	h2 += c;
	h3 += d;
	h4 += e;
}

void SHA1::padMessage(std::vector<uint8_t>& message) const
{
	uint64_t originalLenght = message.size() * 8;

	message.push_back(0x80);
	while ((message.size() % 64) != 56)
	{
		message.push_back(0x00);
	}
	for (int i = 7; i >= 0; i--)
	{
		message.push_back((originalLenght >> (i * 8)) & 0xFF);
	}
}

std::string SHA1::digestToString() const
{
	std::stringstream ss;
	ss << std::hex << std::setfill('0');

	ss << std::setw(8) << h0;
	ss << std::setw(8) << h1;
	ss << std::setw(8) << h2;
	ss << std::setw(8) << h3;
	ss << std::setw(8) << h4;
	return ss.str();
}

SHA1::SHA1() : h0(0x67452301), h1(0xEFCDAB89), h2(0x98BADCFE), h3(0x10325476), h4(0xC3D21)
{
}

std::string SHA1::hash(const std::string& input) const
{
	uint32_t orig_h0 = h0;
	uint32_t orig_h1 = h1;
	uint32_t orig_h2 = h2;
	uint32_t orig_h3 = h3;
	uint32_t orig_h4 = h4;

	std::vector<uint8_t> message(input.begin(), input.end());

	padMessage(message);

	for (size_t i = 0; i < message.size();i += 64)
	{
		processBlock(&message[i]);
	}
	std::string result = digestToString();

	h0 = orig_h0;
	h1 = orig_h1;
	h2 = orig_h2;
	h3 = orig_h3;
	h4 = orig_h4;

	return result;
}


