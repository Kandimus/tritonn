<p align='right'><a href='index.html'>[Оглавление]</a></p>

# sysvar
> 0.19.33.a2f312a3

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:-- |:--
system.version.major | USINT |  | 512 | Yes | 0x80000000 | Версия ПО
system.version.minor | USINT |  | 512 | Yes | 0x80000000 | Подверсия ПО
system.version.build | UINT |  | 512 | Yes | 0x80000000 | Номер сборки
system.version.hash | UDINT |  | 512 | Yes | 0x80000000 | Контрольная сумма сборки
system.metrology.major | USINT |  | 512 | Yes | 0x80000000 | Версия метрологически значимой части ПО
system.metrology.minor | USINT |  | 512 | Yes | 0x80000000 | Подверсия метрологически значимой части ПО
system.metrology.crc | UDINT |  | 512 | Yes | 0x80000000 | Контрольная сумма метрологически значимой части ПО
system.state.alarm | UDINT |  | 512 | Yes | 0x80000000 | Количество не квитированных аварий
system.state.live | USINT |  | 512 | Yes | 0x80000000 | Статус:<br/>0: Не определенный статус<br/>1: Система загружается<br/>3: Система в режиме COLD-START<br/>4: Система работает<br/>4: Система в режиме HALT<br/>
system.state.rebootreason | USINT |  | 512 | Yes | 0x80000000 | Причина перезагрузки
system.state.simulate | USINT |  | 512 | Yes |   | Флаг симуляции системы
system.datetime.sec | INT |  | 512 | Yes |   | Текущее время. Секунды
system.datetime.min | INT |  | 512 | Yes |   | Текущее время. Минуты
system.datetime.hour | INT |  | 512 | Yes |   | Текущее время. Часы
system.datetime.day | INT |  | 512 | Yes |   | Текущее время. День
system.datetime.month | INT |  | 512 | Yes |   | Текущее время. Месяц
system.datetime.year | INT |  | 512 | Yes |   | Текущее время. Год
system.datetime.set.sec | INT |  | 512 |  | 0x00020000 | Установить время. Секунды
system.datetime.set.min | INT |  | 512 |  | 0x00020000 | Установить время. Минуты
system.datetime.set.hour | INT |  | 512 |  | 0x00020000 | Установить время. Часы
system.datetime.set.day | INT |  | 512 |  | 0x00020000 | Установить время. День
system.datetime.set.month | INT |  | 512 |  | 0x00020000 | Установить время. Месяц
system.datetime.set.year | INT |  | 512 |  | 0x00020000 | Установить время. Год
system.datetime.set.accept | INT |  | 512 |  | 0x00020000 | Команда установить время:<br/>0 - нет действия<br/>1 - применить установленное время


<p align='right'><a href='index.html'>[Оглавление]</a></p>

