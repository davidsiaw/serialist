# About

Originally I would make use of C structs to read and write file formats. However, file formats began to become more and more complicated, and eventually I was writing if statements and for loops to read in arrays that were conditional on other things. This led me to write the [FormattedReader](https://github.com/davidsiaw/blueblocks/blob/master/BlueBlocksLib/FileAccess/FormattedReader.cs) and [FormattedWriter](https://github.com/davidsiaw/blueblocks/blob/master/BlueBlocksLib/FileAccess/FormattedWriter.cs) in C# in my [BlueBlocks](https://github.com/davidsiaw/blueblocks) library.

BlueBlocks.FileAccess makes heavy use of the CLI's reflection abilities to describe file formats using classes. However, this is not very portable. While you can use C# on many platforms, it is difficult to get C# to work with say, JavaScript. This made me think of a way to automate file formats more easily.

I came to the conclusion that there needed to be a language to describe file formats and a tool to generate read and write functions for that file format on as many languages as possible.

I investigated many binary file format descriptor languages, but most of them were heavy and unwieldly XML formats where one could make a lot of mistakes and using it is not as easy as it should be. Also a lot of them do not have any tools that were easy to use.

Hence, serialist was born.
