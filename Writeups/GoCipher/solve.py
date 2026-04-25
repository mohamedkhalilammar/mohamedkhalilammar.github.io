import subprocess
flag = ""
# The characters to try
CHOICES = (
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "'_{}=+-*/!?%&@#^~`|\\:;,.<>[]()"
)

def brute_force_GoCipher_flag():
    global flag
    while not flag.endswith('}'):
        for ch in CHOICES:
            candidate = flag + ch
            print(f"Trying key: {candidate}")
            process = subprocess.Popen(
                [r'.\gocipher.exe'],  
                stdin=subprocess.PIPE,  
                stdout=subprocess.PIPE, 
                stderr=subprocess.PIPE,  
                text=True  
            )
            
            # Pass the candidate as input to GOcIPHER.exe and get the output
            output, error = process.communicate(input=candidate + "\n")  

            output = output.strip()  # Clean up the output
            if "Congratulations" in output:
                print(f"Valid flag found so far: {candidate}")
                flag = candidate
                break  

brute_force_GoCipher_flag()
