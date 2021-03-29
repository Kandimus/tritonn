//=================================================================================================
//===
//=== event/manager.cpp
//===
//=== Copyright (c) 2019-2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс-поток для регистрации событий, как системных, так и пользователя.
//===
//=== Все события сохраняются в энергонезависимую память
//===
//=================================================================================================

#include "manager.h"
#include <string.h>
#include "client.h"
#include "log_manager.h"
#include "../text_manager.h"
#include "../precision.h"
#include "../error.h"
#include "simplefile.h"
#include "locker.h"


rEventManager::rEventManager()
	: rTCPClass("0.0.0.0", LanPort::PORT_EVENT, ClientCount::MAX)
{
	RTTI = "rEventManager";

	pthread_mutex_init(&m_mutexList, NULL);

	LoadEEPROM();
}


rEventManager::~rEventManager()
{
	pthread_mutex_destroy(&m_mutexList);
	//SaveEEPROM();
}

rClientTCP *rEventManager::NewClient(SOCKET socket, sockaddr_in *addr)
{
	return (rClientTCP *)new rEventClient(socket, addr);
}

UDINT rEventManager::ClientRecv(rClientTCP *client, USINT *buff, UDINT size)
{
	UNUSED(client);
	UNUSED(buff);
	UNUSED(size);
	return TRITONN_RESULT_OK;
}


rThreadStatus rEventManager::Proccesing()
{
	// Возможно именно тут нужно записывать потихоньку данные в EEPROM, а если скорость будет нормальной, то эта нить не нужна.
	rThreadStatus thread_status = rThreadStatus::UNDEF;

	while(1)
	{
		// Обработка команд нити
		thread_status = rThreadClass::Proccesing();
		if(!THREAD_IS_WORK(thread_status))
		{
			return thread_status;
		}

		rThreadClass::EndProccesing();
	}

	return rThreadStatus::UNDEF;
}


//-------------------------------------------------------------------------------------------------
// Основная функция добавления события
void rEventManager::add(const rEvent& event)
{
	UDINT type    = event.getType();
	UDINT mask    = LOG::EVENT;
	UDINT log_obj = 0;

	if(event.getEID() == 0)
	{
		TRACEP(LOG::EVENTMGR, "Fault add event. EID is null.");
		return;
	}

	// Добавляем событие в кольцевой массив
	{
		rLocker lock(&m_mutexList);

		while (m_list.size() > MAX_EVENT) {
			m_list.pop_front();
		}
		m_list.push_back(event);
	}

	// Проверка на аварийное сообщение
	if (type == EMT_ERROR) {
		UDINT curalarm = m_alarm.Get();
		
		if(curalarm < MAX_ALARM) {
			m_alarm.Set(++curalarm);
		}
	}
	
	switch(type)
	{
		case EMT_INFO:    mask |= LOG::I; break;
		case EMT_SUCCESS: mask |= LOG::I; break;
		case EMT_WARNING: mask |= LOG::W; break;
		case EMT_ERROR:   mask |= LOG::A; break;
	}
	
	log_obj = event.getObject();
	if (log_obj > EVENT_OBJ_MAX) {
		log_obj = EVENT_OBJ__END;
	}
	
	//Выдаем расшифровку сообщения
	std::string descr = getDescr(event);

	rLogManager::instance().add(mask, event.getTime(), descr.c_str());
	
	save(event);
//	SaveEEPROM(curpos * sizeof(rEvent), event);
}


void rEventManager::addEvent(DINT eid)
{
	rEvent event(eid);

	add(event);
}

void rEventManager::addEventUDINT(DINT eid, UDINT val)
{
	rEvent event(eid);

	add(event << val);
}

//-------------------------------------------------------------------------------------------------
//UDINT rEventManager::Get(rEventArray &arr)
//{
//	Lock();
	
//	memcpy((void*)&arr, (const void *)&Event, sizeof(rEventArray));
	
//	Unlock();
	
//	return 0;
//}


//-------------------------------------------------------------------------------------------------
//
std::string rEventManager::getDescr(const rEvent &event)
{
	string  text   = "";
	string  result = "";

	// Получим шаблон описания события
	if(!m_texts.get(event.getEID(), text))
	{
		result = String_format("Event %u does not exist", event.getEID());

		TRACEP(LOG::EVENTMGR, result.c_str());
		return result;
	}

	if(text.empty())
	{
		result = String_format("Event %u have not decription", event.getEID());

		TRACEP(LOG::EVENTMGR, result.c_str());
		return result;
	}

	// Начнем подставлять данные в описание события
	for (UDINT ii = 0; ii < text.size(); ++ii) {
		// Это не начало параметра
		if('$' != text[ii] || ii == text.size() - 1)
		{
			result += text[ii];
			continue;
		}

		// Двойной $$ превращаем в одинарный
		if('$' == text[ii + 1])
		{
			result += '$';
			++ii;
			continue;
		}

		// Если следующий символ не цифра, то скорее всего это ошибка, и пропускаем такие комбинации
		if(!isdigit(text[ii + 1]))
		{
			result += text[ii];
			continue;
		}

		// Сюда попадем только если встретили $[0-9]
		result += parseParameter(event, text.c_str() + ii + 1, ii);
	} // for

	return result;
}



std::string rEventManager::parseParameter(const rEvent& event, const char* str, UDINT& offset)
{
	static std::string floatformat[3] = {"%.*f", "%.*e", "%.*E"};

	UDINT   number = 0;
	UDINT   prec   = 0xFFFFFFFF;
	UDINT   exp    = 0;
	UDINT   type   = 0;
	void   *data   = nullptr;

	offset += parseNumber(str, number, prec, exp);
	data    = event.getParamByID(number - 1, type); // Номера указываются с 1, а по факту с 0

	if(data == nullptr) {
		TRACEP(LOG::EVENTMGR, "Event eid %i: unknow param %i", event.getEID(), number);
		return "<?>";
	}

	// Если точность не была указана, то ищем предшествующие единицы измерений
	if(prec == 0xFFFFFFFF && (type == TYPE_LREAL || type == TYPE_REAL))
	{
		prec = rPrecision::DEFAUILT;

		// Получаем значение точности из предыдущей ед.измерения
		for(DINT jj = number - 2; jj >= 0; --jj)
		{
			UDINT  sidtype = 0;
			UDINT* sid     = (UDINT *)event.getParamByID(jj, sidtype);

			// Нашли ед.измерения
			if(sidtype == TYPE_STRID && *sid < MAX_UNITS_COUNT)
			{
				prec = rPrecision::instance().get(*sid);
				break;
			}
		}
	}

	// форматируем результат
	switch(type)
	{
		case TYPE_UNDEF:
			TRACEP(LOG::EVENTMGR, "Event eid %i, param %i: undefine type", event.getEID(), number);
			return "<undefine type>";

		case TYPE_SINT : return String_format("%hhi", *( SINT *)data);
		case TYPE_USINT: return String_format("%hhu", *(USINT *)data);
		case TYPE_INT  : return String_format("%hi" , *(  INT *)data);
		case TYPE_UINT : return String_format("%hu" , *( UINT *)data);
		case TYPE_DINT : return String_format("%i"  , *( DINT *)data);
		case TYPE_UDINT: return String_format("%u"  , *(UDINT *)data);
		case TYPE_REAL : return String_format(floatformat[exp].c_str(), prec, *( REAL *)data);
		case TYPE_LREAL: return String_format(floatformat[exp].c_str(), prec, *(LREAL *)data);
		case TYPE_STRID:
		{
			const std::string* str = rTextManager::instance().getPtr(*(UDINT *)data);
			return (str == nullptr) ? String_format("<unknow sid %u>", *(UDINT *)data) : *str;
		}

		default:
			TRACEP(LOG::EVENTMGR, "Event eid %i, param %i: unknow type", event.getEID(), number);
			return "<unknow type>";
	}
}



//-------------------------------------------------------------------------------------------------
// Парсинг номеров параметров вида: $1, $2.3, $4.e5, $6.e
UDINT rEventManager::parseNumber(const char* str, UDINT& num, UDINT& prec, UDINT& exp)
{
	const UDINT STATE_NUMBER = 0; // Состояния конечного автомата
	const UDINT STATE_DOT    = 1;
	const UDINT STATE_EXP    = 2;
	const UDINT STATE_PREC   = 3;

	UDINT   state   = STATE_NUMBER;
	string  str_num = "";
	char   *curch   = (char *)str;

	while(0 != *curch)
	{
		switch(state)
		{
			case STATE_NUMBER:
			{
				// Если встретили точку, то получим номер параметра и перейдем в
				// стостояние STATE_EXP (проверка на наличие знака 'e')
				if('.' == *curch)
				{
					if(str_num.empty()) return 0;

					num     = (str_num.size()) ? atoi(str_num.c_str()) : num;
					state   = STATE_DOT;
					str_num = "";
				}
				// Если это цифра, то добавим ее к строковому представлению
				else if(isdigit(*curch))
				{
					str_num += *curch;
					++curch;
				}
				// Если это не точка и не число, то получаем номер парамера и выходим
				else
				{
					num     = (str_num.size()) ? atoi(str_num.c_str()) : num;
					str_num = "";

						  return static_cast<UDINT>(curch - str);
				}
				break;
			}

			case STATE_DOT:
			{
				++curch; // перешли на следующий символ

				if('e' == *curch || 'E' == *curch)
				{
					state = STATE_EXP;
				}
				else if(isdigit(*curch))
				{
					state = STATE_PREC;
				}
				else
				{
					--curch;
					return static_cast<UDINT>(curch - str);
				}
				break;
			}

			case STATE_EXP:
			{
				++curch;
				str_num = "";
				exp     = ('e' == *curch) ? 1 : 2;

				if(isdigit(*curch))
				{
					str_num = "";
					state   = STATE_PREC;
				}
				else
				{
					return static_cast<UDINT>(curch - str);
				}
				break;
			}

			case STATE_PREC:
			{
				if(isdigit(*curch))
				{
					str_num += *curch;
					++curch;
				}
				else
				{
					prec = (str_num.size()) ? atoi(str_num.c_str()) : prec;

					return static_cast<UDINT>(curch - str);
				}
				break;
			}

			default: return 0;
		}
	}

	// Сюда попадем, только если параметр находился в конце строки
	if(state == STATE_NUMBER)
	{
		num = (str_num.size()) ? atoi(str_num.c_str()) : num;
	}
	else
	{
		prec = (str_num.size()) ? atoi(str_num.c_str()) : prec;
	}

	return static_cast<UDINT>(curch - str);
}


UDINT rEventManager::LoadEEPROM()
{
	return 0;
}


UDINT rEventManager::SaveEEPROM(int /*pos*/, rEvent &/*event*/)
{
	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rEventManager::loadText(const string& filename)
{
	rError err;

	if (TRITONN_RESULT_OK != m_texts.loadSystem(filename, err)) {
		TRACEP(LOG::EVENTMGR, "Can't load system event. Error %i, line %i", err.getError(), err.getLineno());
		exit(0);
		//TODO Перейти в HALT
	}

	return err.getError();
}


//
UDINT rEventManager::setCurLang(const std::string& lang)
{
	return m_texts.setCurLang(lang);
}


void rEventManager::save(const rEvent& event)
{
	std::string filename = DIR_EVENT + String_format("%u.event", event.getTime().getSec() / rDateTime::SEC_IN_DAY);
	std::string text = event.toString() + "\n";

	SimpleFileAppend(filename, text);
}


UDINT rEventManager::startServer()
{
	return rTCPClass::StartServer("0.0.0.0", LanPort::PORT_EVENT);
}
