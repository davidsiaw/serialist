# Getting Started

Getting serialist is easy on **Linux**. Simply `sudo apt-get install git build-essential` to be able to get and build serialist. Then scroll down and see how to build it.

If you are using **Mac OS X**, you should have homebrew installed to have git `brew install git`. You also should install XCode and have read and agreed to the license agreement to be able to use the build tools.

If you are using **Windows**, I highly recommend downloading and using [Cygwin](https://cygwin.com). It allows you to use the powerful bash shell on your Windows PC and it is much more sane for development. You will need to install the git and g++ packages.

If you insist on not using a command line on windows, you can use the serialist.sln file to build serialist. It might sometimes be out of date but I do update it every now and then.

Then again, serialist is a commandline tool.

## For Cygwin/Mac OS X/Linux

In order to get serialist, you can either [Download the latest sources](https://github.com/davidsiaw/serialist/archive/master.zip) or if you like the command line, you can clone it using this command:

```bash
git clone --recursive https://github.com/davidsiaw/serialist.git
```

Once you have gotten the sources in a folder, go into the folder and run the build script:

```bash
sh build.sh
make install
```

That's it! You can now use serialist.

# Using Serialist

Simply type serialist into your command line to learn how to use it.

```bash
$ serialist
USAGE: serialist <h|c> <filename>.format
```

Serialist is at its very core a code generator, so if you type

```bash
$ serialist h myformat.format
```

You will have a whole pile of code thrown on your console. This is the header file code for your format. For this to be useful, you will want to pipe the output of serialist into a file like so:

```bash
$ serialist h myformat.format > myformat.h
```

This writes the output (which is code) of serialist into the file myformat.h, which you can now include in your project.

Of course, since it is just the header file, you will also want to generate the c sources:

```bash
$ serialist c myformat.format > myformat.c
```

# Testing Serialist

Serialist comes with a test suite that you can use to check if there are any regressions. Simply type `make check` and the test suite will show any failures if serialist is not performing the way it should.

# Uninstalling

If you don't like serialist you can always go into the serialist folder, build serialist and type `make uninstall`

```bash
sh build.sh
make uninstall
```

This will remove all traces of serialist from your computer.
