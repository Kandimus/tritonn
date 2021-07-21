<p align='right'><a href='index.html'>[Оглавление]</a></p>

# users
> 0.19.156.28f9fa2c
## XML
````xml
<security>
	<!-- binary encode data to text string -->
</security>
````

> Данные кодируются и записываются в текством виде (пример: A0B134FE)

> Формат кодирования: SHA-1</br>Режим: CBC</br>Размер ключа: 128 bits</br>Вектор IV: 0123456789ABCDEF</br>Ключ: 0123456789ABCDEF</br>

## Формат исходных данных
````xml
<?xml version="1.0" encoding="UTF-8"?>
<security>
	<config>
		<password>SHA-1 hash password</password>
	</config>
	<users>
		<user name="user name">
			<password>SHA-1 hash password</password>
			<comms>
				<login>user number name</login>
				<password>SHA-1 hash password</password>
			</comms>
			<rights>
				<internal>internal hex access</internal>
				<external>external hex access</external>
			</rights>
		</user>
		<user>
			<!-- ... -->
		</user>
	</users>
	<opcua>
		<user>
			<login>user name</login>
			<password>uncrypted password</password>
		</user>
	</opcua>
</security>
````


<p align='right'><a href='index.html'>[Оглавление]</a></p>

