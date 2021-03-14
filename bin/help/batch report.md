<p align='right'><a href='index.html'>[Оглавление]</a></p>

# report
> 0.19.18.6bb32f40
## XML
````xml
<report name="valid object name" descr="string index" type="text value" >
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
type | UINT |  | 512 | Yes |   | Тип отчета:<br/>0: Переодический отчет<br/>1: Партионный отчет<br/>
archive.load.accept | UINT |  | 512 |  | 0x00000002 | Команда загрузки архивного отчета:<br/>0 - нет действия<br/>1 - загрузить отчет
archive.load.sec | USINT |  | 512 |  | 0x00000002 | Секунды
archive.load.min | USINT |  | 512 |  | 0x00000002 | Минуты
archive.load.hour | USINT |  | 512 |  | 0x00000002 | Часы
archive.load.day | USINT |  | 512 |  | 0x00000002 | День
archive.load.month | USINT |  | 512 |  | 0x00000002 | Месяц
archive.load.year | UINT |  | 512 |  | 0x00000002 | Год
command | USINT |  | 512 |  | 0x00000020 | Команда:<br/>0: Нет действий<br/>1: Запустить<br/>2: Остановить<br/>3: Перезапустить<br/>
status | UINT |  | 512 | Yes |   | Статус:<br/>0: Не запущен<br/>1: Запущен<br/>2: В паузе<br/>3: Заверщен<br/>
present.status | UINT |  | 512 | Yes | 0x80000000 | Статус:<br/>0: Статус неопределен<br/>1: Недействительный<br/>2: Неполный<br/>3: Действительный<br/>4: В работе<br/>
present.datetime.begin.sec | USINT |  | 512 | Yes | 0x80000000 | Секунды
present.datetime.begin.min | USINT |  | 512 | Yes | 0x80000000 | Минуты
present.datetime.begin.hour | USINT |  | 512 | Yes | 0x80000000 | Часы
present.datetime.begin.day | USINT |  | 512 | Yes | 0x80000000 | День
present.datetime.begin.month | USINT |  | 512 | Yes | 0x80000000 | Месяц
present.datetime.begin.year | UINT |  | 512 | Yes | 0x80000000 | Год
present.datetime.end.sec | USINT |  | 512 | Yes | 0x80000000 | Секунды
present.datetime.end.min | USINT |  | 512 | Yes | 0x80000000 | Минуты
present.datetime.end.hour | USINT |  | 512 | Yes | 0x80000000 | Часы
present.datetime.end.day | USINT |  | 512 | Yes | 0x80000000 | День
present.datetime.end.month | USINT |  | 512 | Yes | 0x80000000 | Месяц
present.datetime.end.year | UINT |  | 512 | Yes | 0x80000000 | Год
present.#report_total_1.total.begin.mass | LREAL |  | 0 | Yes | 0x80000000 | Начало. Масса
present.#report_total_1.total.begin.volume | LREAL |  | 0 | Yes | 0x80000000 | Начало. Объем
present.#report_total_1.total.begin.volume15 | LREAL |  | 0 | Yes | 0x80000000 | Начало. Объем при 15 гр.С
present.#report_total_1.total.begin.volume20 | LREAL |  | 0 | Yes | 0x80000000 | Начало. Объем при 20 гр.С
present.#report_total_1.total.end.mass | LREAL |  | 0 | Yes | 0x80000000 | Окончание. Масса
present.#report_total_1.total.end.volume | LREAL |  | 0 | Yes | 0x80000000 | Окончание. Объем
present.#report_total_1.total.end.volume15 | LREAL |  | 0 | Yes | 0x80000000 | Окончание. Объем при 15 гр.С
present.#report_total_1.total.end.volume20 | LREAL |  | 0 | Yes | 0x80000000 | Окончание. Объем при 20 гр.С
present.#report_total_1.#report_avr_item_1 | LREAL |  | 0 | Yes | 0x80000000 | Значение устредняемого параметра
present.#report_total_1.#report_avr_item_2 | LREAL |  | 0 | Yes | 0x80000000 | Значение устредняемого параметра
present.snapshot.#report_item_1 | LREAL |  | 0 | Yes | 0x80000000 | Значение не устредняемого параметра
present.snapshot.#report_item_2 | LREAL |  | 0 | Yes | 0x80000000 | Значение не устредняемого параметра
completed.status | UINT |  | 512 | Yes | 0x80000000 | Статус:<br/>0: Статус неопределен<br/>1: Недействительный<br/>2: Неполный<br/>3: Действительный<br/>4: В работе<br/>
completed.datetime.begin.sec | USINT |  | 512 | Yes | 0x80000000 | Секунды
completed.datetime.begin.min | USINT |  | 512 | Yes | 0x80000000 | Минуты
completed.datetime.begin.hour | USINT |  | 512 | Yes | 0x80000000 | Часы
completed.datetime.begin.day | USINT |  | 512 | Yes | 0x80000000 | День
completed.datetime.begin.month | USINT |  | 512 | Yes | 0x80000000 | Месяц
completed.datetime.begin.year | UINT |  | 512 | Yes | 0x80000000 | Год
completed.datetime.end.sec | USINT |  | 512 | Yes | 0x80000000 | Секунды
completed.datetime.end.min | USINT |  | 512 | Yes | 0x80000000 | Минуты
completed.datetime.end.hour | USINT |  | 512 | Yes | 0x80000000 | Часы
completed.datetime.end.day | USINT |  | 512 | Yes | 0x80000000 | День
completed.datetime.end.month | USINT |  | 512 | Yes | 0x80000000 | Месяц
completed.datetime.end.year | UINT |  | 512 | Yes | 0x80000000 | Год
completed.#report_total_1.total.begin.mass | LREAL |  | 0 | Yes | 0x80000000 | Начало. Масса
completed.#report_total_1.total.begin.volume | LREAL |  | 0 | Yes | 0x80000000 | Начало. Объем
completed.#report_total_1.total.begin.volume15 | LREAL |  | 0 | Yes | 0x80000000 | Начало. Объем при 15 гр.С
completed.#report_total_1.total.begin.volume20 | LREAL |  | 0 | Yes | 0x80000000 | Начало. Объем при 20 гр.С
completed.#report_total_1.total.end.mass | LREAL |  | 0 | Yes | 0x80000000 | Окончание. Масса
completed.#report_total_1.total.end.volume | LREAL |  | 0 | Yes | 0x80000000 | Окончание. Объем
completed.#report_total_1.total.end.volume15 | LREAL |  | 0 | Yes | 0x80000000 | Окончание. Объем при 15 гр.С
completed.#report_total_1.total.end.volume20 | LREAL |  | 0 | Yes | 0x80000000 | Окончание. Объем при 20 гр.С
completed.#report_total_1.#report_avr_item_1 | LREAL |  | 0 | Yes | 0x80000000 | Значение устредняемого параметра
completed.#report_total_1.#report_avr_item_2 | LREAL |  | 0 | Yes | 0x80000000 | Значение устредняемого параметра
completed.snapshot.#report_item_1 | LREAL |  | 0 | Yes | 0x80000000 | Значение не устредняемого параметра
completed.snapshot.#report_item_2 | LREAL |  | 0 | Yes | 0x80000000 | Значение не устредняемого параметра
archive.status | UINT |  | 512 | Yes | 0x80000000 | Статус:<br/>0: Статус неопределен<br/>1: Недействительный<br/>2: Неполный<br/>3: Действительный<br/>4: В работе<br/>
archive.datetime.begin.sec | USINT |  | 512 | Yes | 0x80000000 | Секунды
archive.datetime.begin.min | USINT |  | 512 | Yes | 0x80000000 | Минуты
archive.datetime.begin.hour | USINT |  | 512 | Yes | 0x80000000 | Часы
archive.datetime.begin.day | USINT |  | 512 | Yes | 0x80000000 | День
archive.datetime.begin.month | USINT |  | 512 | Yes | 0x80000000 | Месяц
archive.datetime.begin.year | UINT |  | 512 | Yes | 0x80000000 | Год
archive.datetime.end.sec | USINT |  | 512 | Yes | 0x80000000 | Секунды
archive.datetime.end.min | USINT |  | 512 | Yes | 0x80000000 | Минуты
archive.datetime.end.hour | USINT |  | 512 | Yes | 0x80000000 | Часы
archive.datetime.end.day | USINT |  | 512 | Yes | 0x80000000 | День
archive.datetime.end.month | USINT |  | 512 | Yes | 0x80000000 | Месяц
archive.datetime.end.year | UINT |  | 512 | Yes | 0x80000000 | Год
archive.#report_total_1.total.begin.mass | LREAL |  | 0 | Yes | 0x80000000 | Начало. Масса
archive.#report_total_1.total.begin.volume | LREAL |  | 0 | Yes | 0x80000000 | Начало. Объем
archive.#report_total_1.total.begin.volume15 | LREAL |  | 0 | Yes | 0x80000000 | Начало. Объем при 15 гр.С
archive.#report_total_1.total.begin.volume20 | LREAL |  | 0 | Yes | 0x80000000 | Начало. Объем при 20 гр.С
archive.#report_total_1.total.end.mass | LREAL |  | 0 | Yes | 0x80000000 | Окончание. Масса
archive.#report_total_1.total.end.volume | LREAL |  | 0 | Yes | 0x80000000 | Окончание. Объем
archive.#report_total_1.total.end.volume15 | LREAL |  | 0 | Yes | 0x80000000 | Окончание. Объем при 15 гр.С
archive.#report_total_1.total.end.volume20 | LREAL |  | 0 | Yes | 0x80000000 | Окончание. Объем при 20 гр.С
archive.#report_total_1.#report_avr_item_1 | LREAL |  | 0 | Yes | 0x80000000 | Значение устредняемого параметра
archive.#report_total_1.#report_avr_item_2 | LREAL |  | 0 | Yes | 0x80000000 | Значение устредняемого параметра
archive.snapshot.#report_item_1 | LREAL |  | 0 | Yes | 0x80000000 | Значение не устредняемого параметра
archive.snapshot.#report_item_2 | LREAL |  | 0 | Yes | 0x80000000 | Значение не устредняемого параметра


## XML Dataset
````xml
<datasets>
	<dataset name="valid name of dataset">
		<totals name="valid name" alias="alias of calculate object contain totals">
			<item name="valid name">
				<link alias="object's output"/>
			</item>
			...
			<item name="valid name">
				<link alias="object's output"/>
			</item>
		</totals>
		...
		<totals> <!-- Optional -->
			...
		</totals>
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

[^mutable]: Если объект не привязан к модулю ввода-вывода, то данная переменная будет записываемой.


<p align='right'><a href='index.html'>[Оглавление]</a></p>
