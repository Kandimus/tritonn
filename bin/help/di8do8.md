<p align='right'><a href='index.html'>[Оглавление]</a></p>

# di8do8
> 0.19.103.fa31bc93
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
di8do8.type | UINT |  | 512 | Yes |   | Нет данных
di8do8.node | UINT |  | 512 | Yes |   | Нет данных
di8do8.vendor | UDINT |  | 512 | Yes |   | Нет данных
di8do8.productcode | UDINT |  | 512 | Yes |   | Нет данных
di8do8.revision | UDINT |  | 512 | Yes |   | Нет данных
di8do8.serialnumber | UDINT |  | 512 | Yes |   | Нет данных
di8do8.temperature | REAL |  | 512 | Yes |   | Нет данных
di8do8.can | UINT |  | 512 | Yes |   | Нет данных
di8do8.firmware | UINT |  | 512 | Yes |   | Нет данных
di8do8.hardware | UINT |  | 512 | Yes |   | Нет данных
di8do8.ch_00.simulate.type | USINT |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_00.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_00.value | USINT |  | 512 | Yes |   | Текущее значение
di8do8.ch_00.state | USINT |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_00.bounce | UDINT | мс | 210 |  |   | Значение таймера антидребезга
di8do8.ch_00.simulate.value | UINT |  | 512 |  |   | Значение симулированного значения
di8do8.ch_00.simulate.blink | UDINT | мс | 210 |  |   | Период мигания
di8do8.ch_01.simulate.type | USINT |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_01.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_01.value | USINT |  | 512 | Yes |   | Текущее значение
di8do8.ch_01.state | USINT |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_01.bounce | UDINT | мс | 210 |  |   | Значение таймера антидребезга
di8do8.ch_01.simulate.value | UINT |  | 512 |  |   | Значение симулированного значения
di8do8.ch_01.simulate.blink | UDINT | мс | 210 |  |   | Период мигания
di8do8.ch_02.simulate.type | USINT |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_02.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_02.value | USINT |  | 512 | Yes |   | Текущее значение
di8do8.ch_02.state | USINT |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_02.bounce | UDINT | мс | 210 |  |   | Значение таймера антидребезга
di8do8.ch_02.simulate.value | UINT |  | 512 |  |   | Значение симулированного значения
di8do8.ch_02.simulate.blink | UDINT | мс | 210 |  |   | Период мигания
di8do8.ch_03.simulate.type | USINT |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_03.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_03.value | USINT |  | 512 | Yes |   | Текущее значение
di8do8.ch_03.state | USINT |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_03.bounce | UDINT | мс | 210 |  |   | Значение таймера антидребезга
di8do8.ch_03.simulate.value | UINT |  | 512 |  |   | Значение симулированного значения
di8do8.ch_03.simulate.blink | UDINT | мс | 210 |  |   | Период мигания
di8do8.ch_04.simulate.type | USINT |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_04.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_04.value | USINT |  | 512 | Yes |   | Текущее значение
di8do8.ch_04.state | USINT |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_04.bounce | UDINT | мс | 210 |  |   | Значение таймера антидребезга
di8do8.ch_04.simulate.value | UINT |  | 512 |  |   | Значение симулированного значения
di8do8.ch_04.simulate.blink | UDINT | мс | 210 |  |   | Период мигания
di8do8.ch_05.simulate.type | USINT |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_05.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_05.value | USINT |  | 512 | Yes |   | Текущее значение
di8do8.ch_05.state | USINT |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_05.bounce | UDINT | мс | 210 |  |   | Значение таймера антидребезга
di8do8.ch_05.simulate.value | UINT |  | 512 |  |   | Значение симулированного значения
di8do8.ch_05.simulate.blink | UDINT | мс | 210 |  |   | Период мигания
di8do8.ch_06.simulate.type | USINT |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_06.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_06.value | USINT |  | 512 | Yes |   | Текущее значение
di8do8.ch_06.state | USINT |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_06.bounce | UDINT | мс | 210 |  |   | Значение таймера антидребезга
di8do8.ch_06.simulate.value | UINT |  | 512 |  |   | Значение симулированного значения
di8do8.ch_06.simulate.blink | UDINT | мс | 210 |  |   | Период мигания
di8do8.ch_07.simulate.type | USINT |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
di8do8.ch_07.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
di8do8.ch_07.value | USINT |  | 512 | Yes |   | Текущее значение
di8do8.ch_07.state | USINT |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_07.bounce | UDINT | мс | 210 |  |   | Значение таймера антидребезга
di8do8.ch_07.simulate.value | UINT |  | 512 |  |   | Значение симулированного значения
di8do8.ch_07.simulate.blink | UDINT | мс | 210 |  |   | Период мигания
di8do8.ch_08.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0004: Инверсия значения<br/>
di8do8.ch_08.value | UINT |  | 512 | Yes |   | Текущее значение
di8do8.ch_08.state | USINT |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_09.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0004: Инверсия значения<br/>
di8do8.ch_09.value | UINT |  | 512 | Yes |   | Текущее значение
di8do8.ch_09.state | USINT |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_10.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0004: Инверсия значения<br/>
di8do8.ch_10.value | UINT |  | 512 | Yes |   | Текущее значение
di8do8.ch_10.state | USINT |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_11.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0004: Инверсия значения<br/>
di8do8.ch_11.value | UINT |  | 512 | Yes |   | Текущее значение
di8do8.ch_11.state | USINT |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_12.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0004: Инверсия значения<br/>
di8do8.ch_12.value | UINT |  | 512 | Yes |   | Текущее значение
di8do8.ch_12.state | USINT |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_13.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0004: Инверсия значения<br/>
di8do8.ch_13.value | UINT |  | 512 | Yes |   | Текущее значение
di8do8.ch_13.state | USINT |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_14.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0004: Инверсия значения<br/>
di8do8.ch_14.value | UINT |  | 512 | Yes |   | Текущее значение
di8do8.ch_14.state | USINT |  | 512 | Yes |   | Статус:<br/>Нет данных
di8do8.ch_15.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0004: Инверсия значения<br/>
di8do8.ch_15.value | UINT |  | 512 | Yes |   | Текущее значение
di8do8.ch_15.state | USINT |  | 512 | Yes |   | Статус:<br/>Нет данных


[^simtype]: **Тип симуляции DI:**<br/>0: Симуляция отключена<br/>1: Константное значение<br/>2: Пульсация сигнала<br/>3: Случайное значение<br/><br/>

<p align='right'><a href='index.html'>[Оглавление]</a></p>

