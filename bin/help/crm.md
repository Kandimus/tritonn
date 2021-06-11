<p align='right'><a href='index.html'>[Оглавление]</a></p>

# crm
> 0.19.118.da2b0fb8
## XML
````xml
<module name="crm" description="string index" >
	<channel number="0" setup="DI setup flags" />
	<channel number="1" setup="DI setup flags" />
	<channel number="2" setup="DI setup flags" />
	<channel number="3" setup="DI setup flags" />
	<channel number="4" setup="FI setup flags" />
</module>
````

## Channels
Number | Type | Comment
:-- |:--:|:--
0 | DI | Детектор 1
1 | DI | Детектор 2
2 | DI | Детектор 3
3 | DI | Детектор 4
4 | FI | Частота с ПР


#### DI setup
* _OFF_  - Выключен и не обрабатывается
* _BOUNCE_  - Устранение дребезга
* _INVERSED_  - Инверсия значения

#### FI setup
* _OFF_  - Выключен и не обрабатывается
* _AVERAGE_  - Усреднять значение частоты

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:-- |:--
crm.type | uint |  | 512 | Yes |   | Нет данных
crm.node | uint |  | 512 | Yes |   | Нет данных
crm.vendor | udint |  | 512 | Yes |   | Нет данных
crm.productcode | udint |  | 512 | Yes |   | Нет данных
crm.revision | udint |  | 512 | Yes |   | Нет данных
crm.serialnumber | udint |  | 512 | Yes |   | Нет данных
crm.temperature | real |  | 512 | Yes |   | Нет данных
crm.can | uint |  | 512 | Yes |   | Нет данных
crm.firmware | uint |  | 512 | Yes |   | Нет данных
crm.hardware | uint |  | 512 | Yes |   | Нет данных
crm.ch_01.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
crm.ch_01.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
crm.ch_01.value | usint |  | 512 | Yes |   | Текущее значение
crm.ch_01.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
crm.ch_01.bounce | udint | мс | 210 |  |   | Значение таймера антидребезга
crm.ch_01.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
crm.ch_01.simulate.blink | udint | мс | 210 |  |   | Период мигания
crm.ch_02.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
crm.ch_02.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
crm.ch_02.value | usint |  | 512 | Yes |   | Текущее значение
crm.ch_02.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
crm.ch_02.bounce | udint | мс | 210 |  |   | Значение таймера антидребезга
crm.ch_02.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
crm.ch_02.simulate.blink | udint | мс | 210 |  |   | Период мигания
crm.ch_03.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
crm.ch_03.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
crm.ch_03.value | usint |  | 512 | Yes |   | Текущее значение
crm.ch_03.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
crm.ch_03.bounce | udint | мс | 210 |  |   | Значение таймера антидребезга
crm.ch_03.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
crm.ch_03.simulate.blink | udint | мс | 210 |  |   | Период мигания
crm.ch_04.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
crm.ch_04.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
crm.ch_04.value | usint |  | 512 | Yes |   | Текущее значение
crm.ch_04.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
crm.ch_04.bounce | udint | мс | 210 |  |   | Значение таймера антидребезга
crm.ch_04.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
crm.ch_04.simulate.blink | udint | мс | 210 |  |   | Период мигания
crm.ch_05.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
crm.ch_05.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Усреднять значение частоты<br/>
crm.ch_05.count | udint |  | 512 | Yes |   | Количество накопленных импульсов
crm.ch_05.frequecy | lreal | Гц | 192 | Yes |   | Частота
crm.ch_05.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
crm.ch_05.simulate.max | uint |  | 512 |  |   | Количество импульсов в секунду. Максимум симулированного значения
crm.ch_05.simulate.min | uint |  | 512 |  |   | Количество импульсов в секунду. Минимум симулированного значения
crm.ch_05.simulate.value | uint |  | 512 |  |   | Количество импульсов в секунду. Симулированное значение
crm.ch_05.simulate.speed | int |  | 512 |  |   | Количество импульсов в секунду. Скорость изменения симулированного значения


[^simtype]: **Тип симуляции DI:**<br/>0: Симуляция отключена<br/>1: Константное значение<br/>2: Пульсация сигнала<br/>3: Случайное значение<br/><br/>**Тип симуляции FI:**</br>0: Симуляция отключена<br/>1: Константное значение<br/>2: Линейная функция<br/>3: Синусообразная функция<br/>4: Случайное значение<br/>

<p align='right'><a href='index.html'>[Оглавление]</a></p>

