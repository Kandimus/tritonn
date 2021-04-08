<p align='right'><a href='index.html'>[Оглавление]</a></p>

# mselector
> 0.19.62.6eb4308c
## XML
````xml
<mselector name="valid object name" description="string index" setup="text value | text value | ... | text value" mode="text value" select="-1" >
	<names>
		<name>valid output 0 name<name/>
		<name>valid output 1 name<name/>
		<name>valid output 2 name<name/>
		<name>valid output 3 name<name/>
		<name>valid output 4 name<name/>
		<name>valid output 5 name<name/>
		<name>valid output 6 name<name/>
		<name>valid output 7 name<name/>
	</names>
	<inputs>
		<group>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
		</group>
		<group>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
		</group>
		<group>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
		</group>
		<group>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
		</group>
	</inputs>
	<faults>
		<group>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
		</group>
		<group>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
		</group>
		<group>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
		</group>
		<group>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
			<link alias="object's output"/>
		</group>
	</faults>
	<keypads>
		<keypad>
			<unit>0<unit/>
			<value>0</value>
		</keypad>
		<keypad>
			<unit>0<unit/>
			<value>0</value>
		</keypad>
		<keypad>
			<unit>0<unit/>
			<value>0</value>
		</keypad>
		<keypad>
			<unit>0<unit/>
			<value>0</value>
		</keypad>
		<keypad>
			<unit>0<unit/>
			<value>0</value>
		</keypad>
		<keypad>
			<unit>0<unit/>
			<value>0</value>
		</keypad>
		<keypad>
			<unit>0<unit/>
			<value>0</value>
		</keypad>
		<keypad>
			<unit>0<unit/>
			<value>0</value>
		</keypad>
	</keypads>
</mselector>
````

#### setup
* _OFF_  - Выключен и не обрабатывается
* _NOEVENT_  - Запретить выдачу сообщений

#### mode
* _NOCHANGE_  - При аварии не переходить на другой вход
* _ERROR_  - При аварии переходить на аварийное значение
* _PREV_  - При аварии переключить на предыдущий вход
* _NEXT_  - При аварии переключить на следующий вход

## Inputs
Input | Unit | Unit ID | Limits | Shadow | Comment
:-- |:--:|:--:|:--:|:--:|:--
#output_1_name.input_1 |  | 0 | OFF |  | группа 1, входное значение 1
#output_1_name.input_1.fault |  | 522 | OFF | #output_1_name.input_1 | группа 1, флаг ошибки входа 1
#output_1_name.input_2 |  | 0 | OFF |  | группа 1, входное значение 2
#output_1_name.input_2.fault |  | 522 | OFF | #output_1_name.input_2 | группа 1, флаг ошибки входа 2
#output_1_name.input_3 |  | 0 | OFF |  | группа 1, входное значение 3
#output_1_name.input_3.fault |  | 522 | OFF | #output_1_name.input_3 | группа 1, флаг ошибки входа 3
#output_1_name.input_4 |  | 0 | OFF |  | группа 1, входное значение 4
#output_1_name.input_4.fault |  | 522 | OFF | #output_1_name.input_4 | группа 1, флаг ошибки входа 4
#output_2_name.input_1 |  | 0 | OFF |  | группа 2, входное значение 1
#output_2_name.input_1.fault |  | 522 | OFF | #output_2_name.input_1 | группа 2, флаг ошибки входа 1
#output_2_name.input_2 |  | 0 | OFF |  | группа 2, входное значение 2
#output_2_name.input_2.fault |  | 522 | OFF | #output_2_name.input_2 | группа 2, флаг ошибки входа 2
#output_2_name.input_3 |  | 0 | OFF |  | группа 2, входное значение 3
#output_2_name.input_3.fault |  | 522 | OFF | #output_2_name.input_3 | группа 2, флаг ошибки входа 3
#output_2_name.input_4 |  | 0 | OFF |  | группа 2, входное значение 4
#output_2_name.input_4.fault |  | 522 | OFF | #output_2_name.input_4 | группа 2, флаг ошибки входа 4
#output_3_name.input_1 |  | 0 | OFF |  | группа 3, входное значение 1
#output_3_name.input_1.fault |  | 522 | OFF | #output_3_name.input_1 | группа 3, флаг ошибки входа 1
#output_3_name.input_2 |  | 0 | OFF |  | группа 3, входное значение 2
#output_3_name.input_2.fault |  | 522 | OFF | #output_3_name.input_2 | группа 3, флаг ошибки входа 2
#output_3_name.input_3 |  | 0 | OFF |  | группа 3, входное значение 3
#output_3_name.input_3.fault |  | 522 | OFF | #output_3_name.input_3 | группа 3, флаг ошибки входа 3
#output_3_name.input_4 |  | 0 | OFF |  | группа 3, входное значение 4
#output_3_name.input_4.fault |  | 522 | OFF | #output_3_name.input_4 | группа 3, флаг ошибки входа 4
#output_4_name.input_1 |  | 0 | OFF |  | группа 4, входное значение 1
#output_4_name.input_1.fault |  | 522 | OFF | #output_4_name.input_1 | группа 4, флаг ошибки входа 1
#output_4_name.input_2 |  | 0 | OFF |  | группа 4, входное значение 2
#output_4_name.input_2.fault |  | 522 | OFF | #output_4_name.input_2 | группа 4, флаг ошибки входа 2
#output_4_name.input_3 |  | 0 | OFF |  | группа 4, входное значение 3
#output_4_name.input_3.fault |  | 522 | OFF | #output_4_name.input_3 | группа 4, флаг ошибки входа 3
#output_4_name.input_4 |  | 0 | OFF |  | группа 4, входное значение 4
#output_4_name.input_4.fault |  | 522 | OFF | #output_4_name.input_4 | группа 4, флаг ошибки входа 4
#output_5_name.input_1 |  | 0 | OFF |  | группа 5, входное значение 1
#output_5_name.input_1.fault |  | 522 | OFF | #output_5_name.input_1 | группа 5, флаг ошибки входа 1
#output_5_name.input_2 |  | 0 | OFF |  | группа 5, входное значение 2
#output_5_name.input_2.fault |  | 522 | OFF | #output_5_name.input_2 | группа 5, флаг ошибки входа 2
#output_5_name.input_3 |  | 0 | OFF |  | группа 5, входное значение 3
#output_5_name.input_3.fault |  | 522 | OFF | #output_5_name.input_3 | группа 5, флаг ошибки входа 3
#output_5_name.input_4 |  | 0 | OFF |  | группа 5, входное значение 4
#output_5_name.input_4.fault |  | 522 | OFF | #output_5_name.input_4 | группа 5, флаг ошибки входа 4
#output_6_name.input_1 |  | 0 | OFF |  | группа 6, входное значение 1
#output_6_name.input_1.fault |  | 522 | OFF | #output_6_name.input_1 | группа 6, флаг ошибки входа 1
#output_6_name.input_2 |  | 0 | OFF |  | группа 6, входное значение 2
#output_6_name.input_2.fault |  | 522 | OFF | #output_6_name.input_2 | группа 6, флаг ошибки входа 2
#output_6_name.input_3 |  | 0 | OFF |  | группа 6, входное значение 3
#output_6_name.input_3.fault |  | 522 | OFF | #output_6_name.input_3 | группа 6, флаг ошибки входа 3
#output_6_name.input_4 |  | 0 | OFF |  | группа 6, входное значение 4
#output_6_name.input_4.fault |  | 522 | OFF | #output_6_name.input_4 | группа 6, флаг ошибки входа 4
#output_7_name.input_1 |  | 0 | OFF |  | группа 7, входное значение 1
#output_7_name.input_1.fault |  | 522 | OFF | #output_7_name.input_1 | группа 7, флаг ошибки входа 1
#output_7_name.input_2 |  | 0 | OFF |  | группа 7, входное значение 2
#output_7_name.input_2.fault |  | 522 | OFF | #output_7_name.input_2 | группа 7, флаг ошибки входа 2
#output_7_name.input_3 |  | 0 | OFF |  | группа 7, входное значение 3
#output_7_name.input_3.fault |  | 522 | OFF | #output_7_name.input_3 | группа 7, флаг ошибки входа 3
#output_7_name.input_4 |  | 0 | OFF |  | группа 7, входное значение 4
#output_7_name.input_4.fault |  | 522 | OFF | #output_7_name.input_4 | группа 7, флаг ошибки входа 4
#output_8_name.input_1 |  | 0 | OFF |  | группа 8, входное значение 1
#output_8_name.input_1.fault |  | 522 | OFF | #output_8_name.input_1 | группа 8, флаг ошибки входа 1
#output_8_name.input_2 |  | 0 | OFF |  | группа 8, входное значение 2
#output_8_name.input_2.fault |  | 522 | OFF | #output_8_name.input_2 | группа 8, флаг ошибки входа 2
#output_8_name.input_3 |  | 0 | OFF |  | группа 8, входное значение 3
#output_8_name.input_3.fault |  | 522 | OFF | #output_8_name.input_3 | группа 8, флаг ошибки входа 3
#output_8_name.input_4 |  | 0 | OFF |  | группа 8, входное значение 4
#output_8_name.input_4.fault |  | 522 | OFF | #output_8_name.input_4 | группа 8, флаг ошибки входа 4

## Outputs
Output | Unit | Unit ID | Limits | Comment
:-- |:--:|:--:|:--:|:--
#output_1_name |  | 0 | OFF | группа 1, выходное значение
#output_2_name |  | 0 | OFF | группа 2, выходное значение
#output_3_name |  | 0 | OFF | группа 3, выходное значение
#output_4_name |  | 0 | OFF | группа 4, выходное значение
#output_5_name |  | 0 | OFF | группа 5, выходное значение
#output_6_name |  | 0 | OFF | группа 6, выходное значение
#output_7_name |  | 0 | OFF | группа 7, выходное значение
#output_8_name |  | 0 | OFF | группа 8, выходное значение

## Variable
Variable | Type | Unit | Unit ID | Readonly | Access | Comment
:-- |:--:|:--:|:--:|:--:|:-- |:--
#output_1_name.input_1.value | LREAL |  | 0 | Yes |   | группа 1, входное значение 1. Текущее значение
#output_1_name.input_1.unit | STRID |  | 512 | Yes |   | группа 1, входное значение 1. Единицы измерения
#output_1_name.input_1.fault | LREAL |  | 522 | Yes |   | группа 1, флаг ошибки входа 1
#output_1_name.input_2.value | LREAL |  | 0 | Yes |   | группа 1, входное значение 2. Текущее значение
#output_1_name.input_2.unit | STRID |  | 512 | Yes |   | группа 1, входное значение 2. Единицы измерения
#output_1_name.input_2.fault | LREAL |  | 522 | Yes |   | группа 1, флаг ошибки входа 2
#output_1_name.input_3.value | LREAL |  | 0 | Yes |   | группа 1, входное значение 3. Текущее значение
#output_1_name.input_3.unit | STRID |  | 512 | Yes |   | группа 1, входное значение 3. Единицы измерения
#output_1_name.input_3.fault | LREAL |  | 522 | Yes |   | группа 1, флаг ошибки входа 3
#output_1_name.input_4.value | LREAL |  | 0 | Yes |   | группа 1, входное значение 4. Текущее значение
#output_1_name.input_4.unit | STRID |  | 512 | Yes |   | группа 1, входное значение 4. Единицы измерения
#output_1_name.input_4.fault | LREAL |  | 522 | Yes |   | группа 1, флаг ошибки входа 4
#output_2_name.input_1.value | LREAL |  | 0 | Yes |   | группа 2, входное значение 1. Текущее значение
#output_2_name.input_1.unit | STRID |  | 512 | Yes |   | группа 2, входное значение 1. Единицы измерения
#output_2_name.input_1.fault | LREAL |  | 522 | Yes |   | группа 2, флаг ошибки входа 1
#output_2_name.input_2.value | LREAL |  | 0 | Yes |   | группа 2, входное значение 2. Текущее значение
#output_2_name.input_2.unit | STRID |  | 512 | Yes |   | группа 2, входное значение 2. Единицы измерения
#output_2_name.input_2.fault | LREAL |  | 522 | Yes |   | группа 2, флаг ошибки входа 2
#output_2_name.input_3.value | LREAL |  | 0 | Yes |   | группа 2, входное значение 3. Текущее значение
#output_2_name.input_3.unit | STRID |  | 512 | Yes |   | группа 2, входное значение 3. Единицы измерения
#output_2_name.input_3.fault | LREAL |  | 522 | Yes |   | группа 2, флаг ошибки входа 3
#output_2_name.input_4.value | LREAL |  | 0 | Yes |   | группа 2, входное значение 4. Текущее значение
#output_2_name.input_4.unit | STRID |  | 512 | Yes |   | группа 2, входное значение 4. Единицы измерения
#output_2_name.input_4.fault | LREAL |  | 522 | Yes |   | группа 2, флаг ошибки входа 4
#output_3_name.input_1.value | LREAL |  | 0 | Yes |   | группа 3, входное значение 1. Текущее значение
#output_3_name.input_1.unit | STRID |  | 512 | Yes |   | группа 3, входное значение 1. Единицы измерения
#output_3_name.input_1.fault | LREAL |  | 522 | Yes |   | группа 3, флаг ошибки входа 1
#output_3_name.input_2.value | LREAL |  | 0 | Yes |   | группа 3, входное значение 2. Текущее значение
#output_3_name.input_2.unit | STRID |  | 512 | Yes |   | группа 3, входное значение 2. Единицы измерения
#output_3_name.input_2.fault | LREAL |  | 522 | Yes |   | группа 3, флаг ошибки входа 2
#output_3_name.input_3.value | LREAL |  | 0 | Yes |   | группа 3, входное значение 3. Текущее значение
#output_3_name.input_3.unit | STRID |  | 512 | Yes |   | группа 3, входное значение 3. Единицы измерения
#output_3_name.input_3.fault | LREAL |  | 522 | Yes |   | группа 3, флаг ошибки входа 3
#output_3_name.input_4.value | LREAL |  | 0 | Yes |   | группа 3, входное значение 4. Текущее значение
#output_3_name.input_4.unit | STRID |  | 512 | Yes |   | группа 3, входное значение 4. Единицы измерения
#output_3_name.input_4.fault | LREAL |  | 522 | Yes |   | группа 3, флаг ошибки входа 4
#output_4_name.input_1.value | LREAL |  | 0 | Yes |   | группа 4, входное значение 1. Текущее значение
#output_4_name.input_1.unit | STRID |  | 512 | Yes |   | группа 4, входное значение 1. Единицы измерения
#output_4_name.input_1.fault | LREAL |  | 522 | Yes |   | группа 4, флаг ошибки входа 1
#output_4_name.input_2.value | LREAL |  | 0 | Yes |   | группа 4, входное значение 2. Текущее значение
#output_4_name.input_2.unit | STRID |  | 512 | Yes |   | группа 4, входное значение 2. Единицы измерения
#output_4_name.input_2.fault | LREAL |  | 522 | Yes |   | группа 4, флаг ошибки входа 2
#output_4_name.input_3.value | LREAL |  | 0 | Yes |   | группа 4, входное значение 3. Текущее значение
#output_4_name.input_3.unit | STRID |  | 512 | Yes |   | группа 4, входное значение 3. Единицы измерения
#output_4_name.input_3.fault | LREAL |  | 522 | Yes |   | группа 4, флаг ошибки входа 3
#output_4_name.input_4.value | LREAL |  | 0 | Yes |   | группа 4, входное значение 4. Текущее значение
#output_4_name.input_4.unit | STRID |  | 512 | Yes |   | группа 4, входное значение 4. Единицы измерения
#output_4_name.input_4.fault | LREAL |  | 522 | Yes |   | группа 4, флаг ошибки входа 4
#output_5_name.input_1.value | LREAL |  | 0 | Yes |   | группа 5, входное значение 1. Текущее значение
#output_5_name.input_1.unit | STRID |  | 512 | Yes |   | группа 5, входное значение 1. Единицы измерения
#output_5_name.input_1.fault | LREAL |  | 522 | Yes |   | группа 5, флаг ошибки входа 1
#output_5_name.input_2.value | LREAL |  | 0 | Yes |   | группа 5, входное значение 2. Текущее значение
#output_5_name.input_2.unit | STRID |  | 512 | Yes |   | группа 5, входное значение 2. Единицы измерения
#output_5_name.input_2.fault | LREAL |  | 522 | Yes |   | группа 5, флаг ошибки входа 2
#output_5_name.input_3.value | LREAL |  | 0 | Yes |   | группа 5, входное значение 3. Текущее значение
#output_5_name.input_3.unit | STRID |  | 512 | Yes |   | группа 5, входное значение 3. Единицы измерения
#output_5_name.input_3.fault | LREAL |  | 522 | Yes |   | группа 5, флаг ошибки входа 3
#output_5_name.input_4.value | LREAL |  | 0 | Yes |   | группа 5, входное значение 4. Текущее значение
#output_5_name.input_4.unit | STRID |  | 512 | Yes |   | группа 5, входное значение 4. Единицы измерения
#output_5_name.input_4.fault | LREAL |  | 522 | Yes |   | группа 5, флаг ошибки входа 4
#output_6_name.input_1.value | LREAL |  | 0 | Yes |   | группа 6, входное значение 1. Текущее значение
#output_6_name.input_1.unit | STRID |  | 512 | Yes |   | группа 6, входное значение 1. Единицы измерения
#output_6_name.input_1.fault | LREAL |  | 522 | Yes |   | группа 6, флаг ошибки входа 1
#output_6_name.input_2.value | LREAL |  | 0 | Yes |   | группа 6, входное значение 2. Текущее значение
#output_6_name.input_2.unit | STRID |  | 512 | Yes |   | группа 6, входное значение 2. Единицы измерения
#output_6_name.input_2.fault | LREAL |  | 522 | Yes |   | группа 6, флаг ошибки входа 2
#output_6_name.input_3.value | LREAL |  | 0 | Yes |   | группа 6, входное значение 3. Текущее значение
#output_6_name.input_3.unit | STRID |  | 512 | Yes |   | группа 6, входное значение 3. Единицы измерения
#output_6_name.input_3.fault | LREAL |  | 522 | Yes |   | группа 6, флаг ошибки входа 3
#output_6_name.input_4.value | LREAL |  | 0 | Yes |   | группа 6, входное значение 4. Текущее значение
#output_6_name.input_4.unit | STRID |  | 512 | Yes |   | группа 6, входное значение 4. Единицы измерения
#output_6_name.input_4.fault | LREAL |  | 522 | Yes |   | группа 6, флаг ошибки входа 4
#output_7_name.input_1.value | LREAL |  | 0 | Yes |   | группа 7, входное значение 1. Текущее значение
#output_7_name.input_1.unit | STRID |  | 512 | Yes |   | группа 7, входное значение 1. Единицы измерения
#output_7_name.input_1.fault | LREAL |  | 522 | Yes |   | группа 7, флаг ошибки входа 1
#output_7_name.input_2.value | LREAL |  | 0 | Yes |   | группа 7, входное значение 2. Текущее значение
#output_7_name.input_2.unit | STRID |  | 512 | Yes |   | группа 7, входное значение 2. Единицы измерения
#output_7_name.input_2.fault | LREAL |  | 522 | Yes |   | группа 7, флаг ошибки входа 2
#output_7_name.input_3.value | LREAL |  | 0 | Yes |   | группа 7, входное значение 3. Текущее значение
#output_7_name.input_3.unit | STRID |  | 512 | Yes |   | группа 7, входное значение 3. Единицы измерения
#output_7_name.input_3.fault | LREAL |  | 522 | Yes |   | группа 7, флаг ошибки входа 3
#output_7_name.input_4.value | LREAL |  | 0 | Yes |   | группа 7, входное значение 4. Текущее значение
#output_7_name.input_4.unit | STRID |  | 512 | Yes |   | группа 7, входное значение 4. Единицы измерения
#output_7_name.input_4.fault | LREAL |  | 522 | Yes |   | группа 7, флаг ошибки входа 4
#output_8_name.input_1.value | LREAL |  | 0 | Yes |   | группа 8, входное значение 1. Текущее значение
#output_8_name.input_1.unit | STRID |  | 512 | Yes |   | группа 8, входное значение 1. Единицы измерения
#output_8_name.input_1.fault | LREAL |  | 522 | Yes |   | группа 8, флаг ошибки входа 1
#output_8_name.input_2.value | LREAL |  | 0 | Yes |   | группа 8, входное значение 2. Текущее значение
#output_8_name.input_2.unit | STRID |  | 512 | Yes |   | группа 8, входное значение 2. Единицы измерения
#output_8_name.input_2.fault | LREAL |  | 522 | Yes |   | группа 8, флаг ошибки входа 2
#output_8_name.input_3.value | LREAL |  | 0 | Yes |   | группа 8, входное значение 3. Текущее значение
#output_8_name.input_3.unit | STRID |  | 512 | Yes |   | группа 8, входное значение 3. Единицы измерения
#output_8_name.input_3.fault | LREAL |  | 522 | Yes |   | группа 8, флаг ошибки входа 3
#output_8_name.input_4.value | LREAL |  | 0 | Yes |   | группа 8, входное значение 4. Текущее значение
#output_8_name.input_4.unit | STRID |  | 512 | Yes |   | группа 8, входное значение 4. Единицы измерения
#output_8_name.input_4.fault | LREAL |  | 522 | Yes |   | группа 8, флаг ошибки входа 4
#output_1_name.output.value | LREAL |  | 0 | Yes |   | группа 1, выходное значение. Текущее значение
#output_1_name.output.unit | STRID |  | 512 | Yes |   | группа 1, выходное значение. Единицы измерения
#output_2_name.output.value | LREAL |  | 0 | Yes |   | группа 2, выходное значение. Текущее значение
#output_2_name.output.unit | STRID |  | 512 | Yes |   | группа 2, выходное значение. Единицы измерения
#output_3_name.output.value | LREAL |  | 0 | Yes |   | группа 3, выходное значение. Текущее значение
#output_3_name.output.unit | STRID |  | 512 | Yes |   | группа 3, выходное значение. Единицы измерения
#output_4_name.output.value | LREAL |  | 0 | Yes |   | группа 4, выходное значение. Текущее значение
#output_4_name.output.unit | STRID |  | 512 | Yes |   | группа 4, выходное значение. Единицы измерения
#output_5_name.output.value | LREAL |  | 0 | Yes |   | группа 5, выходное значение. Текущее значение
#output_5_name.output.unit | STRID |  | 512 | Yes |   | группа 5, выходное значение. Единицы измерения
#output_6_name.output.value | LREAL |  | 0 | Yes |   | группа 6, выходное значение. Текущее значение
#output_6_name.output.unit | STRID |  | 512 | Yes |   | группа 6, выходное значение. Единицы измерения
#output_7_name.output.value | LREAL |  | 0 | Yes |   | группа 7, выходное значение. Текущее значение
#output_7_name.output.unit | STRID |  | 512 | Yes |   | группа 7, выходное значение. Единицы измерения
#output_8_name.output.value | LREAL |  | 0 | Yes |   | группа 8, выходное значение. Текущее значение
#output_8_name.output.unit | STRID |  | 512 | Yes |   | группа 8, выходное значение. Единицы измерения
select | INT |  | 512 |  | 0x00004000 | Выбор коммуцируемого входа
inputcount | UINT |  | 512 | Yes |   | Количество подключенных входов
setup | UINT |  | 512 | Yes | 0x80000000 | Настройка:<br/>0x0001: Выключен и не обрабатывается<br/>0x0004: Запретить выдачу сообщений<br/>
mode | UINT |  | 512 |  | 0x00004000 | Режим:<br/>1: При аварии не переходить на другой вход<br/>2: При аварии переходить на аварийное значение<br/>3: При аварии переключить на предыдущий вход<br/>4: При аварии переключить на следующий вход<br/>
fault | UDINT |  | 512 | Yes |   | Флаг ошибки
selectorcount | UINT |  | 512 | Yes |   | Количество групп
#output_1_name.keypad.unit | UDINT |  | 512 | Yes |   | Группа 0. Значение ручного ввода. Единицы измерения
#output_1_name.keypad.value | LREAL |  | 0 |  | 0x00000800 | Группа 0. Значение ручного ввода
#output_2_name.keypad.unit | UDINT |  | 512 | Yes |   | Группа 1. Значение ручного ввода. Единицы измерения
#output_2_name.keypad.value | LREAL |  | 0 |  | 0x00000800 | Группа 1. Значение ручного ввода
#output_3_name.keypad.unit | UDINT |  | 512 | Yes |   | Группа 2. Значение ручного ввода. Единицы измерения
#output_3_name.keypad.value | LREAL |  | 0 |  | 0x00000800 | Группа 2. Значение ручного ввода
#output_4_name.keypad.unit | UDINT |  | 512 | Yes |   | Группа 3. Значение ручного ввода. Единицы измерения
#output_4_name.keypad.value | LREAL |  | 0 |  | 0x00000800 | Группа 3. Значение ручного ввода
#output_5_name.keypad.unit | UDINT |  | 512 | Yes |   | Группа 4. Значение ручного ввода. Единицы измерения
#output_5_name.keypad.value | LREAL |  | 0 |  | 0x00000800 | Группа 4. Значение ручного ввода
#output_6_name.keypad.unit | UDINT |  | 512 | Yes |   | Группа 5. Значение ручного ввода. Единицы измерения
#output_6_name.keypad.value | LREAL |  | 0 |  | 0x00000800 | Группа 5. Значение ручного ввода
#output_7_name.keypad.unit | UDINT |  | 512 | Yes |   | Группа 6. Значение ручного ввода. Единицы измерения
#output_7_name.keypad.value | LREAL |  | 0 |  | 0x00000800 | Группа 6. Значение ручного ввода
#output_8_name.keypad.unit | UDINT |  | 512 | Yes |   | Группа 7. Значение ручного ввода. Единицы измерения
#output_8_name.keypad.value | LREAL |  | 0 |  | 0x00000800 | Группа 7. Значение ручного ввода


В каждой группе количество входов должно совпадать!

<p align='right'><a href='index.html'>[Оглавление]</a></p>

