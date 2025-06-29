
# e8086

e8086, by the name itself, is a 8086 system emulator written badly in C  

As the description said, this project is NOWHERE to be fully perfect (except if anyone is willing to fix all of its potentital bugs)



## Usage

Clone the repository
```bash
git clone https://github.com/wkokplauo666/e8086
cd e8086
```

Build and run
```bash
make
./bin/e8086 [BINARY_FILE]
```
## Programmer's reference manual
As of today there are only 2 I/O operation to interact with the user, which are:
#### Print a character

```
I/O addresses:
  0x0000-0xffff: CHAR
```

| Field | Size | Direction | Description                |
| :-------- | :------- | :------ |:------------------------- |
| `CHAR` | `byte` | `OUT` | Character to print |

#### Get keypress

```
I/O addresses:
  0x0000-0xffff: CHAR
```

| Field | Size | Direction | Description                |
| :-------- | :------- | :------ |:------------------------- |
| `CHAR` | `byte` | `IN` | Character buffer |

### Examples:

```asm
in al, 0xff ; get keypress, I/O address is arbitrary
mov al, 'H'
out 0xff, al ; prints H
```

## License

[MIT](https://choosealicense.com/licenses/mit/)

