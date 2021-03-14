<p align='right'><a href='index.html'>[Оглавление]</a></p>

# rvar
> 0.19.18.6bb32f40
## XML
````xml
<rvar name="valid object name" descr="string index" setup="text value | text value | ... | text value" >
	<value><link alias="object's output"/></value>
	<limits> <!-- Optional -->
		<limit name="value" setup="LOLO|LO|HI|HIHI">
			<lolo>0</lolo>
			<lo>0</lo>
			<hi>0</hi>
			<hihi>0</hihi>
		</limit>
	</limits>
	<value>0<value/>
	<unit>0<unit/>
</rvar>
````

#### setup
* _CONST_  - Установить как константу

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
value | LREAL |  | 0 |  |   | переменная. Текущее значение
unit | STRID |  | 512 | Yes |   | переменная. Единицы измерения
lolo | LREAL |  | 0 |  | 0x00000200 | переменная. Значение аварийного минимума
lo | LREAL |  | 0 |  | 0x00000200 | переменная. Значение предаварийного минимума
hi | LREAL |  | 0 |  | 0x00000200 | переменная. Значение предаварийного максимума
hihi | LREAL |  | 0 |  | 0x00000200 | переменная. Значение аварийного максимума
hysteresis | LREAL |  | 0 |  | 0x00000200 | переменная. Значение гистерезиса
status | UINT |  | 512 | Yes |   | переменная. Статус:<br/>0: Неопределен<br/>1: Недействительное значение<br/>2: Значение ниже аварийного минимума<br/>3: Значение ниже предаварийного минимума<br/>4: Значение в рабочем диапазоне<br/>5: Значение выше предаварийного максимума<br/>6: Значение выше аварийного максимума<br/>
setup | UINT |  | 512 | Yes | 0x00000200 | переменная. Настройка:<br/>0x0001: Не выдавать сообщения<br/>0x0002: Выдавать сообщение аварийного минимума<br/>0x0004: Выдавать сообщение предаварийного минимума<br/>0x0008: Выдавать сообщение предаварийного максимума<br/>0x0010: Выдавать сообщение аварийного максимума<br/>



[^mutable]: Если объект не привязан к модулю ввода-вывода, то данная переменная будет записываемой.


<p align='right'><a href='index.html'>[Оглавление]</a></p>

