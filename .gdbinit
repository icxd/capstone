set auto-load safe-path ~/

target remote localhost:1234
symbol-file target/osdev.sym
b _start
