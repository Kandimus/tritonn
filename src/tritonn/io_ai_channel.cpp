//=================================================================================================
//===
//=== io_ai_channel.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс канала аналового входного модуля AI (CAN)
//===
//=================================================================================================

#include <math.h>
#include "io_ai_channel.h"


UDINT rIOAIChannel::simulate()
{
	switch(m_simType) {
		case SimType::None:
			return TRITONN_RESULT_OK;

		case SimType::Const: {
			m_ADC = m_simValue;
			return TRITONN_RESULT_OK;
		}

		case SimType::Linear: {
			DINT tmp = m_simValue + m_simSpeed;
			if(tmp >= m_simMax) {
				m_simValue = m_simMax;
				m_ADC      = m_simValue;
				m_simSpeed = -m_simSpeed;
			} else if(tmp <= m_simMin) {
				m_simValue = m_simMin;
				m_ADC      = m_simValue;
				m_simSpeed = -m_simSpeed;
			} else {
				m_simValue += m_simSpeed;
				m_ADC      = m_simValue;
			}
			return TRITONN_RESULT_OK;
		}

		case SimType::Sinus: {
			m_simValue += m_simSpeed;
			if(m_simValue >= 360) {
				m_simValue -= 360;
			}
			LREAL tmp = (sin(static_cast<LREAL>(m_simValue) * 0.017453293) + 1.0) / 2.0;
			tmp   = m_simMin + static_cast<LREAL>(m_simMax - m_simMin) * tmp;
			m_ADC = static_cast<UINT>(tmp);
			return TRITONN_RESULT_OK;
		}

		case SimType::Random: {
			LREAL tmp = m_simMin + static_cast<LREAL>(m_simMax - m_simMin) * (rand() / static_cast<LREAL>(RAND_MAX));
			m_ADC = static_cast<UINT>(tmp);
			return TRITONN_RESULT_OK;
		}
	}

	return TRITONN_RESULT_OK;
}

