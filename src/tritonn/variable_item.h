//=================================================================================================
//===
//=== variable_item.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс доступа к объектам и переменным из внешних источников. Массив данных структур
//=== заполняется на старте приложения и остается не изменным.
//===
//=================================================================================================

#pragma once

#include "def.h"
#include <string>

class rVariableList;
class rVariableClass;

//-------------------------------------------------------------------------------------------------
//
class rVariable
{
	friend class rVariableList;
	friend class rVariableClass;

public:
	enum Flags
	{
		NONE     = 0x0000,
		READONLY = 0x0001, // Запись запрещена
		SUWRITE  = 0x0002, // Запись разрешена суперпользователю
		HIDE     = 0x0004, // Переменная не будет отображатся в ОРС-сервере
		LOADABLE = 0x1000, // Переменная загружается из конфигурации
		EXTERNAL = 0x2000, //

		_____ = NONE,
		R____ = READONLY,
		_S___ = SUWRITE,
		RS___ = READONLY | SUWRITE,
		__H__ = HIDE,
		R_H__ = READONLY | HIDE,
		_SH__ = SUWRITE | HIDE,
		RSH__ = READONLY | SUWRITE | HIDE,
		___L_ = LOADABLE,
		R__L_ = READONLY | LOADABLE,
		_S_L_ = SUWRITE | LOADABLE,
		RS_L_ = READONLY | SUWRITE | LOADABLE,
		__HL_ = HIDE | LOADABLE,
		R_HL_ = READONLY | HIDE | LOADABLE,
		_SHL_ = SUWRITE | HIDE | LOADABLE,
		RSHL_ = READONLY | SUWRITE | HIDE | LOADABLE,
		____E = EXTERNAL,
		R___E = READONLY | EXTERNAL,
		_S__E = SUWRITE | EXTERNAL,
		RS__E = READONLY | SUWRITE | EXTERNAL,
		__H_E = HIDE | EXTERNAL,
		R_H_E = READONLY | HIDE | EXTERNAL,
		_SH_E = SUWRITE | HIDE | EXTERNAL,
		RSH_E = READONLY | SUWRITE | HIDE | EXTERNAL,
		___LE = LOADABLE | EXTERNAL,
		R__LE = READONLY | LOADABLE | EXTERNAL,
		_S_LE = SUWRITE | LOADABLE | EXTERNAL,
		RS_LE = READONLY | SUWRITE | LOADABLE | EXTERNAL,
		__HLE = HIDE | LOADABLE | EXTERNAL,
		R_HLE = READONLY | HIDE | LOADABLE | EXTERNAL,
		_SHLE = SUWRITE | HIDE | LOADABLE | EXTERNAL,
		RSHLE = READONLY | SUWRITE | HIDE | LOADABLE | EXTERNAL,
	};

	rVariable(const std::string& name, TT_TYPE type, UINT flags, void* pointer, STRID unit, UDINT access);
	virtual ~rVariable();

	const std::string& getName() const { return m_name; }
	TT_TYPE getType()    const { return m_type;   }
	UINT    getFlags()   const { return m_flags;  }
	STRID   getUnit()    const { return m_unit;   }
	UDINT   getAccess()  const { return m_access; }
	bool    getBuffer(void* buffer) const;
	bool    setBuffer(void* buffer) const;
	bool    isReadonly() const { return m_flags & Flags::READONLY; }
	bool    isHide()     const { return m_flags & Flags::HIDE;     }
	bool    isSUWrite()  const { return m_flags & Flags::SUWRITE;  }
	bool    isLodable()  const { return m_flags & Flags::LOADABLE; }
	bool    isExternal() const { return m_flags & Flags::EXTERNAL; }

protected:
	UDINT       m_hash;
	std::string m_name;
	TT_TYPE     m_type;
	UINT        m_flags;
	STRID       m_unit;
	UDINT       m_access;

	char  m_extWrite[8];
	char  m_extRead[8];

private:
	void* m_pointer; // Прямой указатель на данные. Использовать вне функций запрещено!
};

