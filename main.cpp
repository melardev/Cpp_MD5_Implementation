#include <cstdio>
#include <string>
#include <cstdint>
#include <iostream>
#include <bitset>


template <typename T>
void showBinaryRepresentation(const T& c)
{
	const char* beg = reinterpret_cast<const char*>(&c);
	const char* end = beg + sizeof(c);
	while (beg != end)
		std::cout << std::bitset<8>(*beg++) << ' ';
	std::cout << '\t';
}


uint32_t toInt32(uint8_t* c)
{
	return c[0] << 0 | c[1] << 8 | c[2] << 16 | c[3] << 24;
}

uint32_t leftRotate(uint32_t val, uint32_t bits)
{
	return val << bits | val >> (32 - bits);
}

int main()
{
	/*
	1. Pad the input message
	2. Add a 64-bit binary string that is the representation of the
	message's length.
	3. Initialize four 32-bit values.
	4. Process each 512 bytes
	5. Generate a 128 bit output (128/8 = 16 bytes)
	*/

	char message[1024] = "The quick fox jumps over the lazy dog";
	const size_t initial_len = strlen(message);

	//1. Add padding bits behind the input message.
	message[initial_len] = '\x80';
	size_t new_length = initial_len + 1;

	// calculate new length
	// new_length should be 418 Mod 512, so new_length Mod 512 should be the same as 448
	while (new_length % (512 / 8) != (448 / 8))
		new_length++;

	// Initialize the bytes with 0x00
	for (size_t i = initial_len + 1; i < (new_length - initial_len + initial_len); i++)
		message[i] = '\x00';

	// 2. Append the initial length (64-bits integer) 
	unsigned char* msg;
	msg = new unsigned char[new_length + 8];
	memcpy(msg, message, new_length);

	const uint32_t hightest_len = (initial_len * 8) & 0xffffffff;
	msg[new_length + 0] = hightest_len >> 0 & 0xff;
	msg[new_length + 1] = hightest_len >> 8 & 0xff;
	msg[new_length + 2] = hightest_len >> 16 & 0xff;
	msg[new_length + 3] = hightest_len >> 24 & 0xff;


	const uint32_t lowest_len = initial_len >> 29; // the same as initial_len * 8 >> 32
	msg[new_length + 4] = lowest_len >> 0 & 0xff;
	msg[new_length + 5] = lowest_len >> 8 & 0xff;
	msg[new_length + 6] = lowest_len >> 16 & 0xff;
	msg[new_length + 7] = lowest_len >> 24 & 0xff;

	new_length += 8;
	for (size_t i = 0; i < new_length; i++)
		showBinaryRepresentation(msg[i]);


	uint32_t T[64];
	for (int i = 0; i < 64; i++)
		T[i] = 4294967296 * abs(sin(i + 1));

	//How many left rotate

	const uint32_t rotate[] = {
		7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
		5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
		4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
		6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
	};

	//Step 3. Initialize MD Buffer
	uint32_t h0 = 0x67452301;
	uint32_t h1 = 0xefcdab89;
	uint32_t h2 = 0x98badcfe;
	uint32_t h3 = 0x10325476;
	size_t offset = 0;
	do
	{
		//we take a block of 512 bits and we split them in 16 DWORD which is 64 bytes 
		int w[16];
		for (int i = 0; i < 16; i++)
			w[i] = toInt32(4 * i + offset + msg);


		int a, b, c, d, f, g, temp;
		a = h0;
		b = h1;
		c = h2;
		d = h3;

		//now that we have 64 bytes we use each of them
		for (int i = 0; i < 64; i++)
		{
			if (i < 16)
			{
				f = (b & c) | ((~b) & d); //b = X c = Y d = Z
				g = i;
			}
			else if (i < 32)
			{
				f = (d & b) | ((~d) & c);
				g = (5 * i + 1) % 16;
			}
			else if (i < 48)
			{
				f = b ^ c ^ d;
				g = (3 * i + 5) % 16;
			}
			else
			{
				f = c ^ (b | (~d));
				g = (7 * i) % 16;
			}

			/*a = d;
			temp = b;
			b = b << 8;
			c = temp;
			d = c;*/

			temp = d;
			d = c;
			c = b;
			b = b + leftRotate((a + f + T[i] + w[g]), rotate[i]);
			a = temp;

			std::cout << std::endl;
			std::cout << "Round [ " << i + 1 << " ]" << std::endl;
			std::cout << "h0: ";
			showBinaryRepresentation(a);
			std::cout << std::endl;
			std::cout << "h1: ";
			showBinaryRepresentation(b);
			std::cout << std::endl;
			std::cout << "h2: ";
			showBinaryRepresentation(c);
			std::cout << std::endl;
			std::cout << "h3: ";
			showBinaryRepresentation(d);
			std::cout << std::endl;
		}
		// Add this chunk's hash to result so far:
		h0 += a;
		h1 += b;
		h2 += c;
		h3 += d;
		offset += 64;
	}
	while (offset < new_length);

	uint8_t* digest = new uint8_t[16 + 1];

	digest[0] = h0 >> 0 & 0xff;
	digest[1] = h0 >> 8 & 0xff;
	digest[2] = h0 >> 16 & 0xff;
	digest[3] = h0 >> 24 & 0xff;

	digest[4] = h1 >> 0 & 0xff;
	digest[5] = h1 >> 8 & 0xff;
	digest[6] = h1 >> 16 & 0xff;
	digest[7] = h1 >> 24 & 0xff;

	digest[8] = h2 >> 0 & 0xff;
	digest[9] = h2 >> 8 & 0xff;
	digest[10] = h2 >> 16 & 0xff;
	digest[11] = h2 >> 24 & 0xff;

	digest[12] = h3 >> 0 & 0xff;
	digest[13] = h3 >> 8 & 0xff;
	digest[14] = h3 >> 16 & 0xff;
	digest[15] = h3 >> 24 & 0xff;
	digest[16] = '\00';

	for (int j = 0; j < strlen((char*)digest); j++)
		printf("%02x", digest[j]);


	delete[] msg;
	delete[] digest;
	return 0;
}
