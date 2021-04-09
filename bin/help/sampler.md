<p align='right'><a href='index.html'>[Оглавление]</a></p>

# sampler
> 0.19.64.51a28d15
## XML
````xml
<sampler name="valid object name" description="string index" method="text value | text value | ... | text value" setup="text value | text value | ... | text value" >
	totals <!-- Optional -->
	<reserve>sampler object<reserve/> <!-- Optional -->
	<io_start><link alias="object's output"/><io_start/> <!-- Optional -->
	<io_stop><link alias="object's output"/><io_stop/> <!-- Optional -->
	<grab_vol>1</grab_vol> <!-- Optional -->
	<period>43200<period/> <!-- Optional -->
	<grab_test>100<grab_test/> <!-- Optional -->
	<can_a>
		<overflow><link alias="object's output"/><overflow/> <!-- Optional -->
		<fault><link alias="object's output"/><fault/> <!-- Optional -->
		<weight><link alias="object's output"/><weight/> <!-- Optional -->
		<volume>0</volume> <!-- Optional -->
	</can_a>
	<can_b>
		<overflow><link alias="object's output"/><overflow/> <!-- Optional -->
		<fault><link alias="object's output"/><fault/> <!-- Optional -->
		<weight><link alias="object's output"/><weight/> <!-- Optional -->
		<volume>0</volume> <!-- Optional -->
	</can_b>
</sampler>
````

#### method
* _PERIOD_  - Отбор проб по времени
* _MASS_  - Отбор проб по данным массового расхода
* _VOLUME_  - Отбор проб по данным объемного расхода

#### setup
* _OFF_  - Отключить объект
* _ERR2RESERVE_  - При аварии перейти на резервный пробоотборник
* _FILL2RESERVE_  - При заполнении переходить на резервный пробоотборник
* _SINGLECAN_  - Используется только один бак пробоотборника
* _DUALCAN_  - Использовать две емкости пробоотборника
* _AUTOSWITCH_  - При заполнении переходить на другую емкость пробоотборника

## Inputs
Input | Unit | Unit ID | Limits | Shadow | Comment
:-- |:--:|:--:|:--:|:--:|:--
can_a.overflow |  | 522 | OFF |  | Емкость А. Флаг переполнения
can_a.fault |  | 522 | OFF |  | Емкость А. Флаг ошибки
can_a.mass | г | 98 | OFF |  | Емкость А. Масса
can_b.overflow |  | 522 | OFF |  | Емкость Б. Флаг переполнения
can_b.fault |  | 522 | OFF |  | Емкость Б. Флаг ошибки
can_b.mass | г | 98 | OFF |  | Емкость Б. Масса
io_start |  | 522 | OFF |  | Команда запуск обора дозы
io_stop |  | 522 | OFF |  | Команда прекращение обора дозы

## Outputs
Output | Unit | Unit ID | Limits | Comment
:-- |:--:|:--:|:--:|:--
grab |  | 522 | OFF | Сигнал отбора
selected |  | 522 | OFF | Текущая емкость

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:-- |:--
can_a.overflow.value | LREAL |  | 522 | Yes |   | Емкость А. Флаг переполнения. Текущее значение
can_a.overflow.unit | STRID |  | 512 | Yes |   | Емкость А. Флаг переполнения. Единицы измерения
can_a.fault.value | LREAL |  | 522 | Yes |   | Емкость А. Флаг ошибки. Текущее значение
can_a.fault.unit | STRID |  | 512 | Yes |   | Емкость А. Флаг ошибки. Единицы измерения
can_a.mass.value | LREAL | г | 98 | Yes |   | Емкость А. Масса. Текущее значение
can_a.mass.unit | STRID |  | 512 | Yes |   | Емкость А. Масса. Единицы измерения
can_b.overflow.value | LREAL |  | 522 | Yes |   | Емкость Б. Флаг переполнения. Текущее значение
can_b.overflow.unit | STRID |  | 512 | Yes |   | Емкость Б. Флаг переполнения. Единицы измерения
can_b.fault.value | LREAL |  | 522 | Yes |   | Емкость Б. Флаг ошибки. Текущее значение
can_b.fault.unit | STRID |  | 512 | Yes |   | Емкость Б. Флаг ошибки. Единицы измерения
can_b.mass.value | LREAL | г | 98 | Yes |   | Емкость Б. Масса. Текущее значение
can_b.mass.unit | STRID |  | 512 | Yes |   | Емкость Б. Масса. Единицы измерения
io_start.value | LREAL |  | 522 | Yes |   | Команда запуск обора дозы. Текущее значение
io_start.unit | STRID |  | 512 | Yes |   | Команда запуск обора дозы. Единицы измерения
io_stop.value | LREAL |  | 522 | Yes |   | Команда прекращение обора дозы. Текущее значение
io_stop.unit | STRID |  | 512 | Yes |   | Команда прекращение обора дозы. Единицы измерения
grab.value | LREAL |  | 522 | Yes |   | Сигнал отбора. Текущее значение
grab.unit | STRID |  | 512 | Yes |   | Сигнал отбора. Единицы измерения
selected.value | LREAL |  | 522 | Yes |   | Текущая емкость. Текущее значение
selected.unit | STRID |  | 512 | Yes |   | Текущая емкость. Единицы измерения
method | UINT |  | 512 |  | 0x00000080 | Метод:<br/>0: Отбор проб по времени<br/>1: Отбор проб по данным массового расхода<br/>2: Отбор проб по данным объемного расхода<br/>
setup | UINT |  | 512 |  | 0x00000100 | Настройка:<br/>0x0001: Отключить объект<br/>0x0002: При аварии перейти на резервный пробоотборник<br/>0x0004: При заполнении переходить на резервный пробоотборник<br/>0x0008: Используется только один бак пробоотборника<br/>0x0010: Использовать две емкости пробоотборника<br/>0x0020: При заполнении переходить на другую емкость пробоотборника<br/>
select | UINT |  | 512 |  | 0x00000080 | Выбор бака:<br/>0 - емкость А<br/>1 - емкость Б
command | UINT |  | 512 |  | 0x00000080 | Команда:<br/>0: Нет действий<br/>1: Запустить<br/>2: Остановить<br/>3: Запустить тест пробоотборника<br/>4: Подтверждение аварий<br/>122: Пауза<br/>145: Продолжить<br/>
state | UINT |  | 512 | Yes |   | Статус:<br/>0: Не в работе<br/>1: Запущен тест<br/>2: Отбор по времени<br/>3: Отбор по объему<br/>4: Отбор по массе<br/>5: Пауза<br/>6: Завершение отбора<br/>7: Аварийное состояние<br/>
noflow | UINT |  | 512 | Yes |   | Флаг отсутствия расхода
probe.period | UDINT |  | 512 |  | 0x00000080 | Отбор по времени. Период отбора
probe.volume | LREAL |  | 512 |  | 0x00000080 | Отбор по объему. Требуемый объем для отбора
probe.mass | LREAL |  | 512 |  | 0x00000080 | Отбор по массе. Требуемая масса для отбора
probe.test | UDINT |  | 512 |  | 0x00000100 | Количество тестовых доз
grab.volume | LREAL | мл | 114 | Yes |   | Объем единичной дозы
grab.count | UDINT |  | 512 | Yes |   | Общее количество доз
grab.present | UDINT |  | 512 | Yes |   | Количество отобранных доз
grab.remain | UDINT |  | 512 | Yes |   | Количство оставшихся доз
can.volume | LREAL | мл | 114 |  | 0x00000100 | Требуемый объем емкости
can.present | LREAL | мл | 114 | Yes |   | Заполенный объем емкости
can.remain | LREAL | мл | 114 | Yes |   | Оставшийся для заполнения объем емкости
interval | LREAL |  | 512 | Yes |   | Интервал отбора
time.remain | UDINT | мс | 210 | Yes |   | Оставшееся время обора
time.start | UDINT | с | 211 | Yes |   | Время старта пробоотбора
can_a.volume | LREAL | мл | 114 |  | 0x00000080 | Объем емкости А
can_b.volume | LREAL | мл | 114 |  | 0x00000080 | Объем емкости Б


<p align='right'><a href='index.html'>[Оглавление]</a></p>

