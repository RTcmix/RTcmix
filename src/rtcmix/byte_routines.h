#define byte_reverse8(data)                                    \
    { char _c, *_t; _t = (char *) data;                        \
    _c = _t[0]; _t[0] = _t[7]; _t[7] = _c;                     \
    _c = _t[1]; _t[1] = _t[6]; _t[6] = _c;                     \
    _c = _t[2]; _t[2] = _t[5]; _t[5] = _c;                     \
    _c = _t[3]; _t[3] = _t[4]; _t[4] = _c; }

#define byte_reverse4(data)                                    \
    { char _c, *_t; _t = (char *) data;                        \
    _c = _t[0]; _t[0] = _t[3]; _t[3] = _c;                     \
    _c = _t[1]; _t[1] = _t[2]; _t[2] = _c; }

#define byte_reverse2(data)                                    \
    { char _c, *_t; _t = (char *) data;                        \
    _c = _t[0]; _t[0] = _t[1]; _t[1] = _c; }

#define reverse_int4(data)                                     \
    (((*(unsigned long int *) (data) & 0x000000ffU) << 24) |   \
     ((*(unsigned long int *) (data) & 0x0000ff00U) <<  8) |   \
     ((*(unsigned long int *) (data) & 0x00ff0000U) >>  8) |   \
     ((*(unsigned long int *) (data) & 0xff000000U) >> 24))

#define reverse_int2(data)                                     \
    (((*(unsigned short int *) (data) & 0x00ff) << 8) |        \
     ((*(unsigned short int *) (data) & 0xff00) >> 8))







