<p align='right'><a href='index.html'>[Оглавление]</a></p>

# ao4
> 0.19.193.e68c0512
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
type | uint |  | 512 | Yes |   | Тип модуля:<br>0: Модуль не определен<br/>1: Базовый модуль ЦПУ<br/>2: Модуль AI6a<br/>3: Модуль AI6p<br/>4: Модуль FI4<br/>5: Модуль DI8DO8<br/>6: Модуль DI16<br/>7: Модуль DO16<br/>8: Модуль CRM<br/>9: Модуль AO4<br/>
node | uint |  | 512 | Yes |   | Уникальный номер
vendor | udint |  | 512 | Yes |   | Код производителя
productcode | udint |  | 512 | Yes |   | Код устройства
revision | udint |  | 512 | Yes |   | Версия сетевого драйвера
serialnumber | udint |  | 512 | Yes |   | Серийный номер нижнего уровня
can | uint |  | 512 | Yes |   | Состояние обмена данными
firmware | uint |  | 512 | Yes |   | Версия ПО модуля
hardware | uint |  | 512 | Yes |   | Версия модуля
ch_00.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>
ch_00.adc | uint |  | 512 |  | 0x80000000 | Код АЦП
ch_00.current | lreal | мА | 515 | Yes |   | Выдаваемый ток
ch_00.mode | uint |  | 512 | Yes |   | Тип канала:<br/>0: Активный канал<br/>1: Пасивный канал<br/>
ch_00.regime | uint |  | 512 | Yes | 0x80000000 | Режим канала:<br/>0 : код АЦП<br/>1 : значение в мкА
ch_01.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>
ch_01.adc | uint |  | 512 |  | 0x80000000 | Код АЦП
ch_01.current | lreal | мА | 515 | Yes |   | Выдаваемый ток
ch_01.mode | uint |  | 512 | Yes |   | Тип канала:<br/>0: Активный канал<br/>1: Пасивный канал<br/>
ch_01.regime | uint |  | 512 | Yes | 0x80000000 | Режим канала:<br/>0 : код АЦП<br/>1 : значение в мкА
ch_02.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>
ch_02.adc | uint |  | 512 |  | 0x80000000 | Код АЦП
ch_02.current | lreal | мА | 515 | Yes |   | Выдаваемый ток
ch_02.mode | uint |  | 512 | Yes |   | Тип канала:<br/>0: Активный канал<br/>1: Пасивный канал<br/>
ch_02.regime | uint |  | 512 | Yes | 0x80000000 | Режим канала:<br/>0 : код АЦП<br/>1 : значение в мкА
ch_03.setup | uint |  | 512 | Yes |   | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>
ch_03.adc | uint |  | 512 |  | 0x80000000 | Код АЦП
ch_03.current | lreal | мА | 515 | Yes |   | Выдаваемый ток
ch_03.mode | uint |  | 512 | Yes |   | Тип канала:<br/>0: Активный канал<br/>1: Пасивный канал<br/>
ch_03.regime | uint |  | 512 | Yes | 0x80000000 | Режим канала:<br/>0 : код АЦП<br/>1 : значение в мкА


<p align='right'><a href='index.html'>[Оглавление]</a></p>

