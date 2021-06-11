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

namespace tinyxml2 {
	class XMLElement;
}

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
		MUTABLE   = 0x0008, // Переменная может быть как RO, так и RW
		EXTERNAL  = 0x4000, //
		DUMP      = 0x8000,
//		EXTWRITED = 0x8000, //

		____ = NONE,
		R___ = READONLY,
		_S__ = SUWRITE,
		RS__ = READONLY | SUWRITE,
		__H_ = HIDE,
		R_H_ = READONLY | HIDE,
		_SH_ = SUWRITE | HIDE,
		RSH_ = READONLY | SUWRITE | HIDE,
		___D = DUMP,
		R__D = READONLY | DUMP,
		_S_D = SUWRITE | DUMP,
		RS_D = READONLY | SUWRITE | DUMP,
		__HD = HIDE | DUMP,
		R_HD = READONLY | HIDE | DUMP,
		_SHD = SUWRITE | HIDE | DUMP,
		RSHD = READONLY | SUWRITE | HIDE | DUMP,
	};

	rVariable(const std::string& name, TYPE type, UINT flags, void* pointer, STRID unit, UDINT access, const std::string& comment = "");
	virtual ~rVariable();

	const std::string& getName() const { return m_name; }
	const std::string& getComment() const { return m_comment; }
	TYPE  getType()    const { return m_type;   }
	UINT  getFlags()   const { return m_flags;  }
	STRID getUnit()    const { return m_unit;   }
	UDINT getAccess()  const { return m_access; }
	UDINT getHash()    const { return m_hash;   }
	bool  isReadonly() const { return m_flags & Flags::READONLY; }
	bool  isHide()     const { return m_flags & Flags::HIDE;     }
	bool  isSUWrite()  const { return m_flags & Flags::SUWRITE;  }
	bool  isMutable()  const { return m_flags & Flags::MUTABLE;  }
	bool  isExternal() const { return m_flags & Flags::EXTERNAL; }
	bool  isDumped()   const { return m_flags & Flags::DUMP;     }

	std::string saveToCSV() const;
	std::string valueToXml() const;
	std::string valueToString() const;
	void        valueFromXml(tinyxml2::XMLElement* root);

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

	void stringToValue(const std::string& value) const;

private:
	rVariable(rVariable& var);

protected:
	UDINT       m_hash    = 0;
	std::string m_name    = "";
	TYPE        m_type    = TYPE::UNDEF;
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

