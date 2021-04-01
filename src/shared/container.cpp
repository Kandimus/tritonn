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

Container::Container(void* buff, unsigned int size)
{
	m_data.reserve(1024 * 1024);

	add(buff, size);
}

Container& Container::add(const void* buff, unsigned int size)
{
	if (!buff) {
		return *this;
	}

	unsigned int oldsize = m_data.size();
	unsigned int newsize = m_data.size() + size;
	m_data.resize(newsize);
	memcpy(m_data.data() + oldsize, buff, size);

	m_isEOF = false;

	if (m_data.capacity() - m_data.size() < 16) {
		m_data.reserve(m_data.capacity() + 1024 * 1024);
	}

	return *this;
}

Container& Container::get(void* buff, unsigned int size)
{
	if (!buff || isEOF()) {
		return *this;
	}

	if (m_readpos + size >  m_data.size()) {
		m_isEOF   = true;
		return *this;
	}

	memcpy(buff, m_data.data() + m_readpos, size);
	m_readpos += size;

	return *this;
}

