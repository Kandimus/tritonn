<p align='right'><a href='index.html'>[Оглавление]</a></p>

# sysvar
> 0.19.165.ba8170a6

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:-- |:--
system.version.major | usint |  | 512 | Yes | 0x80000000 | Версия ПО
system.version.minor | usint |  | 512 | Yes | 0x80000000 | Подверсия ПО
system.version.build | uint |  | 512 | Yes | 0x80000000 | Номер сборки
system.version.hash | udint |  | 512 | Yes | 0x80000000 | Контрольная сумма сборки
system.metrology.major | usint |  | 512 | Yes | 0x80000000 | Версия метрологически значимой части ПО
system.metrology.minor | usint |  | 512 | Yes | 0x80000000 | Подверсия метрологически значимой части ПО
system.metrology.crc | udint |  | 512 | Yes | 0x80000000 | Контрольная сумма метрологически значимой части ПО
system.state.alarm | udint |  | 512 | Yes | 0x80000000 | Количество не квитированных аварий
system.state.live | usint |  | 512 | Yes | 0x80000000 | Статус:<br/>0: Не определенный статус<br/>1: Система загружается<br/>2: Система в режиме COLD-START<br/>3: Система в режиме загрузки нарастающих<br/>4: Система в режиме загрузки уставок<br/>5: Система работает<br/>255: Система в режиме HALT<br/>
system.state.rebootreason | usint |  | 512 | Yes | 0x80000000 | Причина перезагрузки
system.state.simulate | usint |  | 512 | Yes |   | Флаг симуляции системы
system.datetime.sec | usint |  | 512 | Yes |   | Текущее время. Секунды
system.datetime.min | usint |  | 512 | Yes |   | Текущее время. Минуты
system.datetime.hour | usint |  | 512 | Yes |   | Текущее время. Часы
system.datetime.day | usint |  | 512 | Yes |   | Текущее время. День
system.datetime.month | usint |  | 512 | Yes |   | Текущее время. Месяц
system.datetime.year | uint |  | 512 | Yes |   | Текущее время. Год
system.datetime.set.sec | usint |  | 512 |  | 0x00020000 | Установить время. Секунды
system.datetime.set.min | usint |  | 512 |  | 0x00020000 | Установить время. Минуты
system.datetime.set.hour | usint |  | 512 |  | 0x00020000 | Установить время. Часы
system.datetime.set.day | usint |  | 512 |  | 0x00020000 | Установить время. День
system.datetime.set.month | usint |  | 512 |  | 0x00020000 | Установить время. Месяц
system.datetime.set.year | uint |  | 512 |  | 0x00020000 | Установить время. Год
system.datetime.set.accept | usint |  | 512 |  | 0x00020000 | Команда установить время:<br/>0 - нет действия<br/>1 - применить установленное время


<p align='right'><a href='index.html'>[Оглавление]</a></p>

