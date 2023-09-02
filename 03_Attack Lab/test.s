# test

0x401968 <test>         sub    $0x8,%rsp                                                                                    │
0x40196c <test+4>       mov    $0x0,%eax                                                                                    │
0x401971 <test+9>       callq  0x4017a8 <getbuf>                                                                            │
0x401976 <test+14>      mov    %eax,%edx                                                                                    │
0x401978 <test+16>      mov    $0x403188,%esi                                                                               │
0x40197d <test+21>      mov    $0x1,%edi                                                                                    │
0x401982 <test+26>      mov    $0x0,%eax                                                                                    │
0x401987 <test+31>      callq  0x400df0 <__printf_chk@plt>                                                                  │
0x40198c <test+36>      add    $0x8,%rsp                                                                                    │
0x401990 <test+40>      retq   

# getbuf

0x4017a8 <getbuf>       sub    $0x28,%rsp                                                                                   │
0x4017ac <getbuf+4>     mov    %rsp,%rdi                                                                                    │
0x4017af <getbuf+7>     callq  0x401a40 <Gets>                                                                              │
0x4017b4 <getbuf+12>    mov    $0x1,%eax                                                                                    │
0x4017b9 <getbuf+17>    add    $0x28,%rsp                                                                                   │
0x4017bd <getbuf+21>    retq       