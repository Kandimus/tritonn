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

class rVariableClass;

//-------------------------------------------------------------------------------------------------
//
class rVariable
{
	friend class rVariableClass;

public:
	enum Flags
	{
		NONE      = 0x0000,
		READONLY  = 0x0001, // Запись запрещена
		SUWRITE   = 0x0002, // Запись разрешена суперпользователю
		HIDE      = 0x0004, // Переменная не будет отображатся в ОРС-сервере
		LOADABLE  = 0x0008, // Переменная загружается из конфигурации
		EXTERNAL  = 0x4000, //
//		EXTWRITED = 0x8000, //

		____ = NONE,
		R___ = READONLY,
		_S__ = SUWRITE,
		RS__ = READONLY | SUWRITE,
		__H_ = HIDE,
		R_H_ = READONLY | HIDE,
		_SH_ = SUWRITE | HIDE,
		RSH_ = READONLY | SUWRITE | HIDE,
		___L = LOADABLE,
		R__L = READONLY | LOADABLE,
		_S_L = SUWRITE | LOADABLE,
		RS_L = READONLY | SUWRITE | LOADABLE,
		__HL = HIDE | LOADABLE,
		R_HL = READONLY | HIDE | LOADABLE,
		_SHL = SUWRITE | HIDE | LOADABLE,
		RSHL = READONLY | SUWRITE | HIDE | LOADABLE,
	};

	rVariable(const std::string& name, TT_TYPE type, UINT flags, void* pointer, STRID unit, UDINT access, const std::string& comment = "");
	virtual ~rVariable();

	const std::string& getName() const { return m_name; }
	const std::string& getComment() const { return m_comment; }
	TT_TYPE getType()    const { return m_type;   }
	UINT    getFlags()   const { return m_flags;  }
	STRID   getUnit()    const { return m_unit;   }
	UDINT   getAccess()  const { return m_access; }
	UDINT   getHash()    const { return m_hash;   }
	bool    isReadonly() const { return m_flags & Flags::READONLY; }
	bool    isHide()     const { return m_flags & Flags::HIDE;     }
	bool    isSUWrite()  const { return m_flags & Flags::SUWRITE;  }
	bool    isLodable()  const { return m_flags & Flags::LOADABLE; }
	bool    isExternal() const { return m_flags & Flags::EXTERNAL; }
	std::string saveKernel(UDINT offset, const std::string prefix) const;

	std::string saveToCSV();

	bool operator < (const rVariable* right) { return this->m_hash < right->m_hash; }

protected:
	struct rExternal
	{
		rVariable*  m_var = nullptr;
		char m_write[8] = {0};
		char m_read[8] = {0};
		bool m_isWrited = false;
	};

	rVariable(rVariable *var);

	bool    getBuffer(void* buffer) const;
	bool    setBuffer(void* buffer) const;

private:
	rVariable(rVariable& var);

protected:
	UDINT       m_hash    = 0;
	std::string m_name    = "";
	TT_TYPE     m_type    = TYPE_UNDEF;
	UINT        m_flags   = Flags::NONE;
	STRID       m_unit    = 0;
	UDINT       m_access  = 0;
	std::string m_comment = "";

	rExternal*  m_external = nullptr;
//	rVariable*  m_extVar;
//	char        m_extWrite[8];
//	char        m_extRead[8];

private:
	void* m_pointer; // Прямой указатель на данные. Использовать вне функций запрещено!
};

