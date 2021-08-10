/*
 *
 * crc32.h
 *
 * Copyright (c) 2018-2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */
#pragma once

class rCRC32
{
public:
	rCRC32(unsigned long poly) { m_poly = poly; init(); }
	rCRC32() { m_poly = 0xEDB88320; init(); }

	void init()
	{
		int jj;
		unsigned long c;

		memset(crc32_inv, 0, 256 * sizeof(crc32_inv[0]));

		for(int ii = 0; ii < 256; ++ii)
		{
			for(c = ii, jj = 0; jj < 8; ++jj)
			{
				//c = (c >> 1) ^ (CRC32_POLY & (-(signed long)(c & 1)));
				c = (c & 1) ? ((c >> 1) ^ m_poly) : (c >> 1);
			}
			crc32_table[ii] = c;

			crc32_inv[(unsigned char)(c >> 24) & 0xFF] = (c << 8) ^ ii;
		}
	}

	unsigned long update(unsigned long crc, unsigned char byte)
	{
		return crc32_table[(crc & 0xFF) ^ byte] ^ (crc >> 8);
	}

	unsigned long get(void* buf, unsigned long size)
	{
		unsigned char* cbuff = static_cast<unsigned char*>(buf);
		unsigned long crc = 0xffffffff;

		while (size--) {
			crc = update(crc, *cbuff++);
		}
		return ~crc;
	}

	unsigned long getReversedBytes(unsigned long reg_start, unsigned long reg_end)
	{
		reg_start ^= 0xffffffff;
		reg_end   ^= 0xffffffff;

		for(int ii = 0; ii < 4; ++ii)
		{
			for(int t_index = 0; t_index < 256; t_index++)
			{
				if((crc32_table[t_index] & 0xff000000) == (reg_end & 0xff000000))
				{
					reg_end  ^= crc32_table[t_index];
					reg_end <<= 8;
					reg_end  ^= t_index ^ (((0xff000000 >> (ii * 8)) & reg_start) >> ((3 - ii) * 8));
					break;
				}
			}
		}

		return reg_end;
	}

private:
	unsigned long m_poly;

//private:
	unsigned long crc32_table[256];
	unsigned long crc32_inv[256];
};


class rCRC16
{
public:
	rCRC16()
	{
		unsigned short r;

		for(int ii = 0; ii < 256; ++ii)
		{
			//r = ((unsigned short)ii) << 8;
			r = ii;
			
			for(int jj = 0; jj < 8; ++jj)
			{
				r = (r & 1) ? ((r >> 1) ^ 0xA001) : (r >> 1);
			}

			crc16_table[ii] = r;
		}
	}

	unsigned short getModbus(unsigned char *buf,unsigned int Sz)
	{
		unsigned int Code = 0xFFFF;
		unsigned int Flag;
		unsigned int i;
		unsigned int j;

		for (i=0;i<Sz;i++)
		{
			Code ^= buf[i];
			for(j=0;j<8;j++)
			{
				Flag = Code&01;
				Code = (Code >> 1) & 0177777;
				if (Flag) Code ^= 0xA001;
			}
		}
		return (Code);
	}

	unsigned short update(unsigned short Code, unsigned char buf)
	{
		return crc16_table[(Code & 0xFF) ^ buf] ^ (Code >> 8);
		//return crc16_table[((Code >> 8) ^ buf) & 0xFF] ^ (Code << 8);

		for(int ii = 0; ii < 8; ++ii)
		{
			if(((Code & 0x8000) >> 8) ^ (buf & 0x80))
			{
				Code = (Code << 1) ^ 0x8005;
			}
			else
			{
				Code = (Code << 1);
			}

			buf <<= 1;
		}

		return Code;
	}

	unsigned short get(unsigned char *buf, unsigned short size)
	{
		unsigned short crc = 0xffff;

		while(size--)
		{
			crc = update(crc, *buf++);
		}
		return ~crc;
	}


	unsigned short crc16_table[256];
};	
