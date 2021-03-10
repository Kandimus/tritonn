//=================================================================================================
//===
//=== error.h
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

#pragma once

#include <string>
#include "def.h"

class rError
{
public:
	rError() = default;
	virtual ~rError() = default;

	UDINT set(UDINT error, UDINT lineno, const std::string& text = "");
	void clear();

	UDINT       getError()  const { return m_error; }
	UDINT       getLineno() const { return m_lineno; }
	std::string getText()   const { return m_text; }

private:
	UDINT m_error  = TRITONN_RESULT_OK;
	UDINT m_lineno = 0;
	std::string m_text = "";
};

