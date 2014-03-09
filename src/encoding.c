// supports non-huffman encoding

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

int encmax(int b){
  switch (b){
  case 1:
    return 0x01;
  case 2:
    return 0x03;
  case 3:
    return 0x07;
  case 4:
    return 0x0f;
  case 5:
    return 0x1f;
  case 6:
    return 0x3f;
  case 7:
    return 0x7f;
  case 8:
    return 0xff;
  default:
    fprintf(stderr, "encmax does not accept %d.\n", b);
    assert(0); // should not happen
    return -1;
  }
}


// prefix_len = leading number of bits for the first octet
// returns number of bytes consumed
// section 4.1.1
int enc_integer(u_int8_t *buf, int buf_len, int64_t value, int prefix_len){
  int max_first = encmax(8-prefix_len);
  int count = 0;
  if (value < max_first){
    buf[count++] |= value;
    return count;
  } else {
    buf[count++] |= max_first;
    value -= max_first;
    while (value > 0x7f && count < buf_len-1){
      buf[count++] = 0x80 | (value % 0x80);
      value /= 0x80;
    }
    if (count >= buf_len-1){
      fprintf(stderr, "enc_interger: interger overflow\n");
      return -1;
    }
    buf[count++] = value;
    return count;
  }
}

// next_read_pp is pointer-pointer to return where to read next
// section 4.1.1
int64_t dec_integer(u_int8_t *data, int data_len, int prefix_len, u_int8_t **next_read_pp){
  int64_t value;
  long m;
  int max_first = encmax(8-prefix_len);
  int count = 0;
  int b;

  value = data[count++] & max_first;
  if (value == max_first){
    m = 0;
    do {
      b = data[count++];
      value += (b & 0x7f) << 7*m;
      m++; 
    } while ((b & 0x80) != 0 && count < data_len && count < 9);
    // 9 * 7 == 63 is the maximum count it can decode
    if (count == 9){
      fprintf(stderr, "dec_integer: integer overflow\n");
      return -1;
    }
  }
  *next_read_pp = data+count;
  return value;
}

// section 4.1.2
int enc_string(u_int8_t *buf, int buf_len, char *s, int s_len){
  int i;
  u_int8_t *wp;
  buf[0] = 0;
  if ((i = enc_integer(buf, buf_len, s_len, 1)) < 0){
    return -1;
  }
  wp = buf + i;
  if (wp + s_len  > buf + buf_len){
    fprintf(stderr, "enc_string: insufficient buffer\n");
    return -1;
  }
  memcpy(wp, s, s_len);// no trailing \0
  return i+s_len;
}

// returns strlen (does not include trailing \0)
// *str_p will point head of string (thus, no huffman)
// *next_p will point head of the next chunk of data
int dec_string_nocopy(u_int8_t *data, int data_len, char **str_p, u_int8_t **next_p){
  int64_t slen;
  u_int8_t *s;
  if ((data[0] & 0x80) == 0x80){
    fprintf(stderr, "dec_string: huffman encoding is not supported\n");
    return -1;
  }

  slen = dec_integer(data, data_len, 1, (u_int8_t**)str_p);
  *next_p = ((u_int8_t*)*str_p) + slen;

  return slen;
}

// buffer will have trailing \0, but it returns string length (\0 excluded)
// section 4.1.2
int dec_string(u_int8_t *data, int data_len, char *buf, int buf_len, u_int8_t **next_p){
  int64_t slen;
  char *str;
  slen = dec_string_nocopy(data, data_len, &str, next_p);
  if (slen > 0){
    if (slen+1 > buf_len){
      fprintf(stderr, "dec_string: insufficient buffer\n");
      return -1;
    }
    memcpy(buf, str, slen);
    buf[slen] = '\0';
  }
  return slen;
}


#ifdef ENCODING_TEST

int main(int argc, char **argv){

  u_int8_t buf[128];
  int buflen = 128;
  char str[128];
  
  u_int8_t *rp;
  int r;
  int64_t v;
  int i;

  fprintf(stderr, "error cases are not yet tested.\n");
  // section 4.1.1.1
  memset(buf, 0, sizeof(buf));
  buf[0] = 0xc0; // 11000000
  r = enc_integer(buf, sizeof(buf), 10, 3);
  assert(r == 1);
  assert(buf[0] == 0xc0 | 0x0a);
  
  v = dec_integer(buf, sizeof(buf), 3, &rp);
  assert(v == 10);
  assert(rp == buf + 1);

  // additional test
  memset(buf, 0, sizeof(buf));
  buf[0] = 0xc0; 
  r = enc_integer(buf, sizeof(buf), 10, 7);
  assert(r == 2);
  assert(buf[0] == 0xc1);
  assert(buf[1] == 0x09);

  v = dec_integer(buf, sizeof(buf), 7, &rp);
  assert(v == 10);
  assert(rp == buf+2);

  // section 4.1.1.2
  memset(buf, 0, sizeof(buf));
  buf[0] = 0xc0;
  r = enc_integer(buf, sizeof(buf), 1337, 3);
  assert(r == 3);
  assert(buf[0] == 0xc0 | 0x1f);
  assert(buf[1] == 0x9a);
  assert(buf[2] == 0x0a);
  
  v = dec_integer(buf, sizeof(buf), 3, &rp);
  assert(v == 1337);
  assert(rp == buf + 3);

  // section 4.1.1.3
  memset(buf, 0, sizeof(buf));
  r = enc_integer(buf, sizeof(buf), 42, 0);
  assert(r == 1);
  assert(buf[0] == 0x2a);

  v = dec_integer(buf, sizeof(buf), 0, &rp);
  assert(v == 42);
  assert(rp == buf + 1);


  for (i = 0; i < 65536; i++){
    buf[0] = 0;
    r = enc_integer(buf, sizeof(buf), i, 0);
    v = dec_integer(buf, sizeof(buf), 0, &rp);
    assert(i == v);
    assert(rp == buf + r);

    buf[0] = 0;
    r = enc_integer(buf, sizeof(buf), i, 7);
    v = dec_integer(buf, sizeof(buf), 7, &rp);
    assert(i == v);
    assert(rp == buf + r);
  }

  // check decoding and encoding of string
  buf[0] = 3;
  buf[1] = 'a';
  buf[2] = 'b';
  buf[3] = 'c';

  r = dec_string(buf, sizeof(buf), str, sizeof(str), &rp);
  assert(r == 3);
  assert(strlen(str) == 3);
  assert(strcmp("abc", str) == 0);
  assert(rp == buf+strlen(str)+1);

  memset(buf, 0xff, sizeof(buf));
  r = enc_string(buf, sizeof(buf), str, strlen(str));
  assert(r == 4);
  assert(buf[0] == 3);
  assert(buf[1] == 'a');
  assert(buf[2] == 'b');
  assert(buf[3] == 'c');
  assert(buf[4] == 0xff);

  fprintf(stderr, "test ok.\n");

  return 0;
  
}
#endif

