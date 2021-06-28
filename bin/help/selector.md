<p align='right'><a href='index.html'>[Оглавление]</a></p>

# selector
> 0.19.132.77cddc06
## XML
````xml
<selector name="valid object name" description="string index" setup="text value | text value | ... | text value" mode="text value" select="-1" >
	<inputs>
		<link alias="object's output"/>
		<link alias="object's output"/>
		<link alias="object's output"/> <!-- Optional -->
		<link alias="object's output"/> <!-- Optional -->
	</inputs>
	<faults> <!-- Optional -->
		<link alias="object's output"/>
		<link alias="object's output"/>
		<link alias="object's output"/> <!-- Optional -->
		<link alias="object's output"/> <!-- Optional -->
	</faults>
	<keypad>
		<unit>0<unit/>
		<value>0</value>
	</keypad>
</selector>
````

#### setup
* _OFF_  - Выключен и не обрабатывается
* _NOEVENT_  - Запретить выдачу сообщений

#### mode
* _NOCHANGE_  - При аварии не переходить на другой вход
* _ERROR_  - При аварии переходить на аварийное значение
* _PREV_  - При аварии переключить на предыдущий вход
* _NEXT_  - При аварии переключить на следующий вход

## Inputs
Input | Unit | Unit ID | Limits | Shadow | Comment
:-- |:--:|:--:|:--:|:--:|:--
input_1 |  | 0 | OFF |  | входное значение 1
input_1.fault |  | 512 | OFF | input_1 | флаг ошибки входа 1
input_2 |  | 0 | OFF |  | входное значение 2
input_2.fault |  | 512 | OFF | input_2 | флаг ошибки входа 2
input_3 |  | 0 | OFF |  | входное значение 3
input_3.fault |  | 512 | OFF | input_3 | флаг ошибки входа 3
input_4 |  | 0 | OFF |  | входное значение 4
input_4.fault |  | 512 | OFF | input_4 | флаг ошибки входа 4

## Outputs
Output | Unit | Unit ID | Limits | Comment
:-- |:--:|:--:|:--:|:--
output |  | 0 | OFF | выходное значение

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:-- |:--
input_1.value | lreal |  | 0 | Yes |   | входное значение 1. Текущее значение
input_1.unit | strid |  | 512 | Yes |   | входное значение 1. Единицы измерения
input_1.fault | lreal |  | 512 | Yes |   | флаг ошибки входа 1
input_2.value | lreal |  | 0 | Yes |   | входное значение 2. Текущее значение
input_2.unit | strid |  | 512 | Yes |   | входное значение 2. Единицы измерения
input_2.fault | lreal |  | 512 | Yes |   | флаг ошибки входа 2
input_3.value | lreal |  | 0 | Yes |   | входное значение 3. Текущее значение
input_3.unit | strid |  | 512 | Yes |   | входное значение 3. Единицы измерения
input_3.fault | lreal |  | 512 | Yes |   | флаг ошибки входа 3
input_4.value | lreal |  | 0 | Yes |   | входное значение 4. Текущее значение
input_4.unit | strid |  | 512 | Yes |   | входное значение 4. Единицы измерения
input_4.fault | lreal |  | 512 | Yes |   | флаг ошибки входа 4
output.value | lreal |  | 0 | Yes |   | выходное значение. Текущее значение
output.unit | strid |  | 512 | Yes |   | выходное значение. Единицы измерения
select | int |  | 512 |  | 0x00004000 | Выбор коммуцируемого входа
inputcount | uint |  | 512 | Yes |   | Количество подключенных входов
setup | uint |  | 512 | Yes | 0x80000000 | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0004: Запретить выдачу сообщений<br/>
mode | uint |  | 512 |  | 0x00004000 | Режим:<br/>1: При аварии не переходить на другой вход<br/>2: При аварии переходить на аварийное значение<br/>3: При аварии переключить на предыдущий вход<br/>4: При аварии переключить на следующий вход<br/>
fault | udint |  | 512 | Yes |   | Флаг ошибки
keypad.unit | strid |  | 512 | Yes |   | Значение ручного ввода. Единицы измерения
keypad.value | lreal |  | 0 |  | 0x00000800 | Значение ручного ввода


Количество входных значений не должно быть меньше чем количество флагов ошибки входа!

<p align='right'><a href='index.html'>[Оглавление]</a></p>

