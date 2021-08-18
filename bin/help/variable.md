<p align='right'><a href='index.html'>[Оглавление]</a></p>

# variable
> 0.19.165.ba8170a6
## XML
````xml
<variable name="valid object name" description="string index" setup="text value | text value | ... | text value" >
	<value><link alias="object's output"/></value>
	<limits><!-- Optional -->
		<limit name="value" setup="LOLO|LO|HI|HIHI"><!-- Optional -->
			<lolo>0</lolo>
			<lo>0</lo>
			<hi>0</hi>
			<hihi>0</hihi>
			<hysteresis>0</hysteresis>
		</limit>
	</limits>
	<default>0</default>
	<unit>0<unit/>
</variable>
````

#### setup
* _CONST_  - Установить как константу
* _LINK_  - Запретить изменение переменной. Значение будет получено со входа

## Inputs
Input | Unit | Unit ID | Limits | Shadow | Comment
:-- |:--:|:--:|:--:|:--:|:--
value |  | 0 | LOLO, LO, HI, HIHI |  | переменная

## Outputs
Output | Unit | Unit ID | Limits | Comment
:-- |:--:|:--:|:--:|:--
value |  | 0 | LOLO, LO, HI, HIHI | переменная

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:-- |:--
value | lreal |  | 0 |  |   | переменная. Текущее значение
unit | strid |  | 512 | Yes |   | переменная. Единицы измерения
lolo | lreal |  | 0 |  | 0x00000200 | переменная. Значение аварийного минимума
lo | lreal |  | 0 |  | 0x00000200 | переменная. Значение предаварийного минимума
hi | lreal |  | 0 |  | 0x00000200 | переменная. Значение предаварийного максимума
hihi | lreal |  | 0 |  | 0x00000200 | переменная. Значение аварийного максимума
hysteresis | lreal |  | 0 |  | 0x00000200 | переменная. Значение гистерезиса
status | uint |  | 512 | Yes |   | переменная. Статус:<br/>0: Неопределен<br/>1: Недействительное значение<br/>2: Значение ниже аварийного минимума<br/>3: Значение ниже предаварийного минимума<br/>4: Значение в рабочем диапазоне<br/>5: Значение выше предаварийного максимума<br/>6: Значение выше аварийного максимума<br/>
setup | uint |  | 512 | Yes | 0x00000200 | переменная. Настройка:<br/>0x0001: Не выдавать сообщения<br/>0x0002: Выдавать сообщение аварийного минимума<br/>0x0004: Выдавать сообщение предаварийного минимума<br/>0x0008: Выдавать сообщение предаварийного максимума<br/>0x0010: Выдавать сообщение аварийного максимума<br/>


Если установлен флаг LINK, то в переменную value запись будет не возможна.

<p align='right'><a href='index.html'>[Оглавление]</a></p>

