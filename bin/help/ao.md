<p align='right'><a href='index.html'>[Оглавление]</a></p>

# ao
> 0.19.170.15f6f81c
## XML
````xml
<ao name="valid object name" description="string index" setup="text value | text value | ... | text value" mode="text value" >
	<io_link module="module index" channel="channel index"/> <!-- Optional -->
	<present><link alias="object's output"/></present>
	<limits><!-- Optional -->
		<limit name="present" setup="LOLO|LO|HI|HIHI"><!-- Optional -->
			<lolo>0</lolo>
			<lo>0</lo>
			<hi>0</hi>
			<hihi>0</hihi>
			<hysteresis>0</hysteresis>
		</limit>
	</limits>
	<unit>0<unit/>
	<scale>
		<min>0</min>
		<max>100</max>
	</scale>
</ao>
````

#### setup
* _OFF_  - Выключен и не обрабатывается

#### mode
* _PHIS_  - Используется физическое значение
* _KEYPAD_  - Переключение пользователем на ручной ввод

## Inputs
Input | Unit | Unit ID | Limits | Shadow | Comment
:-- |:--:|:--:|:--:|:--:|:--
present |  | 0 | LOLO, LO, HI, HIHI |  | Значение в инженерных единицах

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:-- |:--
present.value | lreal |  | 0 |  |   | Значение в инженерных единицах. Текущее значение
present.unit | strid |  | 512 | Yes |   | Значение в инженерных единицах. Единицы измерения
present.lolo | lreal |  | 0 |  | 0x00000200 | Значение в инженерных единицах. Значение аварийного минимума
present.lo | lreal |  | 0 |  | 0x00000200 | Значение в инженерных единицах. Значение предаварийного минимума
present.hi | lreal |  | 0 |  | 0x00000200 | Значение в инженерных единицах. Значение предаварийного максимума
present.hihi | lreal |  | 0 |  | 0x00000200 | Значение в инженерных единицах. Значение аварийного максимума
present.hysteresis | lreal |  | 0 |  | 0x00000200 | Значение в инженерных единицах. Значение гистерезиса
present.status | uint |  | 512 | Yes |   | Значение в инженерных единицах. Статус:<br/>0: Неопределен<br/>1: Недействительное значение<br/>2: Значение ниже аварийного минимума<br/>3: Значение ниже предаварийного минимума<br/>4: Значение в рабочем диапазоне<br/>5: Значение выше предаварийного максимума<br/>6: Значение выше аварийного максимума<br/>
present.setup | uint |  | 512 | Yes | 0x00000200 | Значение в инженерных единицах. Настройка:<br/>0x0001: Не выдавать сообщения<br/>0x0002: Выдавать сообщение аварийного минимума<br/>0x0004: Выдавать сообщение предаварийного минимума<br/>0x0008: Выдавать сообщение предаварийного максимума<br/>0x0010: Выдавать сообщение аварийного максимума<br/>
scales.low | lreal |  | 0 |  | 0x00000400 | Значение инженерного минимума
scales.high | lreal |  | 0 |  | 0x00000400 | Значение инженерного максимума
phvalue | uint |  | 0 | Yes |   | Значение ЦАП
setup | uint |  | 512 | Yes | 0x80000000 | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>
mode | uint |  | 512 |  | 0x00000800 | Режим:<br/>0: Используется физическое значение<br/>1: Переключение пользователем на ручной ввод<br/>
fault | udint |  | 512 | Yes |   | Флаг ошибки


<p align='right'><a href='index.html'>[Оглавление]</a></p>

