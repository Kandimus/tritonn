<p align='right'><a href='index.html'>[Оглавление]</a></p>

# ai6p
> 0.19.170.15f6f81c
## XML
````xml
<module type="ai6p" name="valid object name" description="string index" >
	<channel number="0" setup="AI setup flags"mode="AI mode flag" />
	<channel number="1" setup="AI setup flags"mode="AI mode flag" />
	<channel number="2" setup="AI setup flags"mode="AI mode flag" />
	<channel number="3" setup="AI setup flags"mode="AI mode flag" />
	<channel number="4" setup="AI setup flags"mode="AI mode flag" />
	<channel number="5" setup="AI setup flags"mode="AI mode flag" />
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


#### AI mode
* _mA_0_20_  - 0..20мА
* _mA_4_20_  - 4..40мА
* _V_m10_10_  - -10..+10V
* _V_0_10_  - 0..+10V

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:-- |:--
ai6p.type | uint |  | 512 | Yes |   | Тип модуля:<br>0: Модуль не определен<br/>1: Базовый модуль ЦПУ<br/>2: Модуль AI6a<br/>3: Модуль AI6p<br/>4: Модуль FI4<br/>5: Модуль DI8DO8<br/>6: Модуль DI16<br/>7: Модуль DO16<br/>8: Модуль CRM<br/>9: Модуль AO4<br/>
ai6p.node | uint |  | 512 | Yes |   | Уникальный номер
ai6p.vendor | udint |  | 512 | Yes |   | Код производителя
ai6p.productcode | udint |  | 512 | Yes |   | Код устройства
ai6p.revision | udint |  | 512 | Yes |   | Версия сетевого драйвера
ai6p.serialnumber | udint |  | 512 | Yes |   | Серийный номер нижнего уровня
ai6p.can | uint |  | 512 | Yes |   | Состояние обмена данными
ai6p.firmware | uint |  | 512 | Yes |   | Версия ПО модуля
ai6p.hardware | uint |  | 512 | Yes |   | Версия модуля
ai6p.temperature | real | °C | 16 | Yes |   | Температура модуля в гр.С.
ai6p.ch_00.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6p.ch_00.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6p.ch_00.adc | uint |  | 512 | Yes |   | Текущий код АЦП
ai6p.ch_00.current | real |  | 512 | Yes |   | Текущее значение тока/напряжения
ai6p.ch_00.state | usint |  | 512 | Yes |   | Статус канала
ai6p.ch_00.mode | usint |  | 512 |  | 0x00000400 | Режим канала:<br/>0: 0..20мА<br/>1: 4..40мА<br/>2: -10..+10V<br/>3: 0..+10V<br/>
ai6p.ch_00.correct | uint |  | 512 |  | 0x00000400 | Калибровка канала:<br/>0: нет действий<br/>1: Корректировка 1 точки (4 мА)<br/>2: Корректировка 2 точки (12 мА)<br/>3: Корректировка 3 точки (20 мА)<br/>
ai6p.ch_00.simulate.max | uint |  | 512 |  |   | Код АЦП. Максимум симулированного значения
ai6p.ch_00.simulate.min | uint |  | 512 |  |   | Код АЦП. Минимум симулированного значения
ai6p.ch_00.simulate.value | uint |  | 512 |  |   | Код АЦП. Симулированное значение
ai6p.ch_00.simulate.speed | int |  | 512 |  |   | Код АЦП. Скорость изменения симулированного значения
ai6p.ch_01.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6p.ch_01.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6p.ch_01.adc | uint |  | 512 | Yes |   | Текущий код АЦП
ai6p.ch_01.current | real |  | 512 | Yes |   | Текущее значение тока/напряжения
ai6p.ch_01.state | usint |  | 512 | Yes |   | Статус канала
ai6p.ch_01.mode | usint |  | 512 |  | 0x00000400 | Режим канала:<br/>0: 0..20мА<br/>1: 4..40мА<br/>2: -10..+10V<br/>3: 0..+10V<br/>
ai6p.ch_01.correct | uint |  | 512 |  | 0x00000400 | Калибровка канала:<br/>0: нет действий<br/>1: Корректировка 1 точки (4 мА)<br/>2: Корректировка 2 точки (12 мА)<br/>3: Корректировка 3 точки (20 мА)<br/>
ai6p.ch_01.simulate.max | uint |  | 512 |  |   | Код АЦП. Максимум симулированного значения
ai6p.ch_01.simulate.min | uint |  | 512 |  |   | Код АЦП. Минимум симулированного значения
ai6p.ch_01.simulate.value | uint |  | 512 |  |   | Код АЦП. Симулированное значение
ai6p.ch_01.simulate.speed | int |  | 512 |  |   | Код АЦП. Скорость изменения симулированного значения
ai6p.ch_02.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6p.ch_02.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6p.ch_02.adc | uint |  | 512 | Yes |   | Текущий код АЦП
ai6p.ch_02.current | real |  | 512 | Yes |   | Текущее значение тока/напряжения
ai6p.ch_02.state | usint |  | 512 | Yes |   | Статус канала
ai6p.ch_02.mode | usint |  | 512 |  | 0x00000400 | Режим канала:<br/>0: 0..20мА<br/>1: 4..40мА<br/>2: -10..+10V<br/>3: 0..+10V<br/>
ai6p.ch_02.correct | uint |  | 512 |  | 0x00000400 | Калибровка канала:<br/>0: нет действий<br/>1: Корректировка 1 точки (4 мА)<br/>2: Корректировка 2 точки (12 мА)<br/>3: Корректировка 3 точки (20 мА)<br/>
ai6p.ch_02.simulate.max | uint |  | 512 |  |   | Код АЦП. Максимум симулированного значения
ai6p.ch_02.simulate.min | uint |  | 512 |  |   | Код АЦП. Минимум симулированного значения
ai6p.ch_02.simulate.value | uint |  | 512 |  |   | Код АЦП. Симулированное значение
ai6p.ch_02.simulate.speed | int |  | 512 |  |   | Код АЦП. Скорость изменения симулированного значения
ai6p.ch_03.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6p.ch_03.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6p.ch_03.adc | uint |  | 512 | Yes |   | Текущий код АЦП
ai6p.ch_03.current | real |  | 512 | Yes |   | Текущее значение тока/напряжения
ai6p.ch_03.state | usint |  | 512 | Yes |   | Статус канала
ai6p.ch_03.mode | usint |  | 512 |  | 0x00000400 | Режим канала:<br/>0: 0..20мА<br/>1: 4..40мА<br/>2: -10..+10V<br/>3: 0..+10V<br/>
ai6p.ch_03.correct | uint |  | 512 |  | 0x00000400 | Калибровка канала:<br/>0: нет действий<br/>1: Корректировка 1 точки (4 мА)<br/>2: Корректировка 2 точки (12 мА)<br/>3: Корректировка 3 точки (20 мА)<br/>
ai6p.ch_03.simulate.max | uint |  | 512 |  |   | Код АЦП. Максимум симулированного значения
ai6p.ch_03.simulate.min | uint |  | 512 |  |   | Код АЦП. Минимум симулированного значения
ai6p.ch_03.simulate.value | uint |  | 512 |  |   | Код АЦП. Симулированное значение
ai6p.ch_03.simulate.speed | int |  | 512 |  |   | Код АЦП. Скорость изменения симулированного значения
ai6p.ch_04.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6p.ch_04.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6p.ch_04.adc | uint |  | 512 | Yes |   | Текущий код АЦП
ai6p.ch_04.current | real |  | 512 | Yes |   | Текущее значение тока/напряжения
ai6p.ch_04.state | usint |  | 512 | Yes |   | Статус канала
ai6p.ch_04.mode | usint |  | 512 |  | 0x00000400 | Режим канала:<br/>0: 0..20мА<br/>1: 4..40мА<br/>2: -10..+10V<br/>3: 0..+10V<br/>
ai6p.ch_04.correct | uint |  | 512 |  | 0x00000400 | Калибровка канала:<br/>0: нет действий<br/>1: Корректировка 1 точки (4 мА)<br/>2: Корректировка 2 точки (12 мА)<br/>3: Корректировка 3 точки (20 мА)<br/>
ai6p.ch_04.simulate.max | uint |  | 512 |  |   | Код АЦП. Максимум симулированного значения
ai6p.ch_04.simulate.min | uint |  | 512 |  |   | Код АЦП. Минимум симулированного значения
ai6p.ch_04.simulate.value | uint |  | 512 |  |   | Код АЦП. Симулированное значение
ai6p.ch_04.simulate.speed | int |  | 512 |  |   | Код АЦП. Скорость изменения симулированного значения
ai6p.ch_05.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6p.ch_05.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6p.ch_05.adc | uint |  | 512 | Yes |   | Текущий код АЦП
ai6p.ch_05.current | real |  | 512 | Yes |   | Текущее значение тока/напряжения
ai6p.ch_05.state | usint |  | 512 | Yes |   | Статус канала
ai6p.ch_05.mode | usint |  | 512 |  | 0x00000400 | Режим канала:<br/>0: 0..20мА<br/>1: 4..40мА<br/>2: -10..+10V<br/>3: 0..+10V<br/>
ai6p.ch_05.correct | uint |  | 512 |  | 0x00000400 | Калибровка канала:<br/>0: нет действий<br/>1: Корректировка 1 точки (4 мА)<br/>2: Корректировка 2 точки (12 мА)<br/>3: Корректировка 3 точки (20 мА)<br/>
ai6p.ch_05.simulate.max | uint |  | 512 |  |   | Код АЦП. Максимум симулированного значения
ai6p.ch_05.simulate.min | uint |  | 512 |  |   | Код АЦП. Минимум симулированного значения
ai6p.ch_05.simulate.value | uint |  | 512 |  |   | Код АЦП. Симулированное значение
ai6p.ch_05.simulate.speed | int |  | 512 |  |   | Код АЦП. Скорость изменения симулированного значения


[^simtype]: **Тип симуляции:**<br/>0: Симуляция отключена<br/>1: Константное значение<br/>2: Линейная функция<br/>3: Синусообразная функция<br/>4: Случайное значение<br/></br>

<p align='right'><a href='index.html'>[Оглавление]</a></p>

