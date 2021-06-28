<p align='right'><a href='index.html'>[Оглавление]</a></p>

# ai6
> 0.19.132.77cddc06
## XML
````xml
<module name="ai6" description="string index" >
	<channel number="0" setup="AI setup flags" />
	<channel number="1" setup="AI setup flags" />
	<channel number="2" setup="AI setup flags" />
	<channel number="3" setup="AI setup flags" />
	<channel number="4" setup="AI setup flags" />
	<channel number="5" setup="AI setup flags" />
</module>
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
ai6.type | uint |  | 512 | Yes |   | Нет данных
ai6.node | uint |  | 512 | Yes |   | Нет данных
ai6.vendor | udint |  | 512 | Yes |   | Нет данных
ai6.productcode | udint |  | 512 | Yes |   | Нет данных
ai6.revision | udint |  | 512 | Yes |   | Нет данных
ai6.serialnumber | udint |  | 512 | Yes |   | Нет данных
ai6.temperature | real |  | 512 | Yes |   | Нет данных
ai6.can | uint |  | 512 | Yes |   | Нет данных
ai6.firmware | uint |  | 512 | Yes |   | Нет данных
ai6.hardware | uint |  | 512 | Yes |   | Нет данных
ai6.ch_00.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6.ch_00.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6.ch_00.adc | uint |  | 512 | Yes |   | Текущий код АЦП
ai6.ch_00.current | real |  | 512 | Yes |   | Текущее значение тока/напряжения
ai6.ch_00.state | usint |  | 512 | Yes |   | Статус канала
ai6.ch_00.type | usint |  | 512 |  |   | Тип канала:<br/>0: 0..20мА<br/>1: 4..40мА<br/>2: -10..+10V<br/>3: 0..+10V<br/>
ai6.ch_00.simulate.max | uint |  | 512 |  |   | Код АЦП. Максимум симулированного значения
ai6.ch_00.simulate.min | uint |  | 512 |  |   | Код АЦП. Минимум симулированного значения
ai6.ch_00.simulate.value | uint |  | 512 |  |   | Код АЦП. Симулированное значение
ai6.ch_00.simulate.speed | int |  | 512 |  |   | Код АЦП. Скорость изменения симулированного значения
ai6.ch_01.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6.ch_01.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6.ch_01.adc | uint |  | 512 | Yes |   | Текущий код АЦП
ai6.ch_01.current | real |  | 512 | Yes |   | Текущее значение тока/напряжения
ai6.ch_01.state | usint |  | 512 | Yes |   | Статус канала
ai6.ch_01.type | usint |  | 512 |  |   | Тип канала:<br/>0: 0..20мА<br/>1: 4..40мА<br/>2: -10..+10V<br/>3: 0..+10V<br/>
ai6.ch_01.simulate.max | uint |  | 512 |  |   | Код АЦП. Максимум симулированного значения
ai6.ch_01.simulate.min | uint |  | 512 |  |   | Код АЦП. Минимум симулированного значения
ai6.ch_01.simulate.value | uint |  | 512 |  |   | Код АЦП. Симулированное значение
ai6.ch_01.simulate.speed | int |  | 512 |  |   | Код АЦП. Скорость изменения симулированного значения
ai6.ch_02.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6.ch_02.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6.ch_02.adc | uint |  | 512 | Yes |   | Текущий код АЦП
ai6.ch_02.current | real |  | 512 | Yes |   | Текущее значение тока/напряжения
ai6.ch_02.state | usint |  | 512 | Yes |   | Статус канала
ai6.ch_02.type | usint |  | 512 |  |   | Тип канала:<br/>0: 0..20мА<br/>1: 4..40мА<br/>2: -10..+10V<br/>3: 0..+10V<br/>
ai6.ch_02.simulate.max | uint |  | 512 |  |   | Код АЦП. Максимум симулированного значения
ai6.ch_02.simulate.min | uint |  | 512 |  |   | Код АЦП. Минимум симулированного значения
ai6.ch_02.simulate.value | uint |  | 512 |  |   | Код АЦП. Симулированное значение
ai6.ch_02.simulate.speed | int |  | 512 |  |   | Код АЦП. Скорость изменения симулированного значения
ai6.ch_03.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6.ch_03.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6.ch_03.adc | uint |  | 512 | Yes |   | Текущий код АЦП
ai6.ch_03.current | real |  | 512 | Yes |   | Текущее значение тока/напряжения
ai6.ch_03.state | usint |  | 512 | Yes |   | Статус канала
ai6.ch_03.type | usint |  | 512 |  |   | Тип канала:<br/>0: 0..20мА<br/>1: 4..40мА<br/>2: -10..+10V<br/>3: 0..+10V<br/>
ai6.ch_03.simulate.max | uint |  | 512 |  |   | Код АЦП. Максимум симулированного значения
ai6.ch_03.simulate.min | uint |  | 512 |  |   | Код АЦП. Минимум симулированного значения
ai6.ch_03.simulate.value | uint |  | 512 |  |   | Код АЦП. Симулированное значение
ai6.ch_03.simulate.speed | int |  | 512 |  |   | Код АЦП. Скорость изменения симулированного значения
ai6.ch_04.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6.ch_04.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6.ch_04.adc | uint |  | 512 | Yes |   | Текущий код АЦП
ai6.ch_04.current | real |  | 512 | Yes |   | Текущее значение тока/напряжения
ai6.ch_04.state | usint |  | 512 | Yes |   | Статус канала
ai6.ch_04.type | usint |  | 512 |  |   | Тип канала:<br/>0: 0..20мА<br/>1: 4..40мА<br/>2: -10..+10V<br/>3: 0..+10V<br/>
ai6.ch_04.simulate.max | uint |  | 512 |  |   | Код АЦП. Максимум симулированного значения
ai6.ch_04.simulate.min | uint |  | 512 |  |   | Код АЦП. Минимум симулированного значения
ai6.ch_04.simulate.value | uint |  | 512 |  |   | Код АЦП. Симулированное значение
ai6.ch_04.simulate.speed | int |  | 512 |  |   | Код АЦП. Скорость изменения симулированного значения
ai6.ch_05.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6.ch_05.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6.ch_05.adc | uint |  | 512 | Yes |   | Текущий код АЦП
ai6.ch_05.current | real |  | 512 | Yes |   | Текущее значение тока/напряжения
ai6.ch_05.state | usint |  | 512 | Yes |   | Статус канала
ai6.ch_05.type | usint |  | 512 |  |   | Тип канала:<br/>0: 0..20мА<br/>1: 4..40мА<br/>2: -10..+10V<br/>3: 0..+10V<br/>
ai6.ch_05.simulate.max | uint |  | 512 |  |   | Код АЦП. Максимум симулированного значения
ai6.ch_05.simulate.min | uint |  | 512 |  |   | Код АЦП. Минимум симулированного значения
ai6.ch_05.simulate.value | uint |  | 512 |  |   | Код АЦП. Симулированное значение
ai6.ch_05.simulate.speed | int |  | 512 |  |   | Код АЦП. Скорость изменения симулированного значения


[^simtype]: **Тип симуляции:**<br/>0: Симуляция отключена<br/>1: Константное значение<br/>2: Линейная функция<br/>3: Синусообразная функция<br/>4: Случайное значение<br/></br>

<p align='right'><a href='index.html'>[Оглавление]</a></p>

