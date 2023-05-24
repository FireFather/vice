# Vice Chess Engine
This is a fork of the repo for the Vice chess engine programming series on YouTube.

This codebase has added all the new code from all the stale branches Chapter 97 thru Chapter 106
see: https://github.com/bluefeversoft/vice/branches/stale
which includes: global hash table, threading, and SMP (symmetric multiprocessing) via shared hash table (Lazy SMP).

I've added support for a NNUE (halfkp_256x2-32-32) evaluation in the form of an embedded binary file (nn.bin)
via Daniel Shawul's nnue-probe library: https://github.com/dshawul/nnue-probe,
and Dale Weiler's INCBIN utility: https://github.com/graphitemaster/incbin.

The executable has been produced using MYSYS MINGW64...see Makefile (type 'make' to compile.

Any halfkp_256x2-32-32 NNUE can be used...see:
https://tests.stockfishchess.org/nns to use a different net.

Compatible nets start on page 72-73 (approx.) with dates of 21-05-02 22:26:43 or earlier.

The nnue file size must = 20,530 KB (halfkp_256x2-32-32)


## 
You can find the youtube playlist here: [Link to playlist](https://www.youtube.com/playlist?list=PLZ1QII7yudbc-Ky058TEaOstZHVbT-2hg)

**From part 97 onwards, this repo has a branch with the code for that video. For exmaple, part 97 code is on branch Chapter 97.**

## Getting in touch / Contributing

You can find the Discord server here: https://discord.gg/9hCUD4n7R2

For now, please do not open pull requests on the repository, it's an archive for the video series. Get in touch via the Discord.

## What is Vice?

Vice is a **V**ideo **I**nstructional **C**hess **E**ngine, written in C.

It was (is) written as part of the follow along series you YouTube by Bluefever Software.

Vice is deliberately simplified - the code structure is by no means to be considered best practice, rather a quick start into understanding the world of computer chesss engine programming.

## Where can I get the binaries and what is the latest version?

You can find the download to the latest release (Vice 1.1) here:

https://bluefeversoft.com/

## I want to copy it, can I?

You can do whatever you want with the code. A lot of engines have been inspired by Vice - not all give credit. Probably it's better to give credit than not.

## What are the main features?

Vice is simple, with the following features:

- Alpha beta search
- Iterative deepening
- Quiescence search
- Transposition table
  - Always replace
- Polyglot opening books
- MVV/LVA move ordering
- Basic evaluation

## bugs to fix:
- The PickMove function needs a BestScore of -(very low) instead of 0
- Time management for x moves in x minutes causes losses
- ID loop needs to only exit when it has a legal move (i.e done depth 1 at least)

## Who actually did this?

The real person behind the series is Richard Allbert.  
[LinkedIn](www.linkedin.com/in/richard-allbert)

