//=================================================================================================
//===
//=== error.cpp
//===
//=== Copyright (c) 2020 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс ошибки
//===
//=================================================================================================


#include "error.h"


UDINT rError::set(UDINT error, UDINT lineno, const std::string& text)
{
	if (m_error || !error) {
		return m_error;
	}

	m_error  = error;
	m_lineno = lineno;
	m_text   = text;

	return m_error;
}

void rError::clear()
{
	m_error  = TRITONN_RESULT_OK;
	m_lineno = 0;
	m_text   = "";
}
