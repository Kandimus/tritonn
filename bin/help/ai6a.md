<p align='right'><a href='index.html'>[Оглавление]</a></p>

# ai6a
> 0.19.170.15f6f81c
## XML
````xml
<module type="ai6a" name="valid object name" description="string index" >
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

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:-- |:--
ai6a.type | uint |  | 512 | Yes |   | Тип модуля:<br>0: Модуль не определен<br/>1: Базовый модуль ЦПУ<br/>2: Модуль AI6a<br/>3: Модуль AI6p<br/>4: Модуль FI4<br/>5: Модуль DI8DO8<br/>6: Модуль DI16<br/>7: Модуль DO16<br/>8: Модуль CRM<br/>9: Модуль AO4<br/>
ai6a.node | uint |  | 512 | Yes |   | Уникальный номер
ai6a.vendor | udint |  | 512 | Yes |   | Код производителя
ai6a.productcode | udint |  | 512 | Yes |   | Код устройства
ai6a.revision | udint |  | 512 | Yes |   | Версия сетевого драйвера
ai6a.serialnumber | udint |  | 512 | Yes |   | Серийный номер нижнего уровня
ai6a.can | uint |  | 512 | Yes |   | Состояние обмена данными
ai6a.firmware | uint |  | 512 | Yes |   | Версия ПО модуля
ai6a.hardware | uint |  | 512 | Yes |   | Версия модуля
ai6a.temperature | real | °C | 16 | Yes |   | Температура модуля в гр.С.
ai6a.correct | uint |  | 512 |  | 0x00000400 | Калибровка каналов:<br/>0: Нет действий<br/>1: Сохранить калибровки для всех каналов<br/>2: Сбросить не сохранненые калибровки для всех каналов<br/>
ai6a.ch_00.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6a.ch_00.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6a.ch_00.adc | uint |  | 512 | Yes |   | Текущий код АЦП
ai6a.ch_00.current | real |  | 512 | Yes |   | Текущее значение тока
ai6a.ch_00.state | usint |  | 512 | Yes |   | Статус канала
ai6a.ch_00.mode | usint |  | 512 |  | 0x00000400 | Режим канала:<br/>0: 0..20мА<br/>1: 4..40мА<br/>
ai6a.ch_00.correct | uint |  | 512 |  | 0x00000400 | Калибровка канала:<br/>0: нет действий<br/>1: Корректировка 1 точки (4 мА)<br/>2: Корректировка 2 точки (12 мА)<br/>3: Корректировка 3 точки (20 мА)<br/>
ai6a.ch_00.simulate.max | uint |  | 512 |  |   | Код АЦП. Максимум симулированного значения
ai6a.ch_00.simulate.min | uint |  | 512 |  |   | Код АЦП. Минимум симулированного значения
ai6a.ch_00.simulate.value | uint |  | 512 |  |   | Код АЦП. Симулированное значение
ai6a.ch_00.simulate.speed | int |  | 512 |  |   | Код АЦП. Скорость изменения симулированного значения
ai6a.ch_01.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6a.ch_01.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6a.ch_01.adc | uint |  | 512 | Yes |   | Текущий код АЦП
ai6a.ch_01.current | real |  | 512 | Yes |   | Текущее значение тока
ai6a.ch_01.state | usint |  | 512 | Yes |   | Статус канала
ai6a.ch_01.mode | usint |  | 512 |  | 0x00000400 | Режим канала:<br/>0: 0..20мА<br/>1: 4..40мА<br/>
ai6a.ch_01.correct | uint |  | 512 |  | 0x00000400 | Калибровка канала:<br/>0: нет действий<br/>1: Корректировка 1 точки (4 мА)<br/>2: Корректировка 2 точки (12 мА)<br/>3: Корректировка 3 точки (20 мА)<br/>
ai6a.ch_01.simulate.max | uint |  | 512 |  |   | Код АЦП. Максимум симулированного значения
ai6a.ch_01.simulate.min | uint |  | 512 |  |   | Код АЦП. Минимум симулированного значения
ai6a.ch_01.simulate.value | uint |  | 512 |  |   | Код АЦП. Симулированное значение
ai6a.ch_01.simulate.speed | int |  | 512 |  |   | Код АЦП. Скорость изменения симулированного значения
ai6a.ch_02.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6a.ch_02.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6a.ch_02.adc | uint |  | 512 | Yes |   | Текущий код АЦП
ai6a.ch_02.current | real |  | 512 | Yes |   | Текущее значение тока
ai6a.ch_02.state | usint |  | 512 | Yes |   | Статус канала
ai6a.ch_02.mode | usint |  | 512 |  | 0x00000400 | Режим канала:<br/>0: 0..20мА<br/>1: 4..40мА<br/>
ai6a.ch_02.correct | uint |  | 512 |  | 0x00000400 | Калибровка канала:<br/>0: нет действий<br/>1: Корректировка 1 точки (4 мА)<br/>2: Корректировка 2 точки (12 мА)<br/>3: Корректировка 3 точки (20 мА)<br/>
ai6a.ch_02.simulate.max | uint |  | 512 |  |   | Код АЦП. Максимум симулированного значения
ai6a.ch_02.simulate.min | uint |  | 512 |  |   | Код АЦП. Минимум симулированного значения
ai6a.ch_02.simulate.value | uint |  | 512 |  |   | Код АЦП. Симулированное значение
ai6a.ch_02.simulate.speed | int |  | 512 |  |   | Код АЦП. Скорость изменения симулированного значения
ai6a.ch_03.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6a.ch_03.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6a.ch_03.adc | uint |  | 512 | Yes |   | Текущий код АЦП
ai6a.ch_03.current | real |  | 512 | Yes |   | Текущее значение тока
ai6a.ch_03.state | usint |  | 512 | Yes |   | Статус канала
ai6a.ch_03.mode | usint |  | 512 |  | 0x00000400 | Режим канала:<br/>0: 0..20мА<br/>1: 4..40мА<br/>
ai6a.ch_03.correct | uint |  | 512 |  | 0x00000400 | Калибровка канала:<br/>0: нет действий<br/>1: Корректировка 1 точки (4 мА)<br/>2: Корректировка 2 точки (12 мА)<br/>3: Корректировка 3 точки (20 мА)<br/>
ai6a.ch_03.simulate.max | uint |  | 512 |  |   | Код АЦП. Максимум симулированного значения
ai6a.ch_03.simulate.min | uint |  | 512 |  |   | Код АЦП. Минимум симулированного значения
ai6a.ch_03.simulate.value | uint |  | 512 |  |   | Код АЦП. Симулированное значение
ai6a.ch_03.simulate.speed | int |  | 512 |  |   | Код АЦП. Скорость изменения симулированного значения
ai6a.ch_04.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6a.ch_04.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6a.ch_04.adc | uint |  | 512 | Yes |   | Текущий код АЦП
ai6a.ch_04.current | real |  | 512 | Yes |   | Текущее значение тока
ai6a.ch_04.state | usint |  | 512 | Yes |   | Статус канала
ai6a.ch_04.mode | usint |  | 512 |  | 0x00000400 | Режим канала:<br/>0: 0..20мА<br/>1: 4..40мА<br/>
ai6a.ch_04.correct | uint |  | 512 |  | 0x00000400 | Калибровка канала:<br/>0: нет действий<br/>1: Корректировка 1 точки (4 мА)<br/>2: Корректировка 2 точки (12 мА)<br/>3: Корректировка 3 точки (20 мА)<br/>
ai6a.ch_04.simulate.max | uint |  | 512 |  |   | Код АЦП. Максимум симулированного значения
ai6a.ch_04.simulate.min | uint |  | 512 |  |   | Код АЦП. Минимум симулированного значения
ai6a.ch_04.simulate.value | uint |  | 512 |  |   | Код АЦП. Симулированное значение
ai6a.ch_04.simulate.speed | int |  | 512 |  |   | Код АЦП. Скорость изменения симулированного значения
ai6a.ch_05.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
ai6a.ch_05.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0020: Усреднение значения<br/>
ai6a.ch_05.adc | uint |  | 512 | Yes |   | Текущий код АЦП
ai6a.ch_05.current | real |  | 512 | Yes |   | Текущее значение тока
ai6a.ch_05.state | usint |  | 512 | Yes |   | Статус канала
ai6a.ch_05.mode | usint |  | 512 |  | 0x00000400 | Режим канала:<br/>0: 0..20мА<br/>1: 4..40мА<br/>
ai6a.ch_05.correct | uint |  | 512 |  | 0x00000400 | Калибровка канала:<br/>0: нет действий<br/>1: Корректировка 1 точки (4 мА)<br/>2: Корректировка 2 точки (12 мА)<br/>3: Корректировка 3 точки (20 мА)<br/>
ai6a.ch_05.simulate.max | uint |  | 512 |  |   | Код АЦП. Максимум симулированного значения
ai6a.ch_05.simulate.min | uint |  | 512 |  |   | Код АЦП. Минимум симулированного значения
ai6a.ch_05.simulate.value | uint |  | 512 |  |   | Код АЦП. Симулированное значение
ai6a.ch_05.simulate.speed | int |  | 512 |  |   | Код АЦП. Скорость изменения симулированного значения


[^simtype]: **Тип симуляции:**<br/>0: Симуляция отключена<br/>1: Константное значение<br/>2: Линейная функция<br/>3: Синусообразная функция<br/>4: Случайное значение<br/></br>

<p align='right'><a href='index.html'>[Оглавление]</a></p>

