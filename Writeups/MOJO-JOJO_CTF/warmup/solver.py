hex_data = "7f31554abb38916c37048de213fbe9ee10fc65642b88ae8647107d729be0399d1fccbaea7b8e31962b248d6bb3f0c60e2f48057a68a8c15567b47f92c369591e5f6cd56e1b3870b637bb2d62ad1069e14f7c7c9a2b2be1864854fd72e3609b3e1facf56a0ad8313b57cb"
data = bytes.fromhex(hex_data)
seed = 0x583a4c6f
out = []
for b in data:
    out.append(chr((seed & 0xff) ^ b))
    seed = (seed * 0x41c64e6d + 0x3039) & 0xffffffff
print("".join(out))
