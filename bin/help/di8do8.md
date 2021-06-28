<p align='right'><a href='index.html'>[Оглавление]</a></p>

# di8do8
> 0.19.132.77cddc06
## XML
````xml
<module name="di8do8" description="string index" >
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
* _BOUNCE_  - Устранение дребезга
* _INVERSED_  - Инверсия значения

#### DO setup
* _OFF_  - Выключен и не обрабатывается
* _INVERSED_  - Инверсия значения

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:-- |:--
di8do8.type | uint |  | 512 | Yes |   | Нет данных
di8do8.node | uint |  | 512 | Yes |   | Нет данных
di8do8.vendor | udint |  | 512 | Yes |   | Нет данных
di8do8.productcode | udint |  | 512 | Yes |   | Нет данных
di8do8.revision | udint |  | 512 | Yes |   | Нет данных
di8do8.serialnumber | udint |  | 512 | Yes |   | Нет данных
di8do8.temperature | real |  | 512 | Yes |   | Нет данных
di8do8.can | uint |  | 512 | Yes |   | Нет данных
di8do8.firmware | uint |  | 512 | Yes |   | Нет данных
di8do8.hardware | uint |  | 512 | Yes |   | Нет данных
di8do8.ch_00.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_00.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_00.value | usint |  | 512 | Yes |   | Текущее значение
di8do8.ch_00.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_00.bounce | udint | мс | 210 |  |   | Значение таймера антидребезга
di8do8.ch_00.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
di8do8.ch_00.simulate.blink | udint | мс | 210 |  |   | Период мигания
di8do8.ch_01.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_01.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_01.value | usint |  | 512 | Yes |   | Текущее значение
di8do8.ch_01.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_01.bounce | udint | мс | 210 |  |   | Значение таймера антидребезга
di8do8.ch_01.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
di8do8.ch_01.simulate.blink | udint | мс | 210 |  |   | Период мигания
di8do8.ch_02.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_02.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_02.value | usint |  | 512 | Yes |   | Текущее значение
di8do8.ch_02.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_02.bounce | udint | мс | 210 |  |   | Значение таймера антидребезга
di8do8.ch_02.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
di8do8.ch_02.simulate.blink | udint | мс | 210 |  |   | Период мигания
di8do8.ch_03.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_03.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_03.value | usint |  | 512 | Yes |   | Текущее значение
di8do8.ch_03.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_03.bounce | udint | мс | 210 |  |   | Значение таймера антидребезга
di8do8.ch_03.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
di8do8.ch_03.simulate.blink | udint | мс | 210 |  |   | Период мигания
di8do8.ch_04.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_04.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_04.value | usint |  | 512 | Yes |   | Текущее значение
di8do8.ch_04.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_04.bounce | udint | мс | 210 |  |   | Значение таймера антидребезга
di8do8.ch_04.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
di8do8.ch_04.simulate.blink | udint | мс | 210 |  |   | Период мигания
di8do8.ch_05.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_05.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_05.value | usint |  | 512 | Yes |   | Текущее значение
di8do8.ch_05.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_05.bounce | udint | мс | 210 |  |   | Значение таймера антидребезга
di8do8.ch_05.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
di8do8.ch_05.simulate.blink | udint | мс | 210 |  |   | Период мигания
di8do8.ch_06.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_06.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_06.value | usint |  | 512 | Yes |   | Текущее значение
di8do8.ch_06.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_06.bounce | udint | мс | 210 |  |   | Значение таймера антидребезга
di8do8.ch_06.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
di8do8.ch_06.simulate.blink | udint | мс | 210 |  |   | Период мигания
di8do8.ch_07.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_07.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_07.value | usint |  | 512 | Yes |   | Текущее значение
di8do8.ch_07.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_07.bounce | udint | мс | 210 |  |   | Значение таймера антидребезга
di8do8.ch_07.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
di8do8.ch_07.simulate.blink | udint | мс | 210 |  |   | Период мигания
di8do8.ch_08.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0004: Инверсия значения<br/>
di8do8.ch_08.value | uint |  | 512 | Yes |   | Текущее значение
di8do8.ch_08.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_09.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0004: Инверсия значения<br/>
di8do8.ch_09.value | uint |  | 512 | Yes |   | Текущее значение
di8do8.ch_09.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_10.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0004: Инверсия значения<br/>
di8do8.ch_10.value | uint |  | 512 | Yes |   | Текущее значение
di8do8.ch_10.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_11.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0004: Инверсия значения<br/>
di8do8.ch_11.value | uint |  | 512 | Yes |   | Текущее значение
di8do8.ch_11.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_12.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0004: Инверсия значения<br/>
di8do8.ch_12.value | uint |  | 512 | Yes |   | Текущее значение
di8do8.ch_12.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_13.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0004: Инверсия значения<br/>
di8do8.ch_13.value | uint |  | 512 | Yes |   | Текущее значение
di8do8.ch_13.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_14.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0004: Инверсия значения<br/>
di8do8.ch_14.value | uint |  | 512 | Yes |   | Текущее значение
di8do8.ch_14.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_15.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0004: Инверсия значения<br/>
di8do8.ch_15.value | uint |  | 512 | Yes |   | Текущее значение
di8do8.ch_15.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных


[^simtype]: **Тип симуляции DI:**<br/>0: Симуляция отключена<br/>1: Константное значение<br/>2: Пульсация сигнала<br/>3: Случайное значение<br/><br/>

<p align='right'><a href='index.html'>[Оглавление]</a></p>

