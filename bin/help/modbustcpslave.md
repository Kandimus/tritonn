<p align='right'><a href='index.html'>[Оглавление]</a></p>

# modbustcpslave
> 0.19.108.53954df2
## XML
````xml
<modbustcpslave name="valid object name" id="0" port="502" count_error="3" security="0" maxclient="8" >
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
		<addressblock begin="start address">datablock name</addressblock>
		...
		<addressblock begin="start address">datablock name</addressblock>
	</addressmap>
</modbustcpslave>
````
## DataBlocks
````xml
<datablocks>
	<datablock name="datablock name">
		<variable>variable alias</variable>
		<variable>variable alias</variable>
		...
		<variable>variable alias</variable>
	</datablock>
	...
	<datablock name="datablock name">
		<variable>variable alias</variable>
		<variable>variable alias</variable>
		...
		<variable>variable alias</variable>
	</datablock>
</datablocks>
````

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:-- |:--
status | UINT |  | 512 | Yes |   | Статус
tx | UDINT |  | 512 | Yes |   | Байт передано
rx | UDINT |  | 512 | Yes |   | Байт считано
errorrx | USINT |  | 512 | Yes |   | Количество ошибок
clients | USINT |  | 512 | Yes |   | Количество подключенных клиентов


<p align='right'><a href='index.html'>[Оглавление]</a></p>

