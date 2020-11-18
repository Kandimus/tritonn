//=================================================================================================
//===
//=== event_manager.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
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

#include <string.h>
#include "log_manager.h"
#include "text_manager.h"
#include "precision.h"
#include "event_manager.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
//

//-------------------------------------------------------------------------------------------------
//
rEventManager::rEventManager()
{
	RTTI = "rEventManager";

	LoadEEPROM();
}


rEventManager::~rEventManager()
{
	//SaveEEPROM();
}

//-------------------------------------------------------------------------------------------------
//


//-------------------------------------------------------------------------------------------------
// Основная функция добавления события
UDINT rEventManager::Add(rEvent &event)
{
	UDINT curpos  = 0;
	UDINT type    = event.GetType();
	UDINT mask    = LM_EVENT;
	UDINT log_obj = 0;

//	if(event.GetEID() == 0)
//	{
//		int a = 1;
//		a = 2;
//	}

	// Добавляем событие в кольцевой массив
	Lock();
	Event[Pos++] = event;
	if(Pos >= MAX_EVENT)
	{
		Pos = 0;
	}
	curpos = Pos;
	Unlock();

	// Проверка на аварийное сообщение
	if(type == EMT_ERROR)
	{
		UDINT curalarm = Alarm.Get();
		
		if(curalarm < MAX_EVENT_ALARM)
		{
			Alarm.Set(++curalarm);
		}
	}
	

	//----------------------------------------------
	// Пишем логи в rsyslog
	
	switch(type)
	{
		case EMT_INFO:    mask |= LM_I; break;
		case EMT_SUCCESS: mask |= LM_I; break;
		case EMT_WARNING: mask |= LM_W; break;
		case EMT_ERROR:   mask |= LM_A; break;
	}
	
	log_obj = event.GetObject();
	if(log_obj > EVENT_OBJ_MAX) log_obj = EVENT_OBJ__END;
	

	//Выдаем расшифровку сообщения
	string descr = GetDescr(event);

	rLogManager::Instance().Add(mask, __FILENAME__, __LINE__, descr.c_str());

	
	//----------------------------------------------
	SaveEEPROM(curpos * sizeof(rEvent), event);

	return curpos;
}


UDINT rEventManager::AddEvent(DINT eid)
{
	rEvent event(eid);

	return Add(event);
}


UDINT rEventManager::AddEventUDINT(DINT eid, UDINT val)
{
	rEvent event(eid);
	event.AddUDINT(val);

	return Add(event);
}


//-------------------------------------------------------------------------------------------------
UDINT rEventManager::Get(rEventArray &arr)
{
	Lock();
	
	memcpy((void*)&arr, (const void *)&Event, sizeof(rEventArray));
	
	Unlock();
	
	return 0;
}


//-------------------------------------------------------------------------------------------------
//
string rEventManager::GetDescr(rEvent &event)
{
	string  text   = "";
	string  result = "";

	// Получим шаблон описания события
	if(Texts.Get(event.GetEID(), text))
	{
		result = String_format("Event %u does not exist", event.GetEID());

		TRACEERROR(result.c_str());
		return result;
	}

	if(text.empty())
	{
		result = String_format("Event %u have not decription", event.GetEID());

		TRACEERROR(result.c_str());
		return result;
	}

	// Начнем подставлять данные в описание события
	for(UDINT ii = 0; ii < text.size(); ++ii)
	{
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
		result += ParseParameter(event, text.c_str() + ii + 1, ii);
	} // for

	return result;
}



string rEventManager::ParseParameter(rEvent &event, const char *str, UDINT &offset)
{
	static string  floatformat[3] = {"%.*f", "%.*e", "%.*E"};

	UDINT   number = 0;
	UDINT   prec   = 0xFFFFFFFF;
	UDINT   exp    = 0;
	UDINT   type   = 0;
	void   *data   = nullptr;

	offset += ParseNumber(str, number, prec, exp);
	data    = event.GetParamByID(number - 1, type); // Номера указываются с 1, а по факту с 0

	if(data == nullptr)
	{
		TRACEERROR(String_format("Event eid %i: unknow param %i", event.GetEID(), number).c_str());
		return "<?>";
	}

	// Если точность не была указана, то ищем предшествующие единицы измерений
	if(prec == 0xFFFFFFFF && (type == TYPE_LREAL || type == TYPE_REAL))
	{
		prec = PRECISION_DEFAUILT;

		// Получаем значение точности из предыдущей ед.измерения
		for(DINT jj = number - 2; jj >= 0; --jj)
		{
			UDINT  sidtype = 0;
			UDINT *sid     = (UDINT *)event.GetParamByID(jj, sidtype);

			// Нашли ед.измерения
			if(sidtype == TYPE_STRID && *sid < MAX_UNITS_COUNT)
			{
				prec = rPrecision::Instance().Get(*sid);
				break;
			}
		}
	}

	// форматируем результат
	switch(type)
	{
		case TYPE_UNDEF:
			TRACEERROR(String_format("Event eid %i, param %i: undefine type", event.GetEID(), number).c_str());
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
			const string *str = rTextManager::Instance().GetPtr(*(UDINT *)data);
			return (str == nullptr) ? String_format("<unknow sid %u>", *(UDINT *)data) : *str;
		}

		default:
			TRACEERROR(String_format("Event eid %i, param %i: unknow type", event.GetEID(), number).c_str());
			return "<unknow type>";
	}
}



//-------------------------------------------------------------------------------------------------
// Парсинг номеров параметров вида: $1, $2.3, $4.e5, $6.e
UDINT rEventManager::ParseNumber(const char *str, UDINT &num, UDINT &prec, UDINT &exp)
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
//
UDINT rEventManager::LoadText(const string& filename)
{
	if(tinyxml2::XML_SUCCESS != Texts.LoadSystem(filename))
	{
		TRACEERROR("Can't load system event. Error %i, line %i", Texts.ErrorID, Texts.ErrorLine);
		exit(0);
	}

	return 0;
}


//
UDINT rEventManager::SetCurLang(const string &lang)
{
	return Texts.SetCurLang(lang);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
void Event_ID(UDINT eid, UDINT id)
{
	rEvent event(eid);
	event.AddUDINT(id);
	
	rEventManager::instance().Add(event);
}


void Event_ID_UINT(UDINT eid, UDINT id, UINT val)
{
	rEvent event(eid);
	event.AddUDINT(id);
	event.AddUINT(val);
	
	rEventManager::instance().Add(event);
}


void Event_ID_UDINT(UDINT eid, UDINT id, UDINT val)
{
	rEvent event(eid);
	event.AddUDINT(id);
	event.AddUDINT(val);

	rEventManager::instance().Add(event);
}


void Event_ID_UINT_UINT(UDINT eid, UDINT id, UINT val1, UINT val2)
{
	rEvent event(eid);
	event.AddUDINT(id);
	event.AddUINT(val1);
	event.AddUINT(val2);
	
	rEventManager::instance().Add(event);
}
