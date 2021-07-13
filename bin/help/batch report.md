<p align='right'><a href='index.html'>[Оглавление]</a></p>

# report
> 0.19.150.5d5232f4
## XML
````xml
<report name="valid object name" description="string index" type="text value" >
	<storage>31<storage/>
	<dataset>dataset name<dataset/>
</report>
````

#### type
* _PERIODIC_  - Переодический отчет
* _BATCH_  - Партионный отчет

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:-- |:--
type | uint |  | 512 | Yes |   | Тип отчета:<br/>0: Переодический отчет<br/>1: Партионный отчет<br/>
archive.load.accept | usint |  | 512 |  | 0x00000002 | Команда загрузки архивного отчета:<br/>0 - нет действия<br/>1 - загрузить отчет
archive.load.sec | usint |  | 512 |  | 0x00000002 | Секунды
archive.load.min | usint |  | 512 |  | 0x00000002 | Минуты
archive.load.hour | usint |  | 512 |  | 0x00000002 | Часы
archive.load.day | usint |  | 512 |  | 0x00000002 | День
archive.load.month | usint |  | 512 |  | 0x00000002 | Месяц
archive.load.year | uint |  | 512 |  | 0x00000002 | Год
command | usint |  | 512 |  | 0x00000020 | Команда:<br/>0: Нет действий<br/>1: Запустить<br/>2: Остановить<br/>3: Перезапустить<br/>
status | uint |  | 512 | Yes |   | Статус:<br/>0: Не запущен<br/>1: Запущен<br/>2: В паузе<br/>3: Заверщен<br/>
present.status | uint |  | 512 | Yes | 0x80000000 | Статус:<br/>0: Статус неопределен<br/>1: Недействительный<br/>2: Неполный<br/>3: Действительный<br/>4: В работе<br/>
present.datetime.begin.sec | usint |  | 512 | Yes | 0x80000000 | Секунды
present.datetime.begin.min | usint |  | 512 | Yes | 0x80000000 | Минуты
present.datetime.begin.hour | usint |  | 512 | Yes | 0x80000000 | Часы
present.datetime.begin.day | usint |  | 512 | Yes | 0x80000000 | День
present.datetime.begin.month | usint |  | 512 | Yes | 0x80000000 | Месяц
present.datetime.begin.year | uint |  | 512 | Yes | 0x80000000 | Год
present.datetime.end.sec | usint |  | 512 | Yes | 0x80000000 | Секунды
present.datetime.end.min | usint |  | 512 | Yes | 0x80000000 | Минуты
present.datetime.end.hour | usint |  | 512 | Yes | 0x80000000 | Часы
present.datetime.end.day | usint |  | 512 | Yes | 0x80000000 | День
present.datetime.end.month | usint |  | 512 | Yes | 0x80000000 | Месяц
present.datetime.end.year | uint |  | 512 | Yes | 0x80000000 | Год
present.#report_total_1.total.begin.mass | lreal |  | 0 | Yes | 0x80000000 | Начало. Масса
present.#report_total_1.total.begin.volume | lreal |  | 0 | Yes | 0x80000000 | Начало. Объем
present.#report_total_1.total.begin.volume15 | lreal |  | 0 | Yes | 0x80000000 | Начало. Объем при 15 гр.С
present.#report_total_1.total.begin.volume20 | lreal |  | 0 | Yes | 0x80000000 | Начало. Объем при 20 гр.С
present.#report_total_1.total.end.mass | lreal |  | 0 | Yes | 0x80000000 | Окончание. Масса
present.#report_total_1.total.end.volume | lreal |  | 0 | Yes | 0x80000000 | Окончание. Объем
present.#report_total_1.total.end.volume15 | lreal |  | 0 | Yes | 0x80000000 | Окончание. Объем при 15 гр.С
present.#report_total_1.total.end.volume20 | lreal |  | 0 | Yes | 0x80000000 | Окончание. Объем при 20 гр.С
present.#report_total_1.#report_avr_item_1 | lreal |  | 0 | Yes | 0x80000000 | Значение устредняемого параметра
present.#report_total_1.#report_avr_item_2 | lreal |  | 0 | Yes | 0x80000000 | Значение устредняемого параметра
present.snapshot.#report_item_1 | lreal |  | 0 | Yes | 0x80000000 | Значение не устредняемого параметра
present.snapshot.#report_item_2 | lreal |  | 0 | Yes | 0x80000000 | Значение не устредняемого параметра
completed.status | uint |  | 512 | Yes | 0x80000000 | Статус:<br/>0: Статус неопределен<br/>1: Недействительный<br/>2: Неполный<br/>3: Действительный<br/>4: В работе<br/>
completed.datetime.begin.sec | usint |  | 512 | Yes | 0x80000000 | Секунды
completed.datetime.begin.min | usint |  | 512 | Yes | 0x80000000 | Минуты
completed.datetime.begin.hour | usint |  | 512 | Yes | 0x80000000 | Часы
completed.datetime.begin.day | usint |  | 512 | Yes | 0x80000000 | День
completed.datetime.begin.month | usint |  | 512 | Yes | 0x80000000 | Месяц
completed.datetime.begin.year | uint |  | 512 | Yes | 0x80000000 | Год
completed.datetime.end.sec | usint |  | 512 | Yes | 0x80000000 | Секунды
completed.datetime.end.min | usint |  | 512 | Yes | 0x80000000 | Минуты
completed.datetime.end.hour | usint |  | 512 | Yes | 0x80000000 | Часы
completed.datetime.end.day | usint |  | 512 | Yes | 0x80000000 | День
completed.datetime.end.month | usint |  | 512 | Yes | 0x80000000 | Месяц
completed.datetime.end.year | uint |  | 512 | Yes | 0x80000000 | Год
completed.#report_total_1.total.begin.mass | lreal |  | 0 | Yes | 0x80000000 | Начало. Масса
completed.#report_total_1.total.begin.volume | lreal |  | 0 | Yes | 0x80000000 | Начало. Объем
completed.#report_total_1.total.begin.volume15 | lreal |  | 0 | Yes | 0x80000000 | Начало. Объем при 15 гр.С
completed.#report_total_1.total.begin.volume20 | lreal |  | 0 | Yes | 0x80000000 | Начало. Объем при 20 гр.С
completed.#report_total_1.total.end.mass | lreal |  | 0 | Yes | 0x80000000 | Окончание. Масса
completed.#report_total_1.total.end.volume | lreal |  | 0 | Yes | 0x80000000 | Окончание. Объем
completed.#report_total_1.total.end.volume15 | lreal |  | 0 | Yes | 0x80000000 | Окончание. Объем при 15 гр.С
completed.#report_total_1.total.end.volume20 | lreal |  | 0 | Yes | 0x80000000 | Окончание. Объем при 20 гр.С
completed.#report_total_1.#report_avr_item_1 | lreal |  | 0 | Yes | 0x80000000 | Значение устредняемого параметра
completed.#report_total_1.#report_avr_item_2 | lreal |  | 0 | Yes | 0x80000000 | Значение устредняемого параметра
completed.snapshot.#report_item_1 | lreal |  | 0 | Yes | 0x80000000 | Значение не устредняемого параметра
completed.snapshot.#report_item_2 | lreal |  | 0 | Yes | 0x80000000 | Значение не устредняемого параметра
archive.status | uint |  | 512 | Yes | 0x80000000 | Статус:<br/>0: Статус неопределен<br/>1: Недействительный<br/>2: Неполный<br/>3: Действительный<br/>4: В работе<br/>
archive.datetime.begin.sec | usint |  | 512 | Yes | 0x80000000 | Секунды
archive.datetime.begin.min | usint |  | 512 | Yes | 0x80000000 | Минуты
archive.datetime.begin.hour | usint |  | 512 | Yes | 0x80000000 | Часы
archive.datetime.begin.day | usint |  | 512 | Yes | 0x80000000 | День
archive.datetime.begin.month | usint |  | 512 | Yes | 0x80000000 | Месяц
archive.datetime.begin.year | uint |  | 512 | Yes | 0x80000000 | Год
archive.datetime.end.sec | usint |  | 512 | Yes | 0x80000000 | Секунды
archive.datetime.end.min | usint |  | 512 | Yes | 0x80000000 | Минуты
archive.datetime.end.hour | usint |  | 512 | Yes | 0x80000000 | Часы
archive.datetime.end.day | usint |  | 512 | Yes | 0x80000000 | День
archive.datetime.end.month | usint |  | 512 | Yes | 0x80000000 | Месяц
archive.datetime.end.year | uint |  | 512 | Yes | 0x80000000 | Год
archive.#report_total_1.total.begin.mass | lreal |  | 0 | Yes | 0x80000000 | Начало. Масса
archive.#report_total_1.total.begin.volume | lreal |  | 0 | Yes | 0x80000000 | Начало. Объем
archive.#report_total_1.total.begin.volume15 | lreal |  | 0 | Yes | 0x80000000 | Начало. Объем при 15 гр.С
archive.#report_total_1.total.begin.volume20 | lreal |  | 0 | Yes | 0x80000000 | Начало. Объем при 20 гр.С
archive.#report_total_1.total.end.mass | lreal |  | 0 | Yes | 0x80000000 | Окончание. Масса
archive.#report_total_1.total.end.volume | lreal |  | 0 | Yes | 0x80000000 | Окончание. Объем
archive.#report_total_1.total.end.volume15 | lreal |  | 0 | Yes | 0x80000000 | Окончание. Объем при 15 гр.С
archive.#report_total_1.total.end.volume20 | lreal |  | 0 | Yes | 0x80000000 | Окончание. Объем при 20 гр.С
archive.#report_total_1.#report_avr_item_1 | lreal |  | 0 | Yes | 0x80000000 | Значение устредняемого параметра
archive.#report_total_1.#report_avr_item_2 | lreal |  | 0 | Yes | 0x80000000 | Значение устредняемого параметра
archive.snapshot.#report_item_1 | lreal |  | 0 | Yes | 0x80000000 | Значение не устредняемого параметра
archive.snapshot.#report_item_2 | lreal |  | 0 | Yes | 0x80000000 | Значение не устредняемого параметра


## XML Dataset
````xml
<datasets>
	<dataset name="valid name of dataset">
		<fwa name="valid name" alias="alias of calculate object contain totals">
			<item name="valid name">
				<link alias="object's output"/>
			</item>
			...
			<item name="valid name">
				<link alias="object's output"/>
			</item>
		</fwa>
		...
		<fwa> <!-- Optional -->
			...
		</fwa>
		<snapshots> <!-- Optional -->
			<item name="valid name">
				<link alias="object's output"/>
			</item>
			...
			<item name="valid name">
				<link alias="object's output"/>
			</item>
		</snapshots>
	</dataset>
	...
	<dataset name="valid report name"> <!-- Optional -->
		...
	</dataset>
</datasets>
````


<p align='right'><a href='index.html'>[Оглавление]</a></p>

