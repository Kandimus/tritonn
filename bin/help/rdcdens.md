# rdcdens
## XML
````xml
<rdcdens name="valid object name" descr="string index" >
	<temperature><link alias="object's output"/></temperature>
	<pressure><link alias="object's output"/></pressure>
	<density15><link alias="object's output"/></density15>
	<b15><link alias="object's output"/></b15><!-- Optional -->
	<limits> <!-- Optional -->
		<limit name="temperature" setup="LOLO|LO|HI|HIHI">
			<lolo>0</lolo>
			<lo>0</lo>
			<hi>0</hi>
			<hihi>0</hihi>
		</limit>
		<limit name="pressure" setup="LOLO|LO|HI|HIHI">
			<lolo>0</lolo>
			<lo>0</lo>
			<hi>0</hi>
			<hihi>0</hihi>
		</limit>
		<limit name="density15" setup="LOLO|LO|HI|HIHI">
			<lolo>0</lolo>
			<lo>0</lo>
			<hi>0</hi>
			<hihi>0</hihi>
		</limit>
		<limit name="b15" setup="LOLO|LO|HI|HIHI">
			<lolo>0</lolo>
			<lo>0</lo>
			<hi>0</hi>
			<hihi>0</hihi>
		</limit>
	</limits>
</rdcdens>
````

## Inputs
Input | Unit | Unit ID | Limits | Shadow | Comment
:-- |:--:|:--:|:--:|:--:|:--
temperature | °C | 16 | LOLO, LO, HI, HIHI |  | 
pressure | МПа | 32 | LOLO, LO, HI, HIHI |  | 
density15 | кг/м³ | 48 | LOLO, LO, HI, HIHI |  | 
b15 | C¯¹ | 517 | LOLO, LO, HI, HIHI | density15 | 

## Outputs
Output | Unit | Unit ID | Limits | Comment
:-- |:--:|:--:|:--:|:--
temperature | °C | 16 | LOLO, LO, HI, HIHI | 
pressure | МПа | 32 | LOLO, LO, HI, HIHI | 
density15 | кг/м³ | 48 | LOLO, LO, HI, HIHI | 
b15 | C¯¹ | 517 | LOLO, LO, HI, HIHI | 
density | кг/м³ | 48 | LOLO, LO, HI, HIHI | 
density20 | кг/м³ | 48 | LOLO, LO, HI, HIHI | 
ctl |  | 512 | LOLO, LO, HI, HIHI | 
cpl |  | 512 | LOLO, LO, HI, HIHI | 
b | C¯¹ | 517 | LOLO, LO, HI, HIHI | 
y | МПа¯¹ | 518 | LOLO, LO, HI, HIHI | 
y15 | МПа¯¹ | 518 | LOLO, LO, HI, HIHI | 

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
density15.value | LREAL | кг/м³ | 48 | Yes | 0x00000000 | 
density15.unit | STRID |  | 512 | Yes | 0x00000000 | 
density15.lolo | LREAL | кг/м³ | 48 |  | 0x00000200 | 
density15.lo | LREAL | кг/м³ | 48 |  | 0x00000200 | 
density15.hi | LREAL | кг/м³ | 48 |  | 0x00000200 | 
density15.hihi | LREAL | кг/м³ | 48 |  | 0x00000200 | 
density15.hysteresis | LREAL | кг/м³ | 48 |  | 0x00000200 | 
density15.status | UINT |  | 512 | Yes | 0x00000000 | 
density15.setup | UINT |  | 512 | Yes | 0x00000200 | 
b15.value | LREAL | C¯¹ | 517 | Yes | 0x00000000 | 
b15.unit | STRID |  | 512 | Yes | 0x00000000 | 
b15.lolo | LREAL | C¯¹ | 517 |  | 0x00000200 | 
b15.lo | LREAL | C¯¹ | 517 |  | 0x00000200 | 
b15.hi | LREAL | C¯¹ | 517 |  | 0x00000200 | 
b15.hihi | LREAL | C¯¹ | 517 |  | 0x00000200 | 
b15.hysteresis | LREAL | C¯¹ | 517 |  | 0x00000200 | 
b15.status | UINT |  | 512 | Yes | 0x00000000 | 
b15.setup | UINT |  | 512 | Yes | 0x00000200 | 
density.value | LREAL | кг/м³ | 48 | Yes | 0x00000000 | 
density.unit | STRID |  | 512 | Yes | 0x00000000 | 
density.lolo | LREAL | кг/м³ | 48 |  | 0x00000200 | 
density.lo | LREAL | кг/м³ | 48 |  | 0x00000200 | 
density.hi | LREAL | кг/м³ | 48 |  | 0x00000200 | 
density.hihi | LREAL | кг/м³ | 48 |  | 0x00000200 | 
density.hysteresis | LREAL | кг/м³ | 48 |  | 0x00000200 | 
density.status | UINT |  | 512 | Yes | 0x00000000 | 
density.setup | UINT |  | 512 | Yes | 0x00000200 | 
density20.value | LREAL | кг/м³ | 48 | Yes | 0x00000000 | 
density20.unit | STRID |  | 512 | Yes | 0x00000000 | 
density20.lolo | LREAL | кг/м³ | 48 |  | 0x00000200 | 
density20.lo | LREAL | кг/м³ | 48 |  | 0x00000200 | 
density20.hi | LREAL | кг/м³ | 48 |  | 0x00000200 | 
density20.hihi | LREAL | кг/м³ | 48 |  | 0x00000200 | 
density20.hysteresis | LREAL | кг/м³ | 48 |  | 0x00000200 | 
density20.status | UINT |  | 512 | Yes | 0x00000000 | 
density20.setup | UINT |  | 512 | Yes | 0x00000200 | 
ctl.value | LREAL |  | 512 | Yes | 0x00000000 | 
ctl.unit | STRID |  | 512 | Yes | 0x00000000 | 
ctl.lolo | LREAL |  | 512 |  | 0x00000200 | 
ctl.lo | LREAL |  | 512 |  | 0x00000200 | 
ctl.hi | LREAL |  | 512 |  | 0x00000200 | 
ctl.hihi | LREAL |  | 512 |  | 0x00000200 | 
ctl.hysteresis | LREAL |  | 512 |  | 0x00000200 | 
ctl.status | UINT |  | 512 | Yes | 0x00000000 | 
ctl.setup | UINT |  | 512 | Yes | 0x00000200 | 
cpl.value | LREAL |  | 512 | Yes | 0x00000000 | 
cpl.unit | STRID |  | 512 | Yes | 0x00000000 | 
cpl.lolo | LREAL |  | 512 |  | 0x00000200 | 
cpl.lo | LREAL |  | 512 |  | 0x00000200 | 
cpl.hi | LREAL |  | 512 |  | 0x00000200 | 
cpl.hihi | LREAL |  | 512 |  | 0x00000200 | 
cpl.hysteresis | LREAL |  | 512 |  | 0x00000200 | 
cpl.status | UINT |  | 512 | Yes | 0x00000000 | 
cpl.setup | UINT |  | 512 | Yes | 0x00000200 | 
b.value | LREAL | C¯¹ | 517 | Yes | 0x00000000 | 
b.unit | STRID |  | 512 | Yes | 0x00000000 | 
b.lolo | LREAL | C¯¹ | 517 |  | 0x00000200 | 
b.lo | LREAL | C¯¹ | 517 |  | 0x00000200 | 
b.hi | LREAL | C¯¹ | 517 |  | 0x00000200 | 
b.hihi | LREAL | C¯¹ | 517 |  | 0x00000200 | 
b.hysteresis | LREAL | C¯¹ | 517 |  | 0x00000200 | 
b.status | UINT |  | 512 | Yes | 0x00000000 | 
b.setup | UINT |  | 512 | Yes | 0x00000200 | 
y.value | LREAL | МПа¯¹ | 518 | Yes | 0x00000000 | 
y.unit | STRID |  | 512 | Yes | 0x00000000 | 
y.lolo | LREAL | МПа¯¹ | 518 |  | 0x00000200 | 
y.lo | LREAL | МПа¯¹ | 518 |  | 0x00000200 | 
y.hi | LREAL | МПа¯¹ | 518 |  | 0x00000200 | 
y.hihi | LREAL | МПа¯¹ | 518 |  | 0x00000200 | 
y.hysteresis | LREAL | МПа¯¹ | 518 |  | 0x00000200 | 
y.status | UINT |  | 512 | Yes | 0x00000000 | 
y.setup | UINT |  | 512 | Yes | 0x00000200 | 
y15.value | LREAL | МПа¯¹ | 518 | Yes | 0x00000000 | 
y15.unit | STRID |  | 512 | Yes | 0x00000000 | 
y15.lolo | LREAL | МПа¯¹ | 518 |  | 0x00000200 | 
y15.lo | LREAL | МПа¯¹ | 518 |  | 0x00000200 | 
y15.hi | LREAL | МПа¯¹ | 518 |  | 0x00000200 | 
y15.hihi | LREAL | МПа¯¹ | 518 |  | 0x00000200 | 
y15.hysteresis | LREAL | МПа¯¹ | 518 |  | 0x00000200 | 
y15.status | UINT |  | 512 | Yes | 0x00000000 | 
y15.setup | UINT |  | 512 | Yes | 0x00000200 | 
fault | UDINT |  | 512 | Yes | 0x00000000 | 

