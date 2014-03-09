#ifndef ENCODING_H
#define ENCODING_H


// prefix_len = leading number of bits for the first octet
// returns number of bytes consumed
// section 4.1.1
int enc_integer(u_int8_t *buf, int buf_len, int64_t value, int prefix_len);

// next_read_pp is pointer-pointer to return where to read next
// section 4.1.1
int64_t dec_integer(u_int8_t *data, int data_len, int prefix_len, u_int8_t **next_read_pp);

// section 4.1.2
int enc_string(u_int8_t *buf, int buf_len, char *s, int s_len);

// returns strlen (does not include trailing \0)
// *str_p will point head of string (thus, no huffman)
// *next_p will point head of the next chunk of data
int dec_string_nocopy(u_int8_t *data, int data_len, char **str_p, u_int8_t **next_p);

// buffer will have trailing \0, but it returns string length (\0 excluded)
// section 4.1.2
int dec_string(u_int8_t *data, int data_len, char *buf, int buf_len, char **next_p);


#endif
