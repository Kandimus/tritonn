# denssol
## XML
````xml
<denssol name="valid object name" >
	<temperature><link alias="object's output"/></temperature>
	<pressure><link alias="object's output"/></pressure>
	<period><link alias="object's output"/></period>
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
	<limit name="period" setup="LOLO|LO|HI|HIHI">
		<lolo>0</lolo>
		<lo>0</lo>
		<hi>0</hi>
		<hihi>0</hihi>
	</limit>
</limits>
</denssol>
````

## Inputs
Input | Unit | Unit ID | Limits | Shadow | Comment
:-- |:--:|:--:|:--:|:--:|:--
temperature | °C | 16 | LOLO, LO, HI, HIHI |  | 
pressure | бар | 35 | LOLO, LO, HI, HIHI |  | 
period | мкс | 209 | LOLO, LO, HI, HIHI |  | 

## Outputs
Output | Unit | Unit ID | Limits | Comment
:-- |:--:|:--:|:--:|:--
density | кг/м³ | 48 | LOLO, LO, HI, HIHI | 
temperature | °C | 16 | LOLO, LO, HI, HIHI | 
pressure | бар | 35 | LOLO, LO, HI, HIHI | 
density15 | кг/м³ | 48 | LOLO, LO, HI, HIHI | 
density20 | кг/м³ | 48 | LOLO, LO, HI, HIHI | 
b | C¯¹ | 517 | LOLO, LO, HI, HIHI | 
b15 | C¯¹ | 517 | LOLO, LO, HI, HIHI | 
y | МПа¯¹ | 518 | LOLO, LO, HI, HIHI | 
y15 | МПа¯¹ | 518 | LOLO, LO, HI, HIHI | 
ctl |  | 512 | LOLO, LO, HI, HIHI | 
cpl |  | 512 | LOLO, LO, HI, HIHI | 

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:--:|:--
temperature.value | LREAL | °C | 16 | Yes | 0x0 | 
temperature.unit | STRID |  | 512 | Yes | 0x0 | 
temperature.lolo | LREAL | °C | 16 |  | 0x200 | 
temperature.lo | LREAL | °C | 16 |  | 0x200 | 
temperature.hi | LREAL | °C | 16 |  | 0x200 | 
temperature.hihi | LREAL | °C | 16 |  | 0x200 | 
temperature.hysteresis | LREAL | °C | 16 |  | 0x200 | 
temperature.status | UINT |  | 512 | Yes | 0x0 | 
temperature.setup | UINT |  | 512 | Yes | 0x200 | 
pressure.value | LREAL | бар | 35 | Yes | 0x0 | 
pressure.unit | STRID |  | 512 | Yes | 0x0 | 
pressure.lolo | LREAL | бар | 35 |  | 0x200 | 
pressure.lo | LREAL | бар | 35 |  | 0x200 | 
pressure.hi | LREAL | бар | 35 |  | 0x200 | 
pressure.hihi | LREAL | бар | 35 |  | 0x200 | 
pressure.hysteresis | LREAL | бар | 35 |  | 0x200 | 
pressure.status | UINT |  | 512 | Yes | 0x0 | 
pressure.setup | UINT |  | 512 | Yes | 0x200 | 
period.value | LREAL | мкс | 209 | Yes | 0x0 | 
period.unit | STRID |  | 512 | Yes | 0x0 | 
period.lolo | LREAL | мкс | 209 |  | 0x200 | 
period.lo | LREAL | мкс | 209 |  | 0x200 | 
period.hi | LREAL | мкс | 209 |  | 0x200 | 
period.hihi | LREAL | мкс | 209 |  | 0x200 | 
period.hysteresis | LREAL | мкс | 209 |  | 0x200 | 
period.status | UINT |  | 512 | Yes | 0x0 | 
period.setup | UINT |  | 512 | Yes | 0x200 | 
density.value | LREAL | кг/м³ | 48 | Yes | 0x0 | 
density.unit | STRID |  | 512 | Yes | 0x0 | 
density.lolo | LREAL | кг/м³ | 48 |  | 0x200 | 
density.lo | LREAL | кг/м³ | 48 |  | 0x200 | 
density.hi | LREAL | кг/м³ | 48 |  | 0x200 | 
density.hihi | LREAL | кг/м³ | 48 |  | 0x200 | 
density.hysteresis | LREAL | кг/м³ | 48 |  | 0x200 | 
density.status | UINT |  | 512 | Yes | 0x0 | 
density.setup | UINT |  | 512 | Yes | 0x200 | 
density15.value | LREAL | кг/м³ | 48 | Yes | 0x0 | 
density15.unit | STRID |  | 512 | Yes | 0x0 | 
density15.lolo | LREAL | кг/м³ | 48 |  | 0x200 | 
density15.lo | LREAL | кг/м³ | 48 |  | 0x200 | 
density15.hi | LREAL | кг/м³ | 48 |  | 0x200 | 
density15.hihi | LREAL | кг/м³ | 48 |  | 0x200 | 
density15.hysteresis | LREAL | кг/м³ | 48 |  | 0x200 | 
density15.status | UINT |  | 512 | Yes | 0x0 | 
density15.setup | UINT |  | 512 | Yes | 0x200 | 
density20.value | LREAL | кг/м³ | 48 | Yes | 0x0 | 
density20.unit | STRID |  | 512 | Yes | 0x0 | 
density20.lolo | LREAL | кг/м³ | 48 |  | 0x200 | 
density20.lo | LREAL | кг/м³ | 48 |  | 0x200 | 
density20.hi | LREAL | кг/м³ | 48 |  | 0x200 | 
density20.hihi | LREAL | кг/м³ | 48 |  | 0x200 | 
density20.hysteresis | LREAL | кг/м³ | 48 |  | 0x200 | 
density20.status | UINT |  | 512 | Yes | 0x0 | 
density20.setup | UINT |  | 512 | Yes | 0x200 | 
b.value | LREAL | C¯¹ | 517 | Yes | 0x0 | 
b.unit | STRID |  | 512 | Yes | 0x0 | 
b.lolo | LREAL | C¯¹ | 517 |  | 0x200 | 
b.lo | LREAL | C¯¹ | 517 |  | 0x200 | 
b.hi | LREAL | C¯¹ | 517 |  | 0x200 | 
b.hihi | LREAL | C¯¹ | 517 |  | 0x200 | 
b.hysteresis | LREAL | C¯¹ | 517 |  | 0x200 | 
b.status | UINT |  | 512 | Yes | 0x0 | 
b.setup | UINT |  | 512 | Yes | 0x200 | 
b15.value | LREAL | C¯¹ | 517 | Yes | 0x0 | 
b15.unit | STRID |  | 512 | Yes | 0x0 | 
b15.lolo | LREAL | C¯¹ | 517 |  | 0x200 | 
b15.lo | LREAL | C¯¹ | 517 |  | 0x200 | 
b15.hi | LREAL | C¯¹ | 517 |  | 0x200 | 
b15.hihi | LREAL | C¯¹ | 517 |  | 0x200 | 
b15.hysteresis | LREAL | C¯¹ | 517 |  | 0x200 | 
b15.status | UINT |  | 512 | Yes | 0x0 | 
b15.setup | UINT |  | 512 | Yes | 0x200 | 
y.value | LREAL | МПа¯¹ | 518 | Yes | 0x0 | 
y.unit | STRID |  | 512 | Yes | 0x0 | 
y.lolo | LREAL | МПа¯¹ | 518 |  | 0x200 | 
y.lo | LREAL | МПа¯¹ | 518 |  | 0x200 | 
y.hi | LREAL | МПа¯¹ | 518 |  | 0x200 | 
y.hihi | LREAL | МПа¯¹ | 518 |  | 0x200 | 
y.hysteresis | LREAL | МПа¯¹ | 518 |  | 0x200 | 
y.status | UINT |  | 512 | Yes | 0x0 | 
y.setup | UINT |  | 512 | Yes | 0x200 | 
y15.value | LREAL | МПа¯¹ | 518 | Yes | 0x0 | 
y15.unit | STRID |  | 512 | Yes | 0x0 | 
y15.lolo | LREAL | МПа¯¹ | 518 |  | 0x200 | 
y15.lo | LREAL | МПа¯¹ | 518 |  | 0x200 | 
y15.hi | LREAL | МПа¯¹ | 518 |  | 0x200 | 
y15.hihi | LREAL | МПа¯¹ | 518 |  | 0x200 | 
y15.hysteresis | LREAL | МПа¯¹ | 518 |  | 0x200 | 
y15.status | UINT |  | 512 | Yes | 0x0 | 
y15.setup | UINT |  | 512 | Yes | 0x200 | 
ctl.value | LREAL |  | 512 | Yes | 0x0 | 
ctl.unit | STRID |  | 512 | Yes | 0x0 | 
ctl.lolo | LREAL |  | 512 |  | 0x200 | 
ctl.lo | LREAL |  | 512 |  | 0x200 | 
ctl.hi | LREAL |  | 512 |  | 0x200 | 
ctl.hihi | LREAL |  | 512 |  | 0x200 | 
ctl.hysteresis | LREAL |  | 512 |  | 0x200 | 
ctl.status | UINT |  | 512 | Yes | 0x0 | 
ctl.setup | UINT |  | 512 | Yes | 0x200 | 
cpl.value | LREAL |  | 512 | Yes | 0x0 | 
cpl.unit | STRID |  | 512 | Yes | 0x0 | 
cpl.lolo | LREAL |  | 512 |  | 0x200 | 
cpl.lo | LREAL |  | 512 |  | 0x200 | 
cpl.hi | LREAL |  | 512 |  | 0x200 | 
cpl.hihi | LREAL |  | 512 |  | 0x200 | 
cpl.hysteresis | LREAL |  | 512 |  | 0x200 | 
cpl.status | UINT |  | 512 | Yes | 0x0 | 
cpl.setup | UINT |  | 512 | Yes | 0x200 | 
k0 | LREAL |  | 512 | Yes | 0x0 | 
k1 | LREAL |  | 512 | Yes | 0x0 | 
k2 | LREAL |  | 512 | Yes | 0x0 | 
factor.k0 | LREAL |  | 521 | Yes | 0x80000000 | 
factor.k1 | LREAL |  | 521 | Yes | 0x80000000 | 
factor.k2 | LREAL |  | 521 | Yes | 0x80000000 | 
factor.k18 | LREAL |  | 521 | Yes | 0x80000000 | 
factor.k19 | LREAL |  | 521 | Yes | 0x80000000 | 
factor.k20a | LREAL |  | 521 | Yes | 0x80000000 | 
factor.k20b | LREAL |  | 521 | Yes | 0x80000000 | 
factor.k21a | LREAL |  | 521 | Yes | 0x80000000 | 
factor.k21b | LREAL |  | 521 | Yes | 0x80000000 | 
factor.set.k0 | LREAL |  | 521 |  | 0x1000 | 
factor.set.k1 | LREAL |  | 521 |  | 0x1000 | 
factor.set.k2 | LREAL |  | 521 |  | 0x1000 | 
factor.set.k18 | LREAL |  | 521 |  | 0x1000 | 
factor.set.k19 | LREAL |  | 521 |  | 0x1000 | 
factor.set.k20a | LREAL |  | 521 |  | 0x1000 | 
factor.set.k20b | LREAL |  | 521 |  | 0x1000 | 
factor.set.k21a | LREAL |  | 521 |  | 0x1000 | 
factor.set.k21b | LREAL |  | 521 |  | 0x1000 | 
factor.set.accept | USINT |  | 512 |  | 0x1000 | 
calibration | LREAL | °C | 16 |  | 0x1000 | 
setup | UINT |  | 512 | Yes | 0x1000 | 
fault | UDINT |  | 512 | Yes | 0x0 | 

