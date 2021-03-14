<p align='right'><a href='index.html'>[Оглавление]</a></p>

# fi4
> 0.19.18.6bb32f40
## XML
````xml
<fi4 name="valid object name" descr="string index" >
	<channel number="0" setup="FI setup flags" />
	<channel number="1" setup="FI setup flags" />
	<channel number="2" setup="FI setup flags" />
	<channel number="3" setup="FI setup flags" />
</fi4>
````

## Channels
Number | Type | Comment
:-- |:--:|:--
0 | FI | 
1 | FI | 
2 | FI | 
3 | FI | 


#### FI setup
* _OFF_  - Выключен и не обрабатывается
* _AVERAGE_  - Усреднять значение частоты

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:-- |:--
fi4.type | UINT |  | 512 | Yes |   | Нет данных
fi4.node | UINT |  | 512 | Yes |   | Нет данных
fi4.vendor | UDINT |  | 512 | Yes |   | Нет данных
fi4.productcode | UDINT |  | 512 | Yes |   | Нет данных
fi4.revision | UDINT |  | 512 | Yes |   | Нет данных
fi4.serialnumber | UDINT |  | 512 | Yes |   | Нет данных
fi4.temperature | REAL |  | 512 | Yes |   | Нет данных
fi4.can | UINT |  | 512 | Yes |   | Нет данных
fi4.firmware | UINT |  | 512 | Yes |   | Нет данных
fi4.hardware | UINT |  | 512 | Yes |   | Нет данных
fi4.ch_01.simulate.type | USINT |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
fi4.ch_01.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Усреднять значение частоты<br/>
fi4.ch_01.count | UDINT |  | 512 | Yes |   | Количество накопленных импульсов
fi4.ch_01.frequecy | UDINT | Гц | 192 | Yes |   | Частота
fi4.ch_01.state | USINT |  | 512 | Yes |   | Статус:<br/>Нет данных
fi4.ch_01.simulate.max | UINT |  | 512 |  |   | Максимум симулированного значения
fi4.ch_01.simulate.min | UINT |  | 512 |  |   | Минимум симулированного значения
fi4.ch_01.simulate.value | UINT |  | 512 |  |   | Симулированное значение
fi4.ch_01.simulate.speed | INT |  | 512 |  |   | Скорость изменения симулированного значения
fi4.ch_02.simulate.type | USINT |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
fi4.ch_02.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Усреднять значение частоты<br/>
fi4.ch_02.count | UDINT |  | 512 | Yes |   | Количество накопленных импульсов
fi4.ch_02.frequecy | UDINT | Гц | 192 | Yes |   | Частота
fi4.ch_02.state | USINT |  | 512 | Yes |   | Статус:<br/>Нет данных
fi4.ch_02.simulate.max | UINT |  | 512 |  |   | Максимум симулированного значения
fi4.ch_02.simulate.min | UINT |  | 512 |  |   | Минимум симулированного значения
fi4.ch_02.simulate.value | UINT |  | 512 |  |   | Симулированное значение
fi4.ch_02.simulate.speed | INT |  | 512 |  |   | Скорость изменения симулированного значения
fi4.ch_03.simulate.type | USINT |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
fi4.ch_03.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Усреднять значение частоты<br/>
fi4.ch_03.count | UDINT |  | 512 | Yes |   | Количество накопленных импульсов
fi4.ch_03.frequecy | UDINT | Гц | 192 | Yes |   | Частота
fi4.ch_03.state | USINT |  | 512 | Yes |   | Статус:<br/>Нет данных
fi4.ch_03.simulate.max | UINT |  | 512 |  |   | Максимум симулированного значения
fi4.ch_03.simulate.min | UINT |  | 512 |  |   | Минимум симулированного значения
fi4.ch_03.simulate.value | UINT |  | 512 |  |   | Симулированное значение
fi4.ch_03.simulate.speed | INT |  | 512 |  |   | Скорость изменения симулированного значения
fi4.ch_04.simulate.type | USINT |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
fi4.ch_04.setup | UINT |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Усреднять значение частоты<br/>
fi4.ch_04.count | UDINT |  | 512 | Yes |   | Количество накопленных импульсов
fi4.ch_04.frequecy | UDINT | Гц | 192 | Yes |   | Частота
fi4.ch_04.state | USINT |  | 512 | Yes |   | Статус:<br/>Нет данных
fi4.ch_04.simulate.max | UINT |  | 512 |  |   | Максимум симулированного значения
fi4.ch_04.simulate.min | UINT |  | 512 |  |   | Минимум симулированного значения
fi4.ch_04.simulate.value | UINT |  | 512 |  |   | Симулированное значение
fi4.ch_04.simulate.speed | INT |  | 512 |  |   | Скорость изменения симулированного значения
fi4outtype | USINT |  | 512 | Yes | 0x80000000 | Выбранный канал для коммутации с выходом


[^simtype]: **Тип симуляции:**<br/>0: Симуляция отключена<br/>1: Константное значение<br/>2: Линейная функция<br/>3: Синусообразная функция<br/>4: Случайное значение<br/><br/>
[^mutable]: Если объект не привязан к модулю ввода-вывода, то данная переменная будет записываемой.


<p align='right'><a href='index.html'>[Оглавление]</a></p>
