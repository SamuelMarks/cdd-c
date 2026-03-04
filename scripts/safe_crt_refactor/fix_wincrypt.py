with open("src/functions/parse/crypto_wincrypt.c", "r") as f:
    text = f.read()

text = text.replace("#include <wincrypt.h>\n#include <windows.h>", "#include <windef.h>\n#include <winbase.h>\n#include <wincrypt.h>")

with open("src/functions/parse/crypto_wincrypt.c", "w") as f:
    f.write(text)
