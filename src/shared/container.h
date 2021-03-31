//=================================================================================================
//===
//=== container.h
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#pragma once

#include <vector>

class Container
{
public:
	Container();
	Container(const Container& cnt);
	virtual ~Container() = default;

	Container& operator << (const          char  val) { m_data.push_back(val); return *this; }
	Container& operator << (const unsigned char  val) { add(&val, sizeof(unsigned char )); return *this; }
	Container& operator << (const          short val) { add(&val, sizeof(short         )); return *this; }
	Container& operator << (const unsigned short val) { add(&val, sizeof(unsigned short)); return *this; }
	Container& operator << (const          int   val) { add(&val, sizeof(int           )); return *this; }
	Container& operator << (const unsigned int   val) { add(&val, sizeof(unsigned int  )); return *this; }
	Container& operator << (const          long  val) { add(&val, sizeof(long          )); return *this; }
	Container& operator << (const unsigned long  val) { add(&val, sizeof(unsigned long )); return *this; }
	Container& operator << (const double         val) { add(&val, sizeof(double        )); return *this; }
	Container& operator << (const float          val) { add(&val, sizeof(float         )); return *this; }

	Container& operator >> (         char&  val);
	Container& operator >> (unsigned char&  val);
	Container& operator >> (         short& val);
	Container& operator >> (unsigned short& val);
	Container& operator >> (         int&   val);
	Container& operator >> (unsigned int&   val);
	Container& operator >> (         long&  val);
	Container& operator >> (unsigned long&  val);
	Container& operator >> (double&         val);
	Container& operator >> (float&          val);

	Container& add(const void* buff, unsigned int size);
	Container& get(void* buff, unsigned int size);

	const unsigned char* getRaw() const { return m_data.data(); }

	bool isEnd() const { return m_readpos >= m_data.size(); }
	unsigned int size() const { return m_data.size(); }

protected:
	std::vector<unsigned char> m_data;
	unsigned int m_readpos = 0;
};
