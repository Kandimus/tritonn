//=================================================================================================
//===
//=== container.cpp
//===
//=== Copyright (c) 2021 by VeduN.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#include "container.h"
#include <string.h>

Container::Container()
{
	m_data.reserve(1024 * 1024);
}

Container::Container(const Container& cnt)
{
	m_data.reserve(1024 * 1024);

	m_data = cnt.m_data;
}

Container& Container::add(const void* buff, unsigned int size)
{
	const unsigned char* cbuff = static_cast<const unsigned char*>(buff);

	if (!buff) {
		return *this;
	}

	for (unsigned int ii = 0; ii < size; ++ii) {
		m_data.push_back(cbuff[ii]);
	}

	if (m_data.capacity() - m_data.size() < 16) {
		m_data.reserve(m_data.capacity() + 1024 * 1024);
	}

	return *this;
}

Container& Container::get(void* buff, unsigned int size)
{
	if (!buff) {
		return *this;
	}

	if (m_readpos + size >  m_data.size()) {
		m_readpos = m_data.size();
		return *this;
	}

	memcpy(buff, m_data.data() + m_readpos, size);
	m_readpos += size;

	return *this;
}

Container& Container::operator >> (char&  val)
{
	if (m_readpos + sizeof(val) >  m_data.size()) {
		m_readpos = m_data.size();
		return *this;
	}

	unsigned char tmp = m_data[m_readpos++];
	val = *(char *)&tmp;

	return *this;
}

Container& Container::operator >> (unsigned char&  val)
{
	if (m_readpos + sizeof(val) >  m_data.size()) {
		m_readpos = m_data.size();
		return *this;
	}

	memcpy(&val, m_data.data() + m_readpos, sizeof(val));
	m_readpos += sizeof(val);

	return *this;
}

Container& Container::operator >> (short& val)
{
	if (m_readpos + sizeof(val) >  m_data.size()) {
		m_readpos = m_data.size();
		return *this;
	}

	memcpy(&val, m_data.data() + m_readpos, sizeof(val));
	m_readpos += sizeof(val);

	return *this;
}

Container& Container::operator >> (unsigned short& val)
{
	if (m_readpos + sizeof(val) >  m_data.size()) {
		m_readpos = m_data.size();
		return *this;
	}

	memcpy(&val, m_data.data() + m_readpos, sizeof(val));
	m_readpos += sizeof(val);

	return *this;
}

Container& Container::operator >> (         int&   val)
{
	if (m_readpos + sizeof(val) >  m_data.size()) {
		m_readpos = m_data.size();
		return *this;
	}

	memcpy(&val, m_data.data() + m_readpos, sizeof(val));
	m_readpos += sizeof(val);

	return *this;
}

Container& Container::operator >> (unsigned int&   val)
{
	if (m_readpos + sizeof(val) >  m_data.size()) {
		m_readpos = m_data.size();
		return *this;
	}

	memcpy(&val, m_data.data() + m_readpos, sizeof(val));
	m_readpos += sizeof(val);

	return *this;
}

Container& Container::operator >> (long& val)
{
	if (m_readpos + sizeof(val) >  m_data.size()) {
		m_readpos = m_data.size();
		return *this;
	}

	memcpy(&val, m_data.data() + m_readpos, sizeof(val));
	m_readpos += sizeof(val);

	return *this;
}

Container& Container::operator >> (unsigned long& val)
{
	if (m_readpos + sizeof(val) >  m_data.size()) {
		m_readpos = m_data.size();
		return *this;
	}

	memcpy(&val, m_data.data() + m_readpos, sizeof(val));
	m_readpos += sizeof(val);

	return *this;
}

Container& Container::operator >> (double& val)
{
	if (m_readpos + sizeof(val) >  m_data.size()) {
		m_readpos = m_data.size();
		return *this;
	}

	memcpy(&val, m_data.data() + m_readpos, sizeof(val));
	m_readpos += sizeof(val);

	return *this;
}

Container& Container::operator >> (float&          val)
{
	if (m_readpos + sizeof(val) >  m_data.size()) {
		m_readpos = m_data.size();
		return *this;
	}

	memcpy(&val, m_data.data() + m_readpos, sizeof(val));
	m_readpos += sizeof(val);

	return *this;
}
