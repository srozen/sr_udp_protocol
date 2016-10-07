# Selective-Repeat on UDP Protocol

## Implementations steps

1. Build a valid frame
2. Build command-line interactions with getopt()
3. Enable client/server to connect to each other and exchange messages
4. Implement selective-repeat over UDP exchange

## Testing Suite

This project will use [Criterion](https://github.com/Snaipe/Criterion) as a testing suite, enabling us to mock objects and write
elegant and useful tests on the program.

Please install Criterion in order to run tests. Criterion's lib is included like any other C library.