<p align='right'><a href='index.html'>[Оглавление]</a></p>

# tcpslave
> 0.19.193.e68c0512
## XML
````xml
<tcpslave name="valid object name" id="0" count_error="3" security="0" maxclient="8" >
	<host> <!-- Optional -->
		<ip>ip address xx.xx.xx.xx<ip/> <!-- Optional -->
		<port>port <port/> <!-- Optional -->
	</host>
	<whitelist> <!-- Optional -->
		<ip>ip address xx.xx.xx.xx<ip/>
		...
		<ip>ip address xx.xx.xx.xx<ip/>
	</whitelist>
	<swap> <!-- Optional -->
		<byte>1<byte/>
		<word>0<word/>
		<dword>0<dword/>
	</swap>
	<addressmap>
		<addressblock begin="start address" lock="0">datablock name</addressblock>
		...
		<addressblock begin="start address" lock="0">datablock name</addressblock>
	</addressmap>
</tcpslave>
````
## DataBlocks
````xml
<datablocks>
	<datablock name="datablock name">
		<variable>variable alias</variable>
		<variable convert="datatype">variable alias</variable> <!-- convert is optional -->
		...
		<variable>variable alias</variable>
	</datablock>
	...
	<datablock name="datablock name">
		<variable>variable alias</variable>
		<variable convert="datatype">variable alias</variable> <!-- convert is optional -->
		...
		<variable>variable alias</variable>
	</datablock>
</datablocks>
````

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:-- |:--
status | usint |  | 512 | Yes |   | Статус
tx | udint |  | 512 | Yes |   | Байт передано
rx | udint |  | 512 | Yes |   | Байт считано
errorrx | usint |  | 512 | Yes |   | Количество ошибок
clients | udint |  | 512 | Yes |   | Количество подключенных клиентов


<p align='right'><a href='index.html'>[Оглавление]</a></p>

