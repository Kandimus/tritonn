<p align='right'><a href='index.html'>[Оглавление]</a></p>

# ao4
> 0.19.169.7aedcf1b
## XML
````xml
<module type="ao4" name="valid object name" description="string index" >
	<channel number="0" setup="AO setup flags" />
	<channel number="1" setup="AO setup flags" />
	<channel number="2" setup="AO setup flags" />
	<channel number="3" setup="AO setup flags" />
</module>
````

## Channels
Number | Type | Comment
:-- |:--:|:--
0 | AO | 
1 | AO | 
2 | AO | 
3 | AO | 


#### AOsetup
* _OFF_  - Выключен и не обрабатывается

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:-- |:--
ao4.type | uint |  | 512 | Yes |   | Тип модуля:<br>0: Модуль не определен<br/>1: Базовый модуль ЦПУ<br/>2: Модуль AI6a<br/>3: Модуль AI6p<br/>4: Модуль FI4<br/>5: Модуль DI8DO8<br/>6: Модуль DI16<br/>7: Модуль DO16<br/>8: Модуль CRM<br/>9: Модуль AO4<br/>
ao4.node | uint |  | 512 | Yes |   | Уникальный номер
ao4.vendor | udint |  | 512 | Yes |   | Код производителя
ao4.productcode | udint |  | 512 | Yes |   | Код устройства
ao4.revision | udint |  | 512 | Yes |   | Версия сетевого драйвера
ao4.serialnumber | udint |  | 512 | Yes |   | Серийный номер нижнего уровня
ao4.can | uint |  | 512 | Yes |   | Состояние обмена данными
ao4.firmware | uint |  | 512 | Yes |   | Версия ПО модуля
ao4.hardware | uint |  | 512 | Yes |   | Версия модуля
ao4.ch_00.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>
ao4.ch_00.adc | uint |  | 512 |  |   | Код АЦП
ao4.ch_00.current | lreal | мА | 515 | Yes |   | Выдаваемый ток
ao4.ch_00.mode | uint |  | 512 | Yes |   | Тип канала:<br/>0: Активный канал<br/>1: Пасивный канал<br/>
ao4.ch_00.regime | uint |  | 512 | Yes |   | Режим канала:<br/>0 : код АЦП<br/>1 : значение в мкА
ao4.ch_01.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>
ao4.ch_01.adc | uint |  | 512 |  |   | Код АЦП
ao4.ch_01.current | lreal | мА | 515 | Yes |   | Выдаваемый ток
ao4.ch_01.mode | uint |  | 512 | Yes |   | Тип канала:<br/>0: Активный канал<br/>1: Пасивный канал<br/>
ao4.ch_01.regime | uint |  | 512 | Yes |   | Режим канала:<br/>0 : код АЦП<br/>1 : значение в мкА
ao4.ch_02.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>
ao4.ch_02.adc | uint |  | 512 |  |   | Код АЦП
ao4.ch_02.current | lreal | мА | 515 | Yes |   | Выдаваемый ток
ao4.ch_02.mode | uint |  | 512 | Yes |   | Тип канала:<br/>0: Активный канал<br/>1: Пасивный канал<br/>
ao4.ch_02.regime | uint |  | 512 | Yes |   | Режим канала:<br/>0 : код АЦП<br/>1 : значение в мкА
ao4.ch_03.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>
ao4.ch_03.adc | uint |  | 512 |  |   | Код АЦП
ao4.ch_03.current | lreal | мА | 515 | Yes |   | Выдаваемый ток
ao4.ch_03.mode | uint |  | 512 | Yes |   | Тип канала:<br/>0: Активный канал<br/>1: Пасивный канал<br/>
ao4.ch_03.regime | uint |  | 512 | Yes |   | Режим канала:<br/>0 : код АЦП<br/>1 : значение в мкА


<p align='right'><a href='index.html'>[Оглавление]</a></p>

