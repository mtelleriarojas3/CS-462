# CS 462 Capstone Project

This is our CS462 capstone project. Authors are: Marcelo Telleria and Mason Waters

## Included Files

client.cpp, server.cpp, includes.cpp, includes.h, makefile, test.txt

## How to compile and run
On Phoenix 1:
```bash
$ make clean
$ make
```
On Phoenix 1:
```bash
$ ./server
```
On Phoenix 2:
```bash
$ ./client
```

## Contributions
We both contributed a lot of efforts to this project. We worked together when possible and then separately when necessary. We were unable to get GBN working at all but SaW and SR are working pretty flawlessly aside from situational errors, we did not have time to implement those. 

## Known issues
The only known issues we have so far are the obvious GBN not being fully implemented, situational errors not being implemented, and then a few issues with bigger files. The code works very well for smaller files, it transfers the file over with each protocol with ease. But when we tried with the bigger files, it took forever and we were not able to even complete it. Though, we assume it was working correctly for the big files because when attempting to open the received.txt file afterwards, the output looked the same. 