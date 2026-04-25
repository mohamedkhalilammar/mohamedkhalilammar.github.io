# Mojo-Vault
* ***Category : Reverse Engineering / Bash***  
 >   Description : **Lock up your secrets!**

A large bash script was attached ( *mojobox.sh* ).

## Solution
Let's run this file :
```bash
$ chmod +x mojobox.sh
$ ./mojobox.sh
Error: 92fadb3d2af729dff4a3c86168a096e6 = 02fadb3d2af729dff4a3c86168a096e6 ? Integrity check failed
```

Wait, it's just exiting. Let's take a look at the script's code:
```bash
$ head -n 46 mojobox.sh
#!/bin/bash
# MOJO-JOJO's INSANE VAULT
MD5="02fadb3d2af729dff4a3c86168a096e6"
SELF_INTEGRITY="635fb1d17f4d7a2c6fec21b2d7512a15"
# [SILENCE...] Mojo-Jojo is watching you.
check_self() {
    # This checks if you touched the code. 
    # Even a single 'space' added will shatter the lock.
    CURRENT=$(sed -n '7,43p' "$0" | md5sum | cut -d' ' -f1)
    if [ "$CURRENT" != "$SELF_INTEGRITY" ]; then
        echo -e "\033[1;31m[!] CRITICAL ERROR: MIRROR LOCK SHATTERED.\033[0m"
        exit 1
    fi
}
...
```

The script is using an integrity check for itself (lines 7 to 43). But looking at the error message, the MD5 variable on line 3 is **actually wrong**. Someone changed a single digit in the source code! This is the "corruption" the challenge is about.

To fix it, we only need to update the MD5 value on line 3 to match the expected one: `92fadb3d2af729dff4a3c86168a096e6`.

Now let's try running it again:
```bash
$ ./mojobox.sh
[+] VAULT INITIALIZED. STARTING MASTER TERMINAL...
[MOJO-VAULT] ENTER MASTER PIN:
```

### Grabbing the Python Script

The script is now waiting for a PIN input. It has decrypted and extracted its contents into a temporary directory in `/tmp`.

If we wait and look at `/tmp`, we find a new hidden folder `.vault_XXXXXXXX`. Inside it, there's a file called `boot.py`. 

The trick here is to use a **named pipe (fifo)** as input so the script blocks while we copy its temporary files before it has a chance to delete them.

```bash
$ mkfifo /tmp/pin_pipe && ./mojobox.sh < /tmp/pin_pipe &
$ sleep 2 && cp -r $(ls -d /tmp/.vault_* | head -1) /tmp/vault_snapshot
```

### Decoding the Flag

Opening `boot.py`, we find the PIN (`2026`) and our flag encoded in **Base64**.

```python
def check():
    pin = input("[MOJO-VAULT] ENTER MASTER PIN: ")
    if pin == "2026":
        flag = "TU9KTy1KT0pPe2MwcnJ1cHQzZF9idXRfbjB0X2Qzc3RyMHkzZH0="
        # b64 decode ...
```

* By decoding the Base64 flag string we find :

-------->     ***MOJO-JOJO{c0rrupt3d_but_n0t_d3str0y3d}***
