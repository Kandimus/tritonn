<p align='right'><a href='index.html'>[Оглавление]</a></p>

# fi4
> 0.19.130.2f0e6b68
## XML
````xml
<module name="fi4" description="string index" >
	<channel number="0" setup="FI setup flags" />
	<channel number="1" setup="FI setup flags" />
	<channel number="2" setup="FI setup flags" />
	<channel number="3" setup="FI setup flags" />
</module>
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
fi4.type | uint |  | 512 | Yes |   | Нет данных
fi4.node | uint |  | 512 | Yes |   | Нет данных
fi4.vendor | udint |  | 512 | Yes |   | Нет данных
fi4.productcode | udint |  | 512 | Yes |   | Нет данных
fi4.revision | udint |  | 512 | Yes |   | Нет данных
fi4.serialnumber | udint |  | 512 | Yes |   | Нет данных
fi4.temperature | real |  | 512 | Yes |   | Нет данных
fi4.can | uint |  | 512 | Yes |   | Нет данных
fi4.firmware | uint |  | 512 | Yes |   | Нет данных
fi4.hardware | uint |  | 512 | Yes |   | Нет данных
fi4.ch_00.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
fi4.ch_00.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Усреднять значение частоты<br/>
fi4.ch_00.count | udint |  | 512 | Yes |   | Количество накопленных импульсов
fi4.ch_00.frequecy | lreal | Гц | 192 | Yes |   | Частота
fi4.ch_00.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
fi4.ch_00.simulate.max | uint |  | 512 |  |   | Количество импульсов в секунду. Максимум симулированного значения
fi4.ch_00.simulate.min | uint |  | 512 |  |   | Количество импульсов в секунду. Минимум симулированного значения
fi4.ch_00.simulate.value | uint |  | 512 |  |   | Количество импульсов в секунду. Симулированное значение
fi4.ch_00.simulate.speed | int |  | 512 |  |   | Количество импульсов в секунду. Скорость изменения симулированного значения
fi4.ch_01.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
fi4.ch_01.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Усреднять значение частоты<br/>
fi4.ch_01.count | udint |  | 512 | Yes |   | Количество накопленных импульсов
fi4.ch_01.frequecy | lreal | Гц | 192 | Yes |   | Частота
fi4.ch_01.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
fi4.ch_01.simulate.max | uint |  | 512 |  |   | Количество импульсов в секунду. Максимум симулированного значения
fi4.ch_01.simulate.min | uint |  | 512 |  |   | Количество импульсов в секунду. Минимум симулированного значения
fi4.ch_01.simulate.value | uint |  | 512 |  |   | Количество импульсов в секунду. Симулированное значение
fi4.ch_01.simulate.speed | int |  | 512 |  |   | Количество импульсов в секунду. Скорость изменения симулированного значения
fi4.ch_02.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
fi4.ch_02.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Усреднять значение частоты<br/>
fi4.ch_02.count | udint |  | 512 | Yes |   | Количество накопленных импульсов
fi4.ch_02.frequecy | lreal | Гц | 192 | Yes |   | Частота
fi4.ch_02.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
fi4.ch_02.simulate.max | uint |  | 512 |  |   | Количество импульсов в секунду. Максимум симулированного значения
fi4.ch_02.simulate.min | uint |  | 512 |  |   | Количество импульсов в секунду. Минимум симулированного значения
fi4.ch_02.simulate.value | uint |  | 512 |  |   | Количество импульсов в секунду. Симулированное значение
fi4.ch_02.simulate.speed | int |  | 512 |  |   | Количество импульсов в секунду. Скорость изменения симулированного значения
fi4.ch_03.simulate.type | usint |  | 512 |  |   | Тип симуляции (зависит от типа канала) [^simtype]
fi4.ch_03.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0002: Усреднять значение частоты<br/>
fi4.ch_03.count | udint |  | 512 | Yes |   | Количество накопленных импульсов
fi4.ch_03.frequecy | lreal | Гц | 192 | Yes |   | Частота
fi4.ch_03.state | usint |  | 512 | Yes |   | Статус:<br/>Нет данных
fi4.ch_03.simulate.max | uint |  | 512 |  |   | Количество импульсов в секунду. Максимум симулированного значения
fi4.ch_03.simulate.min | uint |  | 512 |  |   | Количество импульсов в секунду. Минимум симулированного значения
fi4.ch_03.simulate.value | uint |  | 512 |  |   | Количество импульсов в секунду. Симулированное значение
fi4.ch_03.simulate.speed | int |  | 512 |  |   | Количество импульсов в секунду. Скорость изменения симулированного значения
fi4outtype | usint |  | 512 | Yes | 0x80000000 | Выбранный канал для коммутации с выходом


[^simtype]: **Тип симуляции:**<br/>0: Симуляция отключена<br/>1: Константное значение<br/>2: Линейная функция<br/>3: Синусообразная функция<br/>4: Случайное значение<br/><br/>

<p align='right'><a href='index.html'>[Оглавление]</a></p>

