# Error Correction Coding for Data Transmission

This project was developed as part of studies at TUL and is intended for educational purposes. The goal is to implement a system for encoding and error correction in data transmission. The program allows encoding and decoding of files, utilizing single and double-bit error correction for 8-bit messages (1 byte).
## Usage

The program runs from the command line and accepts the following arguments:

```
./program -f <filename> -e   # Encode file
./program -f <filename> -d   # Decode file
```

## Example

Initial file: `input.txt`
```
World!!
```
Encoded file: `e_input.txt`
```
W$osr�lOd%!�!�
```
Error included in the encoded file: `e_input.txt`
```
T$osr�lOd%!�!�
```
Decoded file: `d_input.txt`
```
World!!
```

