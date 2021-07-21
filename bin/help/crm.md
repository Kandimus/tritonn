<p align='right'><a href='index.html'>[Оглавление]</a></p>

# crm
> 0.19.156.28f9fa2c
## XML
````xml
<module type="crm" name="valid object name" description="string index" >
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
* _FILTER_  - Устранение дребезга
* _INVERSED_  - Инверсия значения

#### FI setup
* _OFF_  - Выключен и не обрабатывается
* _AVERAGE_  - Усреднять значение частоты

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:-- |:--
crm.type | uint |  | 512 | Yes |   | Тип модуля:<br>0: Модуль не определен<br/>1: Базовый модуль ЦПУ<br/>2: Модуль AI6a<br/>3: Модуль AI6p<br/>4: Модуль FI4<br/>5: Модуль DI8DO8<br/>6: Модуль DI16<br/>7: Модуль DO16<br/>8: Модуль CRM<br/>
crm.node | uint |  | 512 | Yes |   | Нет данных
crm.vendor | udint |  | 512 | Yes |   | Нет данных
crm.productcode | udint |  | 512 | Yes |   | Нет данных
crm.revision | udint |  | 512 | Yes |   | Нет данных
crm.serialnumber | udint |  | 512 | Yes |   | Нет данных
crm.can | uint |  | 512 | Yes |   | Нет данных
crm.firmware | uint |  | 512 | Yes |   | Нет данных
crm.hardware | uint |  | 512 | Yes |   | Нет данных
crm.ch_01.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
crm.ch_01.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
crm.ch_01.value | usint |  | 512 | Yes |   | Текущее значение
crm.ch_01.phvalue | usint |  | 512 | Yes |   | Значение сигнала на модуле
crm.ch_01.filter | udint | мс | 210 |  |   | Значение таймера фильтрации
crm.ch_01.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
crm.ch_01.simulate.blink | udint | мс | 210 |  |   | Период мигания
crm.ch_02.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
crm.ch_02.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
crm.ch_02.value | usint |  | 512 | Yes |   | Текущее значение
crm.ch_02.phvalue | usint |  | 512 | Yes |   | Значение сигнала на модуле
crm.ch_02.filter | udint | мс | 210 |  |   | Значение таймера фильтрации
crm.ch_02.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
crm.ch_02.simulate.blink | udint | мс | 210 |  |   | Период мигания
crm.ch_03.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
crm.ch_03.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
crm.ch_03.value | usint |  | 512 | Yes |   | Текущее значение
crm.ch_03.phvalue | usint |  | 512 | Yes |   | Значение сигнала на модуле
crm.ch_03.filter | udint | мс | 210 |  |   | Значение таймера фильтрации
crm.ch_03.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
crm.ch_03.simulate.blink | udint | мс | 210 |  |   | Период мигания
crm.ch_04.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
crm.ch_04.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Устранение дребезга<br/>0x0004: Инверсия значения<br/>
crm.ch_04.value | usint |  | 512 | Yes |   | Текущее значение
crm.ch_04.phvalue | usint |  | 512 | Yes |   | Значение сигнала на модуле
crm.ch_04.filter | udint | мс | 210 |  |   | Значение таймера фильтрации
crm.ch_04.simulate.value | uint |  | 512 |  |   | Значение симулированного значения
crm.ch_04.simulate.blink | udint | мс | 210 |  |   | Период мигания
crm.ch_05.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
crm.ch_05.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Усреднять значение частоты<br/>
crm.ch_05.count | udint | имп | 514 | Yes |   | Количество накопленных импульсов
crm.ch_05.frequecy | lreal | Гц | 192 | Yes |   | Частота
crm.ch_05.simulate.max | uint |  | 512 |  |   | Количество импульсов в секунду. Максимум симулированного значения
crm.ch_05.simulate.min | uint |  | 512 |  |   | Количество импульсов в секунду. Минимум симулированного значения
crm.ch_05.simulate.value | uint |  | 512 |  |   | Количество импульсов в секунду. Симулированное значение
crm.ch_05.simulate.speed | int |  | 512 |  |   | Количество импульсов в секунду. Скорость изменения симулированного значения


[^simtype]: **Тип симуляции DI:**<br/>0: Симуляция отключена<br/>1: Константное значение<br/>2: Пульсация сигнала<br/>3: Случайное значение<br/><br/>**Тип симуляции FI:**</br>0: Симуляция отключена<br/>1: Константное значение<br/>2: Линейная функция<br/>3: Синусообразная функция<br/>4: Случайное значение<br/>

<p align='right'><a href='index.html'>[Оглавление]</a></p>

