<p align='right'><a href='index.html'>[Оглавление]</a></p>

# ai6
> 0.19.24.063224f6
## XML
````xml
<ai6 name="valid object name" descr="string index" >
	<channel number="0" setup="AI setup flags" />
	<channel number="1" setup="AI setup flags" />
	<channel number="2" setup="AI setup flags" />
	<channel number="3" setup="AI setup flags" />
	<channel number="4" setup="AI setup flags" />
	<channel number="5" setup="AI setup flags" />
</ai6>
````

## Channels
Number | Type | Comment
:-- |:--:|:--
0 | AI | 
1 | AI | 
2 | AI | 
3 | AI | 
4 | AI | 
5 | AI | 


#### AI setup
* _OFF_  - Выключен и не обрабатывается
* _AVERAGE_  - Усреднение значения

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:-- |:--
ai6.type | UINT |  | 512 | Yes |   | Нет данных
ai6.node | UINT |  | 512 | Yes |   | Нет данных
ai6.vendor | UDINT |  | 512 | Yes |   | Нет данных
ai6.productcode | UDINT |  | 512 | Yes |   | Нет данных
ai6.revision | UDINT |  | 512 | Yes |   | Нет данных
ai6.serialnumber | UDINT |  | 512 | Yes |   | Нет данных
ai6.temperature | REAL |  | 512 | Yes |   | Нет данных
ai6.can | UINT |  | 512 | Yes |   | Нет данных
ai6.firmware | UINT |  | 512 | Yes |   | Нет данных
ai6.hardware | UINT |  | 512 | Yes |   | Нет данных
ai6.ch_00.simulate.type | USINT |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6.ch_00.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6.ch_00.adc | UINT |  | 512 | Yes |   | Текущий код АЦП
ai6.ch_00.current | REAL |  | 512 | Yes |   | Текущее значение тока/напряжения
ai6.ch_00.state | USINT |  | 512 | Yes |   | Статус канала
ai6.ch_00.type | USINT |  | 512 |  |   | Тип канала:<br/>1: Выключен и не обрабатывается<br/>32: Усреднение значения<br/>
ai6.ch_00.simulate.max | UINT |  | 512 |  |   | Максимум симулированного значения
ai6.ch_00.simulate.min | UINT |  | 512 |  |   | Минимум симулированного значения
ai6.ch_00.simulate.value | UINT |  | 512 |  |   | Симулированное значение
ai6.ch_00.simulate.speed | INT |  | 512 |  |   | Скорость изменения симулированного значения
ai6.ch_01.simulate.type | USINT |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6.ch_01.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6.ch_01.adc | UINT |  | 512 | Yes |   | Текущий код АЦП
ai6.ch_01.current | REAL |  | 512 | Yes |   | Текущее значение тока/напряжения
ai6.ch_01.state | USINT |  | 512 | Yes |   | Статус канала
ai6.ch_01.type | USINT |  | 512 |  |   | Тип канала:<br/>1: Выключен и не обрабатывается<br/>32: Усреднение значения<br/>
ai6.ch_01.simulate.max | UINT |  | 512 |  |   | Максимум симулированного значения
ai6.ch_01.simulate.min | UINT |  | 512 |  |   | Минимум симулированного значения
ai6.ch_01.simulate.value | UINT |  | 512 |  |   | Симулированное значение
ai6.ch_01.simulate.speed | INT |  | 512 |  |   | Скорость изменения симулированного значения
ai6.ch_02.simulate.type | USINT |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6.ch_02.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6.ch_02.adc | UINT |  | 512 | Yes |   | Текущий код АЦП
ai6.ch_02.current | REAL |  | 512 | Yes |   | Текущее значение тока/напряжения
ai6.ch_02.state | USINT |  | 512 | Yes |   | Статус канала
ai6.ch_02.type | USINT |  | 512 |  |   | Тип канала:<br/>1: Выключен и не обрабатывается<br/>32: Усреднение значения<br/>
ai6.ch_02.simulate.max | UINT |  | 512 |  |   | Максимум симулированного значения
ai6.ch_02.simulate.min | UINT |  | 512 |  |   | Минимум симулированного значения
ai6.ch_02.simulate.value | UINT |  | 512 |  |   | Симулированное значение
ai6.ch_02.simulate.speed | INT |  | 512 |  |   | Скорость изменения симулированного значения
ai6.ch_03.simulate.type | USINT |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6.ch_03.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6.ch_03.adc | UINT |  | 512 | Yes |   | Текущий код АЦП
ai6.ch_03.current | REAL |  | 512 | Yes |   | Текущее значение тока/напряжения
ai6.ch_03.state | USINT |  | 512 | Yes |   | Статус канала
ai6.ch_03.type | USINT |  | 512 |  |   | Тип канала:<br/>1: Выключен и не обрабатывается<br/>32: Усреднение значения<br/>
ai6.ch_03.simulate.max | UINT |  | 512 |  |   | Максимум симулированного значения
ai6.ch_03.simulate.min | UINT |  | 512 |  |   | Минимум симулированного значения
ai6.ch_03.simulate.value | UINT |  | 512 |  |   | Симулированное значение
ai6.ch_03.simulate.speed | INT |  | 512 |  |   | Скорость изменения симулированного значения
ai6.ch_04.simulate.type | USINT |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6.ch_04.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6.ch_04.adc | UINT |  | 512 | Yes |   | Текущий код АЦП
ai6.ch_04.current | REAL |  | 512 | Yes |   | Текущее значение тока/напряжения
ai6.ch_04.state | USINT |  | 512 | Yes |   | Статус канала
ai6.ch_04.type | USINT |  | 512 |  |   | Тип канала:<br/>1: Выключен и не обрабатывается<br/>32: Усреднение значения<br/>
ai6.ch_04.simulate.max | UINT |  | 512 |  |   | Максимум симулированного значения
ai6.ch_04.simulate.min | UINT |  | 512 |  |   | Минимум симулированного значения
ai6.ch_04.simulate.value | UINT |  | 512 |  |   | Симулированное значение
ai6.ch_04.simulate.speed | INT |  | 512 |  |   | Скорость изменения симулированного значения
ai6.ch_05.simulate.type | USINT |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6.ch_05.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6.ch_05.adc | UINT |  | 512 | Yes |   | Текущий код АЦП
ai6.ch_05.current | REAL |  | 512 | Yes |   | Текущее значение тока/напряжения
ai6.ch_05.state | USINT |  | 512 | Yes |   | Статус канала
ai6.ch_05.type | USINT |  | 512 |  |   | Тип канала:<br/>1: Выключен и не обрабатывается<br/>32: Усреднение значения<br/>
ai6.ch_05.simulate.max | UINT |  | 512 |  |   | Максимум симулированного значения
ai6.ch_05.simulate.min | UINT |  | 512 |  |   | Минимум симулированного значения
ai6.ch_05.simulate.value | UINT |  | 512 |  |   | Симулированное значение
ai6.ch_05.simulate.speed | INT |  | 512 |  |   | Скорость изменения симулированного значения


[^simtype]: **Тип симуляции:**<br/>0: Симуляция отключена<br/>1: Константное значение<br/>2: Линейная функция<br/>3: Синусообразная функция<br/>4: Случайное значение<br/></br>
[^mutable]: Если объект не привязан к модулю ввода-вывода, то данная переменная будет записываемой.


<p align='right'><a href='index.html'>[Оглавление]</a></p>

