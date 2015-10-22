
Specification
-------------

#At a glance

```
// Specifies valid values for a type
enum DayOfWeek:Uint32
{
	SUNDAY = 0,
	MONDAY = 1,
	TUESDAY = 2,
	WEDNESDAY = 3,
	THURSDAY = 4,
	FRIDAY = 5,
	SUNDAY = 6
}

enum Boolean:Uint8
{
	TRUE = 1,
	FALSE = 0
}

// Inherit enums
enum Tribool:Boolean
{
	MAYBE = 2
}

// Define valid ranges for data
set LowerCase:Uint8
{
	'a' .. 'z'
}

set UpperCase:Uint8
{
	'A' .. 'Z'
}

// Include sets
set Alphabet:Uint8
{
	LowerCase,
	UpperCase
}

set BiggerThanTen:Int8
{
	< -10,
	> 10
}

set AvailablePorts:Uint16
{
	22,
	80,
	443,
	> 32767,
	1024 .. 2048,
}

set Weekend:DayOfWeek
{
	SUNDAY, SATURDAY
}

format IPV4Address
{
	Int8[4] address;
}

format IPV6Address
{
	Int16[8] address;
}

format OtherFormat
{
	Int32 magic = 21412 // Constant. Used by reader for verification, cannot be changed
	Int8[3] magicArray = [ 12, 0x12, 'a' ] // Can also be used to specify array
	Boolean magicBool = TRUE // Use your own values
	Int32 somenum
}

format SomeFormat
{
	Uint32 size
	OtherFormat other
	OtherFormat[size] refOthers
	OtherFormat[10] fixedOthers
	Uint32[size] refUints
	Uint32[25] fixedUints
	Uint8[#0] nullTermArray

	DayOfWeek day
	Boolean isTrue

	IPV4Address ip
}


```

#Basic Types

- `Int8`
- `Int16`
- `Int32`
- `Int64`
- `Uint8`
- `Uint16`
- `Uint32`
- `Uint64`

- `Utf8` - unimplemented
- `Utf16` - unimplemented
- `Utf32` - unimplemented

#Array Spec

- `Type [10]` - constant size
- `Type [size]` - `size` refers to earlier member
- `BasicType [#0]` - only for basic types (zero terminated)
