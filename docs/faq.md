# FAQ

## What is serialist?

Serialist is a command line tool for generating code to read and write binary formats.

## Why reinvent the wheel?

My wheel is rounder and lighter. Other file formats such as BFL and DFDL are heavy verbose XML based languages. They are hard to read and not a lot of people use them because of that. Serialist is meant to be a very lightweight language and tool that allows everyone to describe and share file formats.

## Why is serialist a command line tool and not a library?

I made serialist for myself mostly in the beginning, and I found that making it a commandline tool is much easier since I would simply use it in the buildscripts of my project to generate files. I don't think it would be much use as a library