# PFF
* ***Category : Forensics / Reverse Engineering***  
 >   Description : **Hide and Seek.**

A file was attached ( *JiaTan_Case_Files.pdf* ).

## Solution
Let's open this file :
It renders as an "RECOVERED ARTIFACT DUMP" report with form fields. The name "JiaTan" is a reference to a well-known backdoor's alias. This suggests something is hidden within.

Let's check the contents of the PDF using `strings`:
```bash
$ strings JiaTan_Case_Files.pdf | grep -i javascript
/S /JavaScript
```

Looks like there's an embedded JavaScript script that handles something in the PDF. Let's decompress it with `qpdf` and see what's happening.

```bash
$ qpdf --qdf --object-streams=disable JiaTan_Case_Files.pdf decompressed.pdf
```

### Reversing the JavaScript

Searching through `decompressed.pdf` for `/JS`, we find a script:
```javascript
var d = [57, 88, 6, 60, 67, 106, 6, 84, 15, 59, 95, 126, 14, 80, 55, 74];
try {
    var s = info.SecretCode;
    var p = this.numPages;
    var a = info.Attacker;
    var key = a + s + p;
    var u = this.getField('ID_INPUT').value;
    var o = '';
    for(var i=0; i<d.length; i++) {
        o += String.fromCharCode(d[i] ^ u.charCodeAt(i % u.length));
    }
    if(o.indexOf('pdf') != -1) {
        this.getField('RESULT_LOG').value = 'SUCCESS: ' + o;
    } 
} catch(e) { }
```

It looks like the script takes our input and XORs it with the `d` array. The correct input seems to be derived from the document's metadata. 

Searching for `/Info` in the PDF, we find:
- **info.Attacker:** `JiaTan`
- **info.SecretCode:** `v0id`
- **numPages:** `2`

If we combine these, our input should be `JiaTanv0id2`. Reversing the XOR in Python yields the string `s1ght_pdf_m4g1c!`. This is part 2 of our flag.

### Dealing with the Image

The PDF also contains a stylized image. Let's extract it:
```bash
$ pdfimages -all JiaTan_Case_Files.pdf extracted_image
```

This gives us `extracted_image-000.jpg`. Since we know it's a forensics challenge, let's use `stegseek` (a massive improvement over `steghide` for brute force) with the `rockyou.txt` wordlist.

```bash
$ stegseek extracted_image-000.jpg /path/to/rockyou.txt
[i] Found passphrase: "gangsta"
[i] Original filename: "part1.txt".
[i] Extracting to "output.txt".
```

The cracked passphrase was `"gangsta"`. When we check `output.txt`, we find part 1 of the flag: `MOJO-JOJO{h1dd3n_1n_pl41n_`.

* By joining the two parts together we find the following flag:

-------->     ***MOJO-JOJO{h1dd3n_1n_pl41n_s1ght_pdf_m4g1c!}***
