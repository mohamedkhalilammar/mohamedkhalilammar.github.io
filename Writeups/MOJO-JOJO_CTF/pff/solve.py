# solve.py for pff challenge (JavaScript reversal part)

def solve_js_part():
    # Extracted from PDF metadata: JiaTan + v0id + 2 pages
    u = "JiaTanv0id2"
    d = [57, 88, 6, 60, 67, 106, 6, 84, 15, 59, 95, 126, 14, 80, 55, 74]
    
    o = "".join(chr(d[i] ^ ord(u[i % len(u)])) for i in range(len(d)))
    print(f"Decrypted string (Part 2 of Flag): {o}")

if __name__ == "__main__":
    solve_js_part()
