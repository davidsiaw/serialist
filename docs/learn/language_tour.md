# Introduction

The Serialist language is used to describe binary file formats in a clear an concise way. It makes creating new file formats or describing existing ones a joy. The syntax is designed to be highly readable yet concise with little room for confusion.

# Format Construct

The core of the Serialist Language is the `format` keyword. It is similar the the syntax of a C struct. This construct is used to describe set of members in order. Every format must have a `Name` that must begin with a capital letter.

This is what it looks like:

```
format Name
{
	Type memberName
	...
}
```

Each member consists of a `Type` and a `memberName`. `Type`s must begin with a capital letter while `memberName`s must begin with a small letter.

Like a C struct, a format can have multiple members and the order of the members in the format describes the order of each element in the struct.

Here is a simple format that describes a binary file format for a configuration file that describes a screen resolution using a 32-bit integer followed by another 32-bit integer:

```
// configuration.format

format Resolution
{
	Int32 width
	Int32 height
}
```

With this we have described a binary file format with two numbers that describe your configuration file. Most binary file formats consist of data that come one after another.

Very much like C structs, formats are also types. The `Name` of a format must begin with a capital letter because it is the name of a `Type`.

This means you can use a format to describe a small section of a larger file format.

The following example shows you how to use a format inside another format:

```
// configuration_v2.format

format Resolution
{
	Int32 width
	Int32 height
}

format Configuration
{
	Resolution resolution
	Int32 version_number
}
```

Now you might be thinking, you might as well write a format like this instead:

```
// configuration_v3.format

format Configuration
{
	Int32 width
	Int32 height
	Int32 version_number
}
```

That is correct, the two `Configuration` formats are equivalent. But say your boss now wants to have multiple resolutions, for say, 4 monitors. This becomes:

```
// configuration_v4.format
format Configuration
{
	Int32 width1
	Int32 height1
	Int32 width2
	Int32 height2
	Int32 width3
	Int32 height3
	Int32 width4
	Int32 height4
	Int32 version_number
}
```

It quickly becomes horrible. Also, if we wanted to add a DPI value to each resolution, the file becomes much more unmanageable:

```
// configuration_v5.format
format Configuration
{
	Int32 width1
	Int32 height1
	Int32 dpi1
	Int32 width2
	Int32 height2
	Int32 dpi2
	Int32 width3
	Int32 height3
	Int32 dpi3
	Int32 width4
	Int32 height4
	Int32 dpi4
	Int32 version_number
}
```

It becomes an eyesore. It is also difficult to see if you made a mistake. By keeping a subformat in a separate description, it becomes much more manageable:

```
// configuration_v6.format

format Resolution
{
	Int32 width
	Int32 height
	Int32 dpi
}

format Configuration
{
	Resolution screen1
	Resolution screen2
	Resolution screen3
	Resolution screen4
	Int32 version_number
}
```

We are still repeating ourselves here. So let us use an Array.

# Array Types

An array type is described using two square braces after the type name, with a number in the braces that describe the size of the array like so:

```
format Name {
	Type[10] array_member_name
}
```

Continuing our development of the configuration file above, we will use arrays to make it so we can describe the resolutions of 4 screens easily:

```
// configuration_v7.format

format Resolution
{
	Int32 width
	Int32 height
	Int32 dpi
}

format Configuration
{
	Resolution[4] screens
	Int32 version_number
}
```

Now our configuration file has an array of 4 Resolution objects followed by a version number.

# Array Sizes

The configuration file we have now uses what we call a "fixed size array". This means the array's size is hardcoded. Obviously, this is not very desirable for extensibility. Eventually your boss tells you he wants the customer to be able to customize the number of screens and have all screens have different resolutions.

But this is easy to do by replacing the number in the square brackets with the name of another member:

```
// configuration_v8.format

format Resolution
{
	Int32 width
	Int32 height
	Int32 dpi
}

format Configuration
{
	Uint32 screen_count
	Resolution[screen_count] screens
	Int32 version_number
}
```

Notice we have added an `unsigned int` member named `screen_count` and we have changed the number `4` in the square brackets to `screen_count`. This means that the size of the `screens` array is determined by the member `screen_count`.

There is a small rule to this. If you refer to `screen_count` in `screens`, it must appear before `screens` because that is the order in the file.

If you do not, we cannot read the `screens` array before `screen_count` because `screen_count` tells us how long it is. Serialist will throw an error if `screen_count` comes after `screens` in this case.

# Null Termination

Now, say the program needs to store a set of numbers.

