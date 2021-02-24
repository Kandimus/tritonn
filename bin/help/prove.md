# prove
## XML
````xml
<prove name="valid object name" descr="string index" setup="text's bits" >
	<io_link module="module index"/>
	<temperature><link alias="object's output"/><temperature/> <!-- Optional -->
	<pressure><link alias="object's output"/><pressure/> <!-- Optional -->
	<density><link alias="object's output"/><density/> <!-- Optional -->
	<valve>
		<opened><link alias="object's output"/><opened/>
		<closed><link alias="object's output"/><closed/>
	</valve>
</prove>
````

#### setup
* _NONE_  - Не использовать флаги настройки
* _4WAY_  - ПУ использует четырех ходовой кран
* _STABILIZATION_  - Перед измерением проверять параметры на стабильность
* _NOVALVE_  - ПУ ручным краном
* _ONEDETECTOR_  - Используется один детектор
* _BOUNCE_  - Фильтрация дребезга детекторов
* _SIMULATE_  - Симуляция крана

## Inputs
Input | Unit | Unit ID | Limits | Shadow | Comment
:-- |:--:|:--:|:--:|:--:|:--
temperature | °C | 16 | LOLO, LO, HI, HIHI |  | Температура в ПУ
pressure | МПа | 32 | LOLO, LO, HI, HIHI |  | Давление в ПУ
density | кг/м³ | 48 | LOLO, LO, HI, HIHI |  | Плотность в ПУ
opened |  | 522 | OFF |  | Сигнал кран открыт
closed |  | 522 | OFF |  | Сигнал кран закрыт

## Outputs
Output | Unit | Unit ID | Limits | Comment
:-- |:--:|:--:|:--:|:--
open |  | 522 | OFF | Команда на открытие крана
close |  | 522 | OFF | Команда на закрытие крана

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:-- |:--
temperature.value | LREAL | °C | 16 | Yes | 0x00000000 | 
temperature.unit | STRID |  | 512 | Yes | 0x00000000 | 
temperature.lolo | LREAL | °C | 16 |  | 0x00000200 | 
temperature.lo | LREAL | °C | 16 |  | 0x00000200 | 
temperature.hi | LREAL | °C | 16 |  | 0x00000200 | 
temperature.hihi | LREAL | °C | 16 |  | 0x00000200 | 
temperature.hysteresis | LREAL | °C | 16 |  | 0x00000200 | 
temperature.status | UINT |  | 512 | Yes | 0x00000000 | 
temperature.setup | UINT |  | 512 | Yes | 0x00000200 | 
pressure.value | LREAL | МПа | 32 | Yes | 0x00000000 | 
pressure.unit | STRID |  | 512 | Yes | 0x00000000 | 
pressure.lolo | LREAL | МПа | 32 |  | 0x00000200 | 
pressure.lo | LREAL | МПа | 32 |  | 0x00000200 | 
pressure.hi | LREAL | МПа | 32 |  | 0x00000200 | 
pressure.hihi | LREAL | МПа | 32 |  | 0x00000200 | 
pressure.hysteresis | LREAL | МПа | 32 |  | 0x00000200 | 
pressure.status | UINT |  | 512 | Yes | 0x00000000 | 
pressure.setup | UINT |  | 512 | Yes | 0x00000200 | 
density.value | LREAL | кг/м³ | 48 | Yes | 0x00000000 | 
density.unit | STRID |  | 512 | Yes | 0x00000000 | 
density.lolo | LREAL | кг/м³ | 48 |  | 0x00000200 | 
density.lo | LREAL | кг/м³ | 48 |  | 0x00000200 | 
density.hi | LREAL | кг/м³ | 48 |  | 0x00000200 | 
density.hihi | LREAL | кг/м³ | 48 |  | 0x00000200 | 
density.hysteresis | LREAL | кг/м³ | 48 |  | 0x00000200 | 
density.status | UINT |  | 512 | Yes | 0x00000000 | 
density.setup | UINT |  | 512 | Yes | 0x00000200 | 
opened.value | LREAL |  | 522 | Yes | 0x00000000 | 
opened.unit | STRID |  | 512 | Yes | 0x00000000 | 
closed.value | LREAL |  | 522 | Yes | 0x00000000 | 
closed.unit | STRID |  | 512 | Yes | 0x00000000 | 
open.value | LREAL |  | 522 | Yes | 0x00000000 | 
open.unit | STRID |  | 512 | Yes | 0x00000000 | 
close.value | LREAL |  | 522 | Yes | 0x00000000 | 
close.unit | STRID |  | 512 | Yes | 0x00000000 | 
command | UINT |  | 512 |  | 0x00008000 | 
setup | UINT |  | 512 |  | 0x00008000 | 
state | UINT |  | 512 | Yes | 0x00000000 | 
timer.start | UDINT | мс | 210 |  | 0x00008000 | 
timer.stabilization | UDINT | мс | 210 |  | 0x00008000 | 
timer.detector1 | UDINT | мс | 210 |  | 0x00008000 | 
timer.detector2 | UDINT | мс | 210 |  | 0x00008000 | 
timer.volume | UDINT | мс | 210 |  | 0x00008000 | 
timer.valve | UDINT | мс | 210 |  | 0x00008000 | 
timer.bounce | UDINT | мс | 210 |  | 0x00008000 | 
result.volume1.count | LREAL | имп | 514 | Yes | 0x00000000 | 
result.volume1.time | LREAL | с | 211 | Yes | 0x00000000 | 
result.volume2.count | LREAL | имп | 514 | Yes | 0x00000000 | 
result.volume2.time | LREAL | с | 211 | Yes | 0x00000000 | 
result.prove.frequency | LREAL | Гц | 192 | Yes | 0x00000000 | 
result.prove.temperature | LREAL | °C | 16 | Yes | 0x00000000 | 
result.prove.pressure | LREAL | МПа | 32 | Yes | 0x00000000 | 
result.prove.density | LREAL | кг/м³ | 48 | Yes | 0x00000000 | 
result.stream.temperature | LREAL | °C | 16 | Yes | 0x00000000 | 
result.stream.pressure | LREAL | МПа | 32 | Yes | 0x00000000 | 
result.stream.density | LREAL | кг/м³ | 48 | Yes | 0x00000000 | 
detectors.present | UINT |  | 512 | Yes | 0x00000000 | 
detectors.fixed | UINT |  | 512 | Yes | 0x00000000 | 
stabilization.temperature | LREAL | °C | 16 |  | 0x00008000 | 
stabilization.pressure | LREAL | МПа | 32 |  | 0x00008000 | 
stabilization.density | LREAL | кг/м³ | 48 |  | 0x00008000 | 
stabilization.frequency | LREAL | Гц | 192 |  | 0x00008000 | 
fault | UDINT |  | 512 | Yes | 0x00000000 | 

