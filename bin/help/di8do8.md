<p align='right'><a href='index.html'>[Оглавление]</a></p>

# di8do8
> 0.19.165.ba8170a6
## XML
````xml
<module type="di8do8" name="valid object name" description="string index" >
	<channel number="0" setup="DI setup flags" />
	<channel number="1" setup="DI setup flags" />
	<channel number="2" setup="DI setup flags" />
	<channel number="3" setup="DI setup flags" />
	<channel number="4" setup="DI setup flags" />
	<channel number="5" setup="DI setup flags" />
	<channel number="6" setup="DI setup flags" />
	<channel number="7" setup="DI setup flags" />
	<channel number="8" setup="DO setup flags" />
	<channel number="9" setup="DO setup flags" />
	<channel number="10" setup="DO setup flags" />
	<channel number="11" setup="DO setup flags" />
	<channel number="12" setup="DO setup flags" />
	<channel number="13" setup="DO setup flags" />
	<channel number="14" setup="DO setup flags" />
	<channel number="15" setup="DO setup flags" />
</module>
````

## Channels
Number | Type | Comment
:-- |:--:|:--
0 | DI | 
1 | DI | 
2 | DI | 
3 | DI | 
4 | DI | 
5 | DI | 
6 | DI | 
7 | DI | 
8 | DO | 
9 | DO | 
10 | DO | 
11 | DO | 
12 | DO | 
13 | DO | 
14 | DO | 
15 | DO | 


#### DI setup
* _OFF_  - Выключен и не обрабатывается
* _FILTER_  - Устранение дребезга
* _INVERSED_  - Инверсия значения

#### DO setup
* _OFF_  - Выключен и не обрабатывается
* _PULSE_  - Выдать одиночный импульс
* _INVERSED_  - Инверсия значения

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:-- |:--
di8do8.type | uint |  | 512 | Yes |   | Тип модуля:<br>0: Модуль не определен<br/>1: Базовый модуль ЦПУ<br/>2: Модуль AI6a<br/>3: Модуль AI6p<br/>4: Модуль FI4<br/>5: Модуль DI8DO8<br/>6: Модуль DI16<br/>7: Модуль DO16<br/>8: Модуль CRM<br/>9: Модуль AO4<br/>
di8do8.node | uint |  | 512 | Yes |   | Нет данных
di8do8.vendor | udint |  | 512 | Yes |   | Нет данных
di8do8.productcode | udint |  | 512 | Yes |   | Нет данных
di8do8.revision | udint |  | 512 | Yes |   | Нет данных
di8do8.serialnumber | udint |  | 512 | Yes |   | Нет данных
di8do8.can | uint |  | 512 | Yes |   | Нет данных
di8do8.firmware | uint |  | 512 | Yes |   | Нет данных
di8do8.hardware | uint |  | 512 | Yes |   | Нет данных
di8do8.ch_00.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_00.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_00.value | usint |  | 512 | Yes |   | Текущее значение
di8do8.ch_00.phvalue | usint |  | 512 | Yes |   | Значение сигнала на модуле
di8do8.ch_00.filter | udint | мс | 210 |  |   | Значение таймера фильтрации
di8do8.ch_00.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
di8do8.ch_00.simulate.blink | udint | мс | 210 |  |   | Период мигания
di8do8.ch_01.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_01.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_01.value | usint |  | 512 | Yes |   | Текущее значение
di8do8.ch_01.phvalue | usint |  | 512 | Yes |   | Значение сигнала на модуле
di8do8.ch_01.filter | udint | мс | 210 |  |   | Значение таймера фильтрации
di8do8.ch_01.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
di8do8.ch_01.simulate.blink | udint | мс | 210 |  |   | Период мигания
di8do8.ch_02.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_02.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_02.value | usint |  | 512 | Yes |   | Текущее значение
di8do8.ch_02.phvalue | usint |  | 512 | Yes |   | Значение сигнала на модуле
di8do8.ch_02.filter | udint | мс | 210 |  |   | Значение таймера фильтрации
di8do8.ch_02.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
di8do8.ch_02.simulate.blink | udint | мс | 210 |  |   | Период мигания
di8do8.ch_03.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_03.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_03.value | usint |  | 512 | Yes |   | Текущее значение
di8do8.ch_03.phvalue | usint |  | 512 | Yes |   | Значение сигнала на модуле
di8do8.ch_03.filter | udint | мс | 210 |  |   | Значение таймера фильтрации
di8do8.ch_03.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
di8do8.ch_03.simulate.blink | udint | мс | 210 |  |   | Период мигания
di8do8.ch_04.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_04.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_04.value | usint |  | 512 | Yes |   | Текущее значение
di8do8.ch_04.phvalue | usint |  | 512 | Yes |   | Значение сигнала на модуле
di8do8.ch_04.filter | udint | мс | 210 |  |   | Значение таймера фильтрации
di8do8.ch_04.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
di8do8.ch_04.simulate.blink | udint | мс | 210 |  |   | Период мигания
di8do8.ch_05.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_05.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_05.value | usint |  | 512 | Yes |   | Текущее значение
di8do8.ch_05.phvalue | usint |  | 512 | Yes |   | Значение сигнала на модуле
di8do8.ch_05.filter | udint | мс | 210 |  |   | Значение таймера фильтрации
di8do8.ch_05.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
di8do8.ch_05.simulate.blink | udint | мс | 210 |  |   | Период мигания
di8do8.ch_06.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_06.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_06.value | usint |  | 512 | Yes |   | Текущее значение
di8do8.ch_06.phvalue | usint |  | 512 | Yes |   | Значение сигнала на модуле
di8do8.ch_06.filter | udint | мс | 210 |  |   | Значение таймера фильтрации
di8do8.ch_06.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
di8do8.ch_06.simulate.blink | udint | мс | 210 |  |   | Период мигания
di8do8.ch_07.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_07.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_07.value | usint |  | 512 | Yes |   | Текущее значение
di8do8.ch_07.phvalue | usint |  | 512 | Yes |   | Значение сигнала на модуле
di8do8.ch_07.filter | udint | мс | 210 |  |   | Значение таймера фильтрации
di8do8.ch_07.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
di8do8.ch_07.simulate.blink | udint | мс | 210 |  |   | Период мигания
di8do8.ch_08.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Выдать одиночный импульс<br/>0x0004: Инверсия значения<br/>
di8do8.ch_08.value | uint |  | 512 |  |   | Текущее значение
di8do8.ch_08.pulse | udint | мс | 210 |  |   | Длина одиночного импульса
di8do8.ch_09.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Выдать одиночный импульс<br/>0x0004: Инверсия значения<br/>
di8do8.ch_09.value | uint |  | 512 |  |   | Текущее значение
di8do8.ch_09.pulse | udint | мс | 210 |  |   | Длина одиночного импульса
di8do8.ch_10.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Выдать одиночный импульс<br/>0x0004: Инверсия значения<br/>
di8do8.ch_10.value | uint |  | 512 |  |   | Текущее значение
di8do8.ch_10.pulse | udint | мс | 210 |  |   | Длина одиночного импульса
di8do8.ch_11.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Выдать одиночный импульс<br/>0x0004: Инверсия значения<br/>
di8do8.ch_11.value | uint |  | 512 |  |   | Текущее значение
di8do8.ch_11.pulse | udint | мс | 210 |  |   | Длина одиночного импульса
di8do8.ch_12.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Выдать одиночный импульс<br/>0x0004: Инверсия значения<br/>
di8do8.ch_12.value | uint |  | 512 |  |   | Текущее значение
di8do8.ch_12.pulse | udint | мс | 210 |  |   | Длина одиночного импульса
di8do8.ch_13.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Выдать одиночный импульс<br/>0x0004: Инверсия значения<br/>
di8do8.ch_13.value | uint |  | 512 |  |   | Текущее значение
di8do8.ch_13.pulse | udint | мс | 210 |  |   | Длина одиночного импульса
di8do8.ch_14.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Выдать одиночный импульс<br/>0x0004: Инверсия значения<br/>
di8do8.ch_14.value | uint |  | 512 |  |   | Текущее значение
di8do8.ch_14.pulse | udint | мс | 210 |  |   | Длина одиночного импульса
di8do8.ch_15.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Выдать одиночный импульс<br/>0x0004: Инверсия значения<br/>
di8do8.ch_15.value | uint |  | 512 |  |   | Текущее значение
di8do8.ch_15.pulse | udint | мс | 210 |  |   | Длина одиночного импульса


[^simtype]: **Тип симуляции DI:**<br/>0: Симуляция отключена<br/>1: Константное значение<br/>2: Пульсация сигнала<br/>3: Случайное значение<br/><br/>

<p align='right'><a href='index.html'>[Оглавление]</a></p>

