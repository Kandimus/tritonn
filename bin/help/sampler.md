# sampler
## XML
````xml
<sampler name="valid object name" method="text's bits" setup="text's bits" >
	totals <!-- Optional -->
	<reserve>sampler object<reserve/> <!-- Optional -->
	<io_start><link alias="object's output"/><io_start/> <!-- Optional -->
	<io_stop><link alias="object's output"/><io_stop/> <!-- Optional -->
	<grab_vol>1<grab_vol/> <!-- Optional -->
	<period>43200<period/> <!-- Optional -->
	<grab_test>100<grab_test/> <!-- Optional -->
	<can_a>
		<overflow><link alias="object's output"/><overflow/> <!-- Optional -->
		<fault><link alias="object's output"/><fault/> <!-- Optional -->
		<weight><link alias="object's output"/><weight/> <!-- Optional -->
		<volume>0<volume/> <!-- Optional -->
	</can_a>
	<can_b>
		<overflow><link alias="object's output"/><overflow/> <!-- Optional -->
		<fault><link alias="object's output"/><fault/> <!-- Optional -->
		<weight><link alias="object's output"/><weight/> <!-- Optional -->
		<volume>0<volume/> <!-- Optional -->
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
* _DUALCAN_  - Использовать два бака пробоотборника
* _AUTOSWITCH_  - При заполнении переходить на другой бак пробоотборника

## Inputs
Input | Unit | Unit ID | Limits | Shadow | Comment
:-- |:--:|:--:|:--:|:--:|:--
can_a.overflow |  | 522 | OFF |  | 
can_a.fault |  | 522 | OFF |  | 
can_a.mass | г | 98 | OFF |  | 
can_b.overflow |  | 522 | OFF |  | 
can_b.fault |  | 522 | OFF |  | 
can_b.mass | г | 98 | OFF |  | 
io_start |  | 522 | OFF |  | 
io_stop |  | 522 | OFF |  | 

## Outputs
Output | Unit | Unit ID | Limits | Comment
:-- |:--:|:--:|:--:|:--
grab |  | 522 | OFF | 
selected |  | 522 | OFF | 

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:-- |:--
can_a.overflow.value | LREAL |  | 522 | Yes | 0x00000000 | 
can_a.overflow.unit | STRID |  | 512 | Yes | 0x00000000 | 
can_a.fault.value | LREAL |  | 522 | Yes | 0x00000000 | 
can_a.fault.unit | STRID |  | 512 | Yes | 0x00000000 | 
can_a.mass.value | LREAL | г | 98 | Yes | 0x00000000 | 
can_a.mass.unit | STRID |  | 512 | Yes | 0x00000000 | 
can_b.overflow.value | LREAL |  | 522 | Yes | 0x00000000 | 
can_b.overflow.unit | STRID |  | 512 | Yes | 0x00000000 | 
can_b.fault.value | LREAL |  | 522 | Yes | 0x00000000 | 
can_b.fault.unit | STRID |  | 512 | Yes | 0x00000000 | 
can_b.mass.value | LREAL | г | 98 | Yes | 0x00000000 | 
can_b.mass.unit | STRID |  | 512 | Yes | 0x00000000 | 
io_start.value | LREAL |  | 522 | Yes | 0x00000000 | 
io_start.unit | STRID |  | 512 | Yes | 0x00000000 | 
io_stop.value | LREAL |  | 522 | Yes | 0x00000000 | 
io_stop.unit | STRID |  | 512 | Yes | 0x00000000 | 
grab.value | LREAL |  | 522 | Yes | 0x00000000 | 
grab.unit | STRID |  | 512 | Yes | 0x00000000 | 
selected.value | LREAL |  | 522 | Yes | 0x00000000 | 
selected.unit | STRID |  | 512 | Yes | 0x00000000 | 
method | UINT |  | 512 |  | 0x00000180 | 
setup | UINT |  | 512 |  | 0x00000180 | 
select | UINT |  | 512 |  | 0x00000180 | 
command | UINT |  | 512 |  | 0x00000180 | 
state | UINT |  | 512 | Yes | 0x00000000 | 
noflow | UINT |  | 512 | Yes | 0x00000000 | 
probe.period | UDINT |  | 512 |  | 0x00000180 | 
probe.volume | LREAL |  | 512 |  | 0x00000180 | 
probe.mass | LREAL |  | 512 |  | 0x00000180 | 
probe.test | UDINT |  | 512 |  | 0x00000180 | 
grab.volume | LREAL | мл | 114 | Yes | 0x00000000 | 
grab.count | UDINT |  | 512 | Yes | 0x00000000 | 
grab.present | UDINT |  | 512 | Yes | 0x00000000 | 
grab.remain | UDINT |  | 512 | Yes | 0x00000000 | 
can.volume | LREAL | мл | 114 |  | 0x00000000 | 
can.present | LREAL | мл | 114 | Yes | 0x00000000 | 
can.remain | LREAL | мл | 114 | Yes | 0x00000000 | 
interval | LREAL |  | 512 | Yes | 0x00000000 | 
time.remain | UDINT | мс | 210 | Yes | 0x00000000 | 
time.start | UDINT | с | 211 | Yes | 0x00000000 | 
can_a.volume | LREAL | мл | 114 |  | 0x00000180 | 
can_b.volume | LREAL | мл | 114 |  | 0x00000180 | 

