import gdb

alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789{}_-!"
flag_len = 40
base_arrays = 0x4350a0

gdb.execute("break *0x418143")
gdb.execute("run AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")

for i in range(flag_len):
    arr_ptr_addr = base_arrays + i * 8
    arr_addr = int(gdb.parse_and_eval(f"*(long*){arr_ptr_addr}"))
    
    found = False
    for j, c in enumerate(alphabet):
        func_ptr_addr = arr_addr + j * 8
        func_addr = int(gdb.parse_and_eval(f"*(long*){func_ptr_addr}"))
        
        bytes_at_func = gdb.selected_inferior().read_memory(func_addr, 32).tobytes()
        if b'\xb8\x22\x00\x00\x00' not in bytes_at_func:
            print(f"Position {i}: Candidate {c} (Function {hex(func_addr)})")
    if not found:
        print(f"Position {i}: NOT FOUND")
