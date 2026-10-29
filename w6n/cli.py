"""w6n CLI - asmanalyzer binary'sini çağırır."""
import os
import subprocess
import sys

def get_binary_path():
    """Paketlenmiş asmanalyzer binary'sinin yolunu döndür."""
    here = os.path.dirname(os.path.abspath(__file__))
    return os.path.join(here, "bin", "asmanalyzer")

def main():
    binary = get_binary_path()
    if not os.path.exists(binary):
        sys.stderr.write(
            f"HATA: asmanalyzer binary bulunamadi: {binary}\n"
            f"pip install --force-reinstall w6n\n"
        )
        sys.exit(1)
    os.chmod(binary, 0o755)
    try:
        sys.exit(subprocess.call([binary] + sys.argv[1:]))
    except KeyboardInterrupt:
        sys.exit(130)
